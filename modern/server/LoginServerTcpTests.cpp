// LOGIN-002: real Winsock TCP integration tests for the Login Server.
//
// WHAT MAKES THIS A SEPARATE FILE, AND WHY IT IS NOT PART OF LoginServerTests.
//
// LoginServerTests drives the exchange through LoopbackTransport, an in-process
// pair of byte queues. That is a stronger test of the PROTOCOL than a socket can
// be - it controls exactly where every boundary falls, byte by byte, forever - and
// it stays that way. This file tests something a byte queue cannot: that a real
// socket, the kernel's segmentation, and a real accept loop actually work
// together.
//
// So the two files are complementary rather than successive revisions. What is
// proven HERE and nowhere else:
//
//   * the Login Server binds, listens and accepts on a real port;
//   * an OS-assigned port is readable back, which is the only reason an automated
//     test can exist at all without agreeing with the machine in advance;
//   * REQ_GAME_SVR, SND_GAME_SVR and SND_GAME_SVR_END cross a real TCP stream;
//   * the response survives reads of 1, 2, 3, 8 and 13 bytes - arbitrary
//     fragmentation, with the boundaries chosen by neither side;
//   * malformed packets are refused without a crash, and the server is STILL
//     SERVING afterwards, which is the part a crash-only test would miss;
//   * three clients in a row get three correct, independent answers.
//
// EVERY TEST BINDS 127.0.0.1:0. Nothing here reaches a real interface, a real
// port, or any external host.
//
// EVERY BLOCKING OPERATION IS BOUNDED, and a test that cannot finish is a test
// CI cannot trust. The server-side budget is LoginServerRuntime's; the client-side
// one is passed explicitly to LoginServerSession::Pump.
//
// NO THREADS. A TCP connect completes its handshake in the kernel without anyone
// calling accept() - the connection waits in the backlog - so bind, connect and
// then accept is all one thread's work. Adding a thread per accept would make
// these tests nondeterministic for no gain.

#include "TestHarness.h"

#include "login/LoginServerClient.h"
#include "login/LoginServerConfig.h"
#include "login/LoginServerRuntime.h"
#include "login/LoginServerSession.h"

#include "GameServerListProtocol.h"
#include "NetworkTypes.h"
#include "TcpListener.h"
#include "TcpTransport.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

namespace ModernTests
{
	// So a failing socket assertion prints the transport enum rather than "<?>".
	template <>
	struct Describer<Modern::Network::TransportFault>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::Network::TransportFault& value)
		{
			return Modern::Network::ToString(value);
		}
	};

	template <>
	struct Describer<Modern::Server::LoginServerRefusal>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::Server::LoginServerRefusal& value)
		{
			return Modern::Server::ToString(value);
		}
	};

	template <>
	struct Describer<Modern::Client::LoginServerSessionOutcome>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::Client::LoginServerSessionOutcome& value)
		{
			return Modern::Client::ToString(value);
		}
	};
}

namespace
{
	using namespace Modern;
	using namespace Modern::Network;
	using namespace Modern::Server;
	using namespace Modern::Client;

	constexpr const char* kLoopback = "127.0.0.1";

	// Long enough that a loaded machine does not produce a false failure, short
	// enough that a genuine hang fails the test rather than the run.
	constexpr int kDeadline = 8000;

	// A server bound to an OS-assigned loopback port, with its events collected.
	//
	// Collecting the events rather than printing them is what lets these tests assert
	// that the server DID something, which a log line nobody reads cannot.
	struct TestServer
	{
		LoginServerRuntime runtime;
		std::vector<LoginServerLogEntry> events;
		mutable std::mutex m_eventsMutex;   // guards `events`
		std::thread m_thread;   // the background server, when one is running

		explicit TestServer(Network::GameServerGrid grid)
			: runtime(MakeConfig(std::move(grid)), [this](const LoginServerLogEntry& entry) {
				  // The server runs on another thread, so vents is mutated while the
				  // test thread may be reading it. Unlocked that is a data race, and a
				  // reallocation can invalidate an iterator mid-loop - undefined
				  // behaviour, which shows up as an intermittent failure rather than as
				  // an obvious one.
				  const std::lock_guard<std::mutex> lock(m_eventsMutex);
				  events.push_back(entry);
			  })
		{
		}

		static LoginServerConfig MakeConfig(GameServerGrid grid)
		{
			LoginServerConfig config;
			config.bind.host = kLoopback;
			config.bind.port = 0;  // OS-assigned
			config.servers   = std::move(grid);
			return config;
		}

		// Serves `count` connections on a background thread, and waits for them.
		//
		// NECESSARY, AND THE REASON IS WORTH STATING. LoginServerRuntime serves one
		// connection per ServeOneClient call, so a client that connects and then WAITS
		// for a response cannot be served from the same thread - it would be waiting
		// for a reply that only it could ask the server to send. The handshake
		// completes in the kernel without anyone accepting, which is why the ordering
		// works: the client connects and sends, the server thread accepts and answers,
		// and neither blocks on the other.
		//
		// A thread here is not a claim that the server needs threads. It does not -
		// LoginServerRuntime is synchronous on purpose. This is the test harness
		// standing in for the process boundary that a real deployment has.
		void ServeInBackground(int count)
		{
			m_thread = std::thread([this, count] {
				for (int served = 0; served < count; ++served)
				{
					if (runtime.ServeOneClient(kDeadline).IsError())
					{
						return;
					}
				}
			});
		}

		// Joins the background server. Called before any assertion about the final
		// counters, because those counters are written by the server thread and
		// reading them while it runs would be reading a value mid-update.
		void WaitForBackground()
		{
			if (m_thread.joinable())
			{
				m_thread.join();
			}
		}

		~TestServer() { WaitForBackground(); }

		Status Start() { return runtime.Start(); }

		WireU16 Port() const { return runtime.BoundEndpoint().port; }

		EndpointAddress Address() const
		{
			EndpointAddress address;
			address.ip   = runtime.BoundEndpoint().host;
			address.port = runtime.BoundEndpoint().port;
			return address;
		}

		std::size_t CountOf(LoginServerEvent event) const
		{
			const std::lock_guard<std::mutex> lock(m_eventsMutex);
			std::size_t count = 0;
			for (const LoginServerLogEntry& entry : events)
			{
				if (entry.event == event)
				{
					++count;
				}
			}
			return count;
		}

		std::size_t LastListEntryCount() const
		{
			const std::lock_guard<std::mutex> lock(m_eventsMutex);
			for (std::size_t i = events.size(); i > 0; --i)
			{
				if (events[i - 1].event == LoginServerEvent::ListSent)
				{
					return events[i - 1].count;
				}
			}
			return 0;
		}
	};

	// ---- a raw client, for tests that must send something a correct client never
	// ---- would -----------------------------------------------------------

	// Connects to `server` without any protocol help.
	//
	// Used for the malformed-packet tests, where the whole point is to send bytes no
	// conforming client would produce. A raw socket is the only way to do that without
	// teaching the production client a "send garbage" mode.
	class RawClient
	{
	public:
		Status Connect(const TestServer& server, int timeout = kDeadline)
		{
			Endpoint target;
			target.host = kLoopback;
			target.port = server.Port();
			return m_transport.Connect(target, timeout);
		}

		Status Send(const std::vector<WireU8>& bytes)
		{
			return m_transport.Send(bytes.data(), bytes.size());
		}

		// Closes immediately, which is what a client that gave up mid-message looks
		// like on the wire.
		void HangUp() noexcept { m_transport.Disconnect(); }

		~RawClient() { m_transport.Disconnect(); }

	private:
		TcpTransport m_transport;
	};

	// Builds one complete framed message.
	//
	// Codec::EncodeMessage CLEARS its output, so it encodes exactly one message; a
	// caller wanting a stream of them must concatenate. That is a property worth
	// knowing before writing a test that expects three.
	std::vector<WireU8> Frame(MessageId id, std::size_t declaredSize,
	                          std::size_t bodyBytes = 0)
	{
		MessageHeader header;
		header.type = id;
		header.size = static_cast<WireU32>(declaredSize);

		std::vector<WireU8> frame;
		(void)Codec::WriteU32(frame, header.size);
		(void)Codec::WriteU32(frame, header.type);
		frame.resize(frame.size() + bodyBytes, 0x5A);
		return frame;
	}

	// The real request, as the protocol codec builds it.
	std::vector<WireU8> RealRequest()
	{
		std::vector<WireU8> request;
		(void)GameServerListCodec::AppendRequest(request);
		return request;
	}

	// A bare listening socket used to impersonate a Login Server in the
	// client-failure tests.
	//
	// Deliberately dumb: it accepts one connection and then does exactly what the
	// test tells it to, so that what the CLIENT does with a hostile or broken
	// response can be observed without involving the real server at all.
	class FakeServer
	{
	public:
		Status Start()
		{
			Endpoint bind;
			bind.host = kLoopback;
			bind.port = 0;
			return m_listener.Listen(bind);
		}

		EndpointAddress Address() const
		{
			EndpointAddress address;
			address.ip   = m_listener.BoundEndpoint().host;
			address.port = m_listener.BoundEndpoint().port;
			return address;
		}

		// Accepts the pending connection, adopts it, and reads whatever the client
		// has already sent.
		//
		// THE DRAIN IS NOT COSMETIC. A socket closed with unread data still in its
		// receive buffer is closed ABORTIVELY on Windows: the peer observes an RST
		// rather than a FIN, and a receive reports a reset rather than an orderly
		// close. So a fake server that accepted, sent a partial response and hung up
		// without reading the client's request would be testing the reset path while
		// claiming to test the orderly-close path - which is exactly what happened, and
		// the symptom was a client reporting Faulted where PeerClosed was expected.
		//
		// A real Login Server always reads the request first (LoginServerRuntime reads
		// it before it answers, and before it closes), so draining here is what makes
		// the fake behave like the thing it stands in for.
		//
		// `drainRequest` is exposed so a caller that deliberately wants the abortive
		// close can still have it.
		Status Accept(int timeout = kDeadline, bool drainRequest = true)
		{
			std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
			if (const Status status = m_listener.Accept(raw, timeout); status.IsError())
			{
				return status;
			}
			if (raw == TcpListener::kNoAcceptedSocket)
			{
				return Status(ErrorCode::NotFound);
			}
			Result<TcpTransport> adopted = TcpTransport::Adopt(raw);
			if (adopted.IsError())
			{
				TcpTransport::CloseOwnedHandle(raw);
				return adopted.GetStatus();
			}
			m_connection = std::move(adopted.GetValue());

			if (drainRequest)
			{
				// Read until the client stops sending. The content is irrelevant - this
				// fake is not parsing anything - but the socket has to be drained so that
				// HangUp() produces a FIN rather than an RST. Bounded, so a client that
				// never sends cannot hold this up.
				WireU8      scratch[256] = {};
				std::size_t got          = 0;
				for (int i = 0; i < 8; ++i)
				{
					const Status status = m_connection.Receive(scratch, sizeof(scratch),
					                                           got, 100);
					if (status.IsError() || got == 0)
					{
						break;
					}
				}
			}
			return Ok();
		}

		Status Send(const std::vector<WireU8>& bytes)
		{
			return m_connection.Send(bytes.data(), bytes.size());
		}

		void HangUp() noexcept { m_connection.Disconnect(); }

	private:
		TcpListener m_listener;
		TcpTransport m_connection;
	};

	// One valid entry, framed, for the fake server to send.
	std::vector<WireU8> RealEntryFrame(WireI32 group, WireI32 number)
	{
		GameServerInfo info;
		info.ip             = "127.0.0.1";
		info.servicePort    = 5101;
		info.serverGroup    = group;
		info.serverNumber   = number;
		info.currentClients = 1;
		info.maxClients     = 10;

		std::vector<WireU8> frame;
		(void)GameServerListCodec::AppendEntry(frame, info);
		return frame;
	}
}

// ===========================================================================
// Real exchange
// ===========================================================================

// The milestone's headline test: a real client and a real server, over a real
// socket, with the list arriving whole.
// The milestone's headline test: a real client and a real server, over a real
// socket, with the list arriving whole.
MODERN_TEST(LoginServerTcp_ExchangeOverRealSocketDeliversTheWholeGrid)
{
	TestServer server(LoginServerFixture::Default());
	CHECK(server.Start().IsOk());
	CHECK_NE(server.Port(), 0);
	server.ServeInBackground(1);

	LoginServerClient client;
	LoginServerSession session(client);

	const LoginServerExchange result = session.Exchange(server.Address(), kDeadline);
	server.WaitForBackground();

	CHECK_EQ(result.outcome, LoginServerSessionOutcome::Completed);
	CHECK(result.status.IsOk());
	CHECK(client.IsComplete());

	// Three entries: two on group 1 (channels 1 and 3, with a GAP at 2) and one on
	// group 2. The gap is the interesting part - a client that indexed by arrival
	// rather than by grid would produce a different order or a different count.
	const GameServerGrid& grid = client.Servers();
	CHECK_EQ(grid.Count(), static_cast<std::size_t>(3));
	CHECK_EQ(client.DroppedEntryCount(), static_cast<std::size_t>(0));

	const std::vector<GameServerInfo> received = grid.Servers();
	CHECK_EQ(received.size(), static_cast<std::size_t>(3));

	// Canonical grid order: group ascending, then channel. So group 1 channel 1,
	// group 1 channel 3, then group 2 channel 1 - NOT the fixture's insertion order.
	if (received.size() == 3)
	{
		CHECK_EQ(received[0].serverGroup, 1);
		CHECK_EQ(received[0].serverNumber, 1);
		CHECK_EQ(received[0].servicePort, 5101);
		CHECK_EQ(received[0].currentClients, 3);
		CHECK_EQ(received[0].maxClients, 100);

		CHECK_EQ(received[1].serverGroup, 1);
		CHECK_EQ(received[1].serverNumber, 3);
		CHECK_EQ(received[1].servicePort, 5102);

		CHECK_EQ(received[2].serverGroup, 2);
		CHECK_EQ(received[2].serverNumber, 1);
		CHECK_EQ(received[2].servicePort, 5103);
		CHECK_EQ(received[2].currentClients, 17);
	}

	// The server saw a well-formed request and sent one response.
	CHECK_EQ(server.CountOf(LoginServerEvent::ClientConnected), static_cast<std::size_t>(1));
	CHECK_EQ(server.CountOf(LoginServerEvent::RequestReceived), static_cast<std::size_t>(1));
	CHECK_EQ(server.CountOf(LoginServerEvent::ListSent), static_cast<std::size_t>(1));
	CHECK_EQ(server.LastListEntryCount(), static_cast<std::size_t>(3));
	CHECK_EQ(server.runtime.ServedClientCount(), static_cast<std::size_t>(1));
	CHECK_EQ(server.runtime.RefusedClientCount(), static_cast<std::size_t>(0));
}

// An empty list must still terminate. Legacy sends a bare SND_GAME_SVR_END when it
// has nothing to advertise (s_CLoginServerMsg.cpp:136-144); silence instead would
// leave a client that completes only on END waiting forever.
// An empty list must still terminate. Legacy sends a bare SND_GAME_SVR_END when it
// has nothing to advertise (s_CLoginServerMsg.cpp:136-144); silence instead would
// leave a client that completes only on END waiting forever.
MODERN_TEST(LoginServerTcp_EmptyListStillCompletesWithOnlyTheTerminator)
{
	TestServer server(LoginServerFixture::Empty());
	CHECK(server.Start().IsOk());
	server.ServeInBackground(1);

	LoginServerClient client;
	LoginServerSession session(client);

	const LoginServerExchange result = session.Exchange(server.Address(), kDeadline);
	server.WaitForBackground();

	CHECK_EQ(result.outcome, LoginServerSessionOutcome::Completed);
	CHECK(client.IsComplete());
	CHECK_EQ(client.Servers().Count(), static_cast<std::size_t>(0));

	// Exactly one message crossed the wire: the 8-byte terminator.
	CHECK_EQ(result.messagesHandled, static_cast<std::size_t>(1));
	CHECK_EQ(server.LastListEntryCount(), static_cast<std::size_t>(0));
}

// Port 0 must be readable back, and a FIXED port must be honoured. Together these
// are what make an automated test possible at all: without the first, a test would
// have to guess a port; without the second, the OS-assigned path would be the only
// one ever exercised.
// Port 0 must be readable back, and a FIXED port must be honoured. Together these
// are what make an automated test possible at all: without the first, a test would
// have to guess a port; without the second, the OS-assigned path would be the only
// one ever exercised.
MODERN_TEST(LoginServerTcp_BoundPortIsRealWhetherAssignedOrRequested)
{
	// Assigned.
	{
		TestServer server(LoginServerFixture::Single());
		CHECK(server.Start().IsOk());
		CHECK_NE(server.Port(), 0);
		CHECK_EQ(server.runtime.BoundEndpoint().host, std::string(kLoopback));
		server.ServeInBackground(1);

		LoginServerClient client;
		LoginServerSession session(client);
		CHECK_EQ(session.Exchange(server.Address(), kDeadline).outcome,
		         LoginServerSessionOutcome::Completed);
		server.WaitForBackground();
	}

	// Requested: take an ephemeral port from a throwaway listener, release it, and
	// ask the server for exactly that one.
	WireU16 wanted = 0;
	{
		TcpListener probe;
		Endpoint bind;
		bind.host = kLoopback;
		bind.port = 0;
		CHECK(probe.Listen(bind).IsOk());
		wanted = probe.BoundEndpoint().port;
		probe.Close();
	}
	CHECK_NE(wanted, 0);

	LoginServerConfig config;
	config.bind.host = kLoopback;
	config.bind.port = wanted;
	config.servers   = LoginServerFixture::Single();

	LoginServerRuntime runtime(config);
	CHECK(runtime.Start().IsOk());
	CHECK_EQ(runtime.BoundEndpoint().port, wanted);

	std::thread serverThread([&runtime] {
		for (int i = 0; i < 1; ++i)
		{
			if (runtime.ServeOneClient(kDeadline).IsError())
			{
				return;
			}
		}
	});

	EndpointAddress address;
	address.ip   = kLoopback;
	address.port = wanted;

	LoginServerClient client;
	LoginServerSession session(client);
	const LoginServerExchange result = session.Exchange(address, kDeadline);
	serverThread.join();

	CHECK_EQ(result.outcome, LoginServerSessionOutcome::Completed);
	CHECK_EQ(client.Servers().Count(), static_cast<std::size_t>(1));

	runtime.Stop();
}

// ===========================================================================
// Fragmentation, through the real socket
// ===========================================================================

// Every read size produces the same grid.
//
// The read size is chosen by the TEST and honoured by the client's transport, so
// the boundaries are dictated by neither the server's writes nor the kernel's
// segmentation - which is the strongest form of this test. 1 splits every byte off
// alone; 8 lands exactly on header boundaries; 13 splits a 56-byte body in a way no
// writer would choose.
// Every read size produces the same grid.
//
// The read size is chosen by the TEST and honoured by the client's transport, so
// the boundaries are dictated by neither the server's writes nor the kernel's
// segmentation - which is the strongest form of this test. 1 splits every byte off
// alone; 8 lands exactly on header boundaries; 13 splits a 56-byte body in a way no
// writer would choose.
MODERN_TEST(LoginServerTcp_ArbitraryReadSizesAllReassembleTheSameGrid)
{
	const std::size_t chunkSizes[] = { 1, 2, 3, 7, 8, 13, 56, 64, 4096 };

	const std::vector<GameServerInfo> expected = LoginServerFixture::Default().Servers();

	for (std::size_t chunk : chunkSizes)
	{
		TestServer server(LoginServerFixture::Default());
		CHECK(server.Start().IsOk());
		server.ServeInBackground(1);

		LoginServerClient client;
		LoginServerSession session(client);

		const LoginServerExchange result =
		    session.Exchange(server.Address(), kDeadline, chunk);
		server.WaitForBackground();

		CHECK_EQ(result.outcome, LoginServerSessionOutcome::Completed);
		CHECK(client.IsComplete());
		CHECK_EQ(client.Servers().Count(), static_cast<std::size_t>(3));

		// The bytes must be identical, not merely the same count: a framer that
		// recovered three entries out of order, or with the wrong client counts, would
		// pass a count-only assertion.
		const std::vector<GameServerInfo> received = client.Servers().Servers();
		CHECK_EQ(received.size(), expected.size());
		if (received.size() == expected.size())
		{
			for (std::size_t i = 0; i < expected.size(); ++i)
			{
				CHECK(received[i] == expected[i]);
			}
		}

		server.runtime.Stop();
	}
}

// A header and its body split across separate reads, with the header itself cut in
// half - the case that a parser reading dwSize in one recv would get wrong.
// A header and its body split across separate reads, with the header itself cut in
// half - the case that a parser reading dwSize in one recv would get wrong.
MODERN_TEST(LoginServerTcp_HeaderSplitInHalfStillYieldsAnEntry)
{
	TestServer server(LoginServerFixture::Single());
	CHECK(server.Start().IsOk());
	server.ServeInBackground(1);

	LoginServerClient client;
	LoginServerSession session(client);

	// 4 bytes: less than one header, so the first read cannot even contain dwSize.
	const LoginServerExchange result = session.Exchange(server.Address(), kDeadline, 4);
	server.WaitForBackground();

	CHECK_EQ(result.outcome, LoginServerSessionOutcome::Completed);
	CHECK_EQ(client.Servers().Count(), static_cast<std::size_t>(1));

	const GameServerInfo* entry = client.Servers().Find(0, 0);
	CHECK(entry != nullptr);
	if (entry != nullptr)
	{
		CHECK_EQ(entry->servicePort, 5101);
		CHECK_EQ(entry->maxClients, 10);
	}
}

// ===========================================================================
// Malformed input
// ===========================================================================

// The wrong message id must be refused, not treated as a request.
//
// This is the case the first version of this smoke test hit by accident: a
// hand-built request with the wrong nType bytes produced a clean "unexpected
// message id" rejection and the server carried on.
MODERN_TEST(LoginServerTcp_WrongMessageIdIsRejected)
{
	TestServer server(LoginServerFixture::Default());
	CHECK(server.Start().IsOk());

	{
		RawClient client;
		CHECK(client.Connect(server).IsOk());
		// A complete, well-framed message carrying an id this conversation never uses.
		CHECK(client.Send(Frame(GameServerList::kGameServerInfoId + 1, 8)).IsOk());
		client.HangUp();
	}

	CHECK(server.runtime.ServeOneClient(kDeadline).IsOk());
	CHECK_EQ(server.runtime.RefusedClientCount(), static_cast<std::size_t>(1));
	CHECK_EQ(server.runtime.RefusalKind(), LoginServerRefusal::UnexpectedMessage);
	CHECK_EQ(server.runtime.ServedClientCount(), static_cast<std::size_t>(0));
}

// A REQ_GAME_SVR that is not 8 bytes is refused.
//
// Legacy answers ANY message carrying this id regardless of dwSize
// (s_CLoginServerMsg.cpp:36-38); GameServerListCodec::ValidateRequest is modern
// hardening, and it must hold over a real socket too.
MODERN_TEST(LoginServerTcp_ReqGameSvrWithAWrongSizeIsRejected)
{
	TestServer server(LoginServerFixture::Default());
	CHECK(server.Start().IsOk());

	{
		RawClient client;
		CHECK(client.Connect(server).IsOk());
		// Right id, wrong length: a 12-byte "request" with a 4-byte body.
		CHECK(client.Send(Frame(GameServerList::kRequestGameServersId, 12, 4)).IsOk());
		client.HangUp();
	}

	CHECK(server.runtime.ServeOneClient(kDeadline).IsOk());
	CHECK_EQ(server.runtime.RefusalKind(), LoginServerRefusal::BadRequestSize);
	CHECK_EQ(server.runtime.ServedClientCount(), static_cast<std::size_t>(0));
}

// Half a header, then hang up. The server must notice rather than wait for the
// rest of a message that is never coming.
MODERN_TEST(LoginServerTcp_TruncatedHeaderThenHangUpIsRejected)
{
	TestServer server(LoginServerFixture::Default());
	CHECK(server.Start().IsOk());

	{
		RawClient client;
		CHECK(client.Connect(server).IsOk());
		const std::vector<WireU8> half = { 0x08, 0x00, 0x00, 0x00 };
		CHECK(client.Send(half).IsOk());
		client.HangUp();
	}

	CHECK(server.runtime.ServeOneClient(kDeadline).IsOk());
	CHECK_EQ(server.runtime.RefusedClientCount(), static_cast<std::size_t>(1));
	CHECK_EQ(server.runtime.RefusalKind(), LoginServerRefusal::PeerClosedFirst);
}

// A header promising a 56-byte body, followed by a few bytes and a hang up.
MODERN_TEST(LoginServerTcp_TruncatedBodyThenHangUpIsRejected)
{
	TestServer server(LoginServerFixture::Default());
	CHECK(server.Start().IsOk());

	{
		RawClient client;
		CHECK(client.Connect(server).IsOk());
		CHECK(client.Send(Frame(GameServerList::kGameServerInfoId,
		                        GameServerList::kEntrySize, 8)).IsOk());
		client.HangUp();
	}

	CHECK(server.runtime.ServeOneClient(kDeadline).IsOk());
	CHECK_EQ(server.runtime.RefusalKind(), LoginServerRefusal::PeerClosedFirst);
}

// A declared size beyond the protocol maximum. The framer must refuse rather than
// reserving room for it: a peer that can name any size can otherwise make a server
// allocate whatever it asked for.
MODERN_TEST(LoginServerTcp_OversizedDeclaredSizeIsRejected)
{
	TestServer server(LoginServerFixture::Default());
	CHECK(server.Start().IsOk());

	{
		RawClient client;
		CHECK(client.Connect(server).IsOk());
		CHECK(client.Send(Frame(GameServerList::kGameServerInfoId, 100000, 16)).IsOk());
		client.HangUp();
	}

	CHECK(server.runtime.ServeOneClient(kDeadline).IsOk());
	CHECK_EQ(server.runtime.RefusedClientCount(), static_cast<std::size_t>(1));
	CHECK_EQ(server.runtime.RefusalKind(), LoginServerRefusal::MalformedFrame);
}

// A size below the 8-byte header, which no real message can have.
MODERN_TEST(LoginServerTcp_DeclaredSizeBelowTheHeaderIsRejected)
{
	TestServer server(LoginServerFixture::Default());
	CHECK(server.Start().IsOk());

	{
		RawClient client;
		CHECK(client.Connect(server).IsOk());
		CHECK(client.Send(Frame(GameServerList::kGameServerInfoId, 1)).IsOk());
		client.HangUp();
	}

	CHECK(server.runtime.ServeOneClient(kDeadline).IsOk());
	CHECK_EQ(server.runtime.RefusalKind(), LoginServerRefusal::MalformedFrame);
}

// A stream of pure noise. The header read off it is unlikely to be valid, and if it
// somehow is, the request validation catches it.
MODERN_TEST(LoginServerTcp_GarbageIsRejectedWithoutCrashing)
{
	TestServer server(LoginServerFixture::Default());
	CHECK(server.Start().IsOk());

	{
		RawClient client;
		CHECK(client.Connect(server).IsOk());
		std::vector<WireU8> noise(512);
		for (std::size_t i = 0; i < noise.size(); ++i)
		{
			noise[i] = static_cast<WireU8>((i * 37 + 11) & 0xFF);
		}
		CHECK(client.Send(noise).IsOk());
		client.HangUp();
	}

	CHECK(server.runtime.ServeOneClient(kDeadline).IsOk());
	CHECK_EQ(server.runtime.RefusedClientCount(), static_cast<std::size_t>(1));
}

// THE MOST IMPORTANT MALFORMED CASE.
//
// Every rejection above could be followed by a crash, a hang, or a listener left in
// a state that breaks the next connection - and a test that only checks "it did not
// crash" would pass. So after all of them, a well-behaved client must still get a
// complete, correct answer.
//
// This is the difference between refusing a packet and surviving it.
MODERN_TEST(LoginServerTcp_ServerStillServesCorrectlyAfterEveryKindOfRejection)
{
	TestServer server(LoginServerFixture::Default());
	CHECK(server.Start().IsOk());

	// Every malformed shape, in one batch, on separate connections.
	{
		RawClient wrongId;
		CHECK(wrongId.Connect(server).IsOk());
		CHECK(wrongId.Send(Frame(GameServerList::kGameServerInfoId + 1, 8)).IsOk());
		wrongId.HangUp();
	}
	{
		RawClient wrongSize;
		CHECK(wrongSize.Connect(server).IsOk());
		CHECK(wrongSize.Send(Frame(GameServerList::kRequestGameServersId, 12, 4)).IsOk());
		wrongSize.HangUp();
	}
	{
		RawClient halfHeader;
		CHECK(halfHeader.Connect(server).IsOk());
		CHECK(halfHeader.Send({ 0x08, 0x00, 0x00, 0x00 }).IsOk());
		halfHeader.HangUp();
	}
	{
		RawClient truncatedBody;
		CHECK(truncatedBody.Connect(server).IsOk());
		CHECK(truncatedBody.Send(Frame(GameServerList::kGameServerInfoId,
		                               GameServerList::kEntrySize, 8)).IsOk());
		truncatedBody.HangUp();
	}
	{
		RawClient oversized;
		CHECK(oversized.Connect(server).IsOk());
		CHECK(oversized.Send(Frame(GameServerList::kGameServerInfoId, 100000, 16)).IsOk());
		oversized.HangUp();
	}
	{
		RawClient garbage;
		CHECK(garbage.Connect(server).IsOk());
		std::vector<WireU8> noise(300);
		for (std::size_t i = 0; i < noise.size(); ++i)
		{
			noise[i] = static_cast<WireU8>((i * 91 + 7) & 0xFF);
		}
		CHECK(garbage.Send(noise).IsOk());
		garbage.HangUp();
	}
	{
		RawClient silent;
		CHECK(silent.Connect(server).IsOk());
		silent.HangUp();
	}

	CHECK(server.runtime.ServeClients(7, kDeadline).IsOk());
	CHECK_EQ(server.runtime.RefusedClientCount(), static_cast<std::size_t>(7));
	CHECK_EQ(server.runtime.ServedClientCount(), static_cast<std::size_t>(0));

	// And now a good client, on the same listener, after all of that.
	server.ServeInBackground(1);
	
	LoginServerClient client;
	LoginServerSession session(client);
	const LoginServerExchange result = session.Exchange(server.Address(), kDeadline);

	CHECK_EQ(result.outcome, LoginServerSessionOutcome::Completed);
	CHECK_EQ(client.Servers().Count(), static_cast<std::size_t>(3));
	server.WaitForBackground();
	
	CHECK_EQ(server.runtime.ServedClientCount(), static_cast<std::size_t>(1));
	CHECK_EQ(server.runtime.RefusedClientCount(), static_cast<std::size_t>(7));
}

// ===========================================================================
// Multiple clients
// ===========================================================================

// Three clients, one after another, each with its own connection and its own
// answer.
//
// Sequential rather than concurrent on purpose: the claim being tested is that one
// connection does not corrupt another, and a sequential test cannot be flaky for
// scheduling reasons. A caller wanting simultaneous clients should serve from
// several threads.
MODERN_TEST(LoginServerTcp_ThreeSequentialClientsEachGetTheCompleteList)
{
	TestServer server(LoginServerFixture::Default());
	CHECK(server.Start().IsOk());
	server.ServeInBackground(3);

	for (int clientIndex = 0; clientIndex < 3; ++clientIndex)
	{
		LoginServerClient client;
		LoginServerSession session(client);

		const LoginServerExchange result = session.Exchange(server.Address(), kDeadline);
		CHECK_EQ(result.outcome, LoginServerSessionOutcome::Completed);
		CHECK_EQ(client.Servers().Count(), static_cast<std::size_t>(3));

		const std::vector<GameServerInfo> expected = LoginServerFixture::Default().Servers();
		const std::vector<GameServerInfo> received = client.Servers().Servers();
		CHECK_EQ(received.size(), expected.size());
		if (received.size() == expected.size())
		{
			for (std::size_t i = 0; i < expected.size(); ++i)
			{
				CHECK(received[i] == expected[i]);
			}
		}

		session.Disconnect();
	}

	server.WaitForBackground();
	
	CHECK_EQ(server.runtime.ServedClientCount(), static_cast<std::size_t>(3));
	CHECK_EQ(server.CountOf(LoginServerEvent::ClientConnected), static_cast<std::size_t>(3));
}

// Each connection must be answered from the configured list and from nothing else.
//
// The grid is a member of the server, and the server is reused across connections,
// so this is where a framer or a buffer that outlived its connection would show
// up: client 2 seeing client 1's entries, or entries arriving twice.
// Each connection must be answered from the configured list and from nothing else.
//
// The grid is a member of the server and the server is reused across connections,
// so this is where a framer or a buffer that outlived its connection would show up:
// client 2 seeing client 1's entries, or entries arriving twice.
MODERN_TEST(LoginServerTcp_EachConnectionSeesOnlyItsOwnList)
{
	TestServer server(LoginServerFixture::Single());
	CHECK(server.Start().IsOk());

	// The join is AFTER the loop, and that placement is load-bearing twice over.
	//
	// Joining inside the loop would block on a server thread still waiting to accept
	// clients that do not exist yet.
	//
	// And reading the server's event log inside the loop asks the server thread a
	// question it may not have finished answering: a client can hold the complete
	// list before the server has recorded having sent it, because the bytes are in
	// flight the moment Send returns. An earlier version of this test asserted
	// LastListEntryCount() per iteration and failed intermittently - in Release more
	// often than in Debug, which is exactly the signature of a race rather than of a
	// logic error. The mutex on the log removes the undefined behaviour; moving the
	// assertion past the join removes the ordering mistake.
	server.ServeInBackground(3);

	for (int clientIndex = 0; clientIndex < 3; ++clientIndex)
	{
		LoginServerClient client;
		LoginServerSession session(client);

		CHECK_EQ(session.Exchange(server.Address(), kDeadline).outcome,
		         LoginServerSessionOutcome::Completed);

		// One entry, every time. If any state leaked between connections this would be
		// two or three on the later iterations. These assertions are about the CLIENT,
		// which is entirely in this thread's hands by the time Exchange returns.
		CHECK_EQ(client.Servers().Count(), static_cast<std::size_t>(1));
		CHECK_EQ(client.DroppedEntryCount(), static_cast<std::size_t>(0));
	}

	server.WaitForBackground();

	// Now that the server thread is finished, that it reported one entry each time.
	CHECK_EQ(server.CountOf(LoginServerEvent::ListSent), static_cast<std::size_t>(3));
	CHECK_EQ(server.runtime.ServedClientCount(), static_cast<std::size_t>(3));
	CHECK_EQ(server.runtime.RefusedClientCount(), static_cast<std::size_t>(0));
}

// ===========================================================================
// The client's handling of a bad or absent server
// ===========================================================================

// The server hangs up after one entry, with no terminator.
//
// The client must report PeerClosed and NOT claim completion. Completing on a
// short list would hand the caller a server list that silently lost entries - the
// one failure mode worse than an error, because it looks like success.
MODERN_TEST(LoginServerClient_HandlesServerClosingBeforeTheTerminator)
{
	FakeServer server;
	CHECK(server.Start().IsOk());

	LoginServerClient client;
	LoginServerSession session(client);

	CHECK(session.Connect(server.Address()).IsOk());
	CHECK(session.RequestGameServers().IsOk());

	CHECK(server.Accept().IsOk());
	CHECK(server.Send(RealEntryFrame(1, 1)).IsOk());
	server.HangUp();

	const LoginServerExchange result = session.Pump(kDeadline);

	CHECK_EQ(result.outcome, LoginServerSessionOutcome::PeerClosed);
	// One entry did arrive, and it is genuinely there - the point is that the list is
	// NOT reported as complete.
	CHECK_EQ(client.Servers().Count(), static_cast<std::size_t>(1));
	CHECK(!client.IsComplete());
}

// A server that says nothing at all. Bounded, and reported as a timeout rather than
// as a fault - nothing went wrong, the answer is simply late.
MODERN_TEST(LoginServerClient_ReportsATimeoutWhenTheServerSaysNothing)
{
	FakeServer server;
	CHECK(server.Start().IsOk());

	LoginServerClient client;
	LoginServerSession session(client);

	CHECK(session.Connect(server.Address()).IsOk());
	CHECK(session.RequestGameServers().IsOk());
	CHECK(server.Accept().IsOk());

	// Deliberately silent, and deliberately short: 600ms. A test that waited the
	// transport's full default here would be a slow test rather than a bounded one.
	const LoginServerExchange result = session.Pump(600);

	CHECK_EQ(result.outcome, LoginServerSessionOutcome::TimedOut);
	CHECK(!client.IsComplete());
	CHECK(result.status.IsOk());   // a timeout is not a fault
	CHECK_EQ(client.Servers().Count(), static_cast<std::size_t>(0));
}

// A header that cannot be valid. The client must fail rather than guess.
MODERN_TEST(LoginServerClient_RejectsAMalformedResponseHeader)
{
	FakeServer server;
	CHECK(server.Start().IsOk());

	LoginServerClient client;
	LoginServerSession session(client);

	CHECK(session.Connect(server.Address()).IsOk());
	CHECK(session.RequestGameServers().IsOk());

	CHECK(server.Accept().IsOk());
	// dwSize = 1, which is below the 8-byte header.
	CHECK(server.Send(Frame(GameServerList::kGameServerListEndId, 1)).IsOk());

	const LoginServerExchange result = session.Pump(kDeadline);
	CHECK_EQ(result.outcome, LoginServerSessionOutcome::Faulted);
	CHECK(result.status.IsError());
	CHECK(!client.IsComplete());
}

// A well-framed message carrying an id that is not part of this conversation.
MODERN_TEST(LoginServerClient_RejectsAnUnexpectedResponseMessage)
{
	FakeServer server;
	CHECK(server.Start().IsOk());

	LoginServerClient client;
	LoginServerSession session(client);

	CHECK(session.Connect(server.Address()).IsOk());
	CHECK(session.RequestGameServers().IsOk());

	CHECK(server.Accept().IsOk());
	CHECK(server.Send(Frame(GameServerList::kGameServerListEndId + 7, 8)).IsOk());

	const LoginServerExchange result = session.Pump(kDeadline);
	CHECK_EQ(result.outcome, LoginServerSessionOutcome::Faulted);
	CHECK(!client.IsComplete());
}

// A client that is asked to respond to nothing. Sending a request without a
// connection is refused rather than silently doing nothing.
MODERN_TEST(LoginServerClient_RefusesToWorkWithoutAConnection)
{
	LoginServerClient client;
	LoginServerSession session(client);

	CHECK(session.RequestGameServers().IsError());
	CHECK(!session.IsConnected());

	// Two clients at once is refused, because one session is one conversation and a
	// second connect would otherwise leave the first connection's bytes in the
	// protocol object with nothing to say which server they came from.
	FakeServer server;
	CHECK(server.Start().IsOk());

	CHECK(session.Connect(server.Address()).IsOk());
	CHECK(session.Connect(server.Address()).IsError());
}

// ===========================================================================
// Configuration
// ===========================================================================

// A bad bind address is refused rather than defaulted.
//
// Legacy substitutes INADDR_ANY for an unparsable server_ip
// (s_CServer.cpp:627-631), so a typo in a config file quietly turns a
// loopback-only server into one exposed to every interface. Refusing it is the
// whole reason this type exists.
MODERN_TEST(LoginServerConfig_RejectsBadBindAddresses)
{
	LoginServerConfig config;

	config.bind.host.clear();
	CHECK(config.Validate().GetCode() == ErrorCode::InvalidArgument);

	config.bind.host = "not-an-address";
	CHECK(config.Validate().GetCode() == ErrorCode::InvalidArgument);

	// A name the transport could resolve is still refused as CONFIGURATION. The RAN
	// convention is a numeric address, and a bind address nobody can read off the
	// server's own configuration is a bad bind address.
	config.bind.host = "some.invalid.name";
	CHECK(config.Validate().GetCode() == ErrorCode::InvalidArgument);

	config.bind.host = "127.0.0.1";
	CHECK(config.Validate().IsOk());

	// localhost is the one name allowed, because it names this machine only.
	config.bind.host = "localhost";
	CHECK(config.Validate().IsOk());
}

// An empty grid is a VALID configuration. "No game servers are running" is a state
// a Login Server has to be able to report, and the wire has a way to say it.
MODERN_TEST(LoginServerConfig_EmptyGridIsValidAndEphemeralPortIsRecognised)
{
	LoginServerConfig config;
	config.bind.host = "127.0.0.1";
	config.bind.port = 0;

	CHECK(config.Validate().IsOk());
	CHECK_EQ(config.servers.Count(), static_cast<std::size_t>(0));
	CHECK(config.UsesEphemeralPort());

	config.bind.port = 5001;
	CHECK(!config.UsesEphemeralPort());
}

// A server that was never started must refuse to serve, rather than listening on
// nothing and reporting success.
MODERN_TEST(LoginServerRuntime_RefusesToServeWhenNotRunning)
{
	LoginServerConfig config;
	config.bind.host = kLoopback;
	config.bind.port = 0;
	config.servers   = LoginServerFixture::Single();

	LoginServerRuntime server(config);

	// Constructed but never started: not listening, and it must say so rather than
	// pretend to serve. A runtime that accepted connections before Start() would be
	// listening on nothing while reporting success.
	CHECK(!server.IsRunning());
	CHECK(server.ServeOneClient(50).IsError());
	CHECK(server.ServeClients(1, 50).IsError());

	// Stop is idempotent, so a shutdown path that calls it twice - which every
	// shutdown path eventually does - is not an error.
	server.Stop();
	server.Stop();
	CHECK(!server.IsRunning());

	// A configuration the runtime refuses must be reported at Start, before a socket
	// is opened, so the caller sees a bad address rather than a bind failure.
	LoginServerConfig bad;
	bad.bind.host = "not-an-address";
	bad.bind.port = 0;

	LoginServerRuntime refused(bad);
	CHECK_EQ(refused.Start().GetCode(), ErrorCode::InvalidArgument);
	CHECK(!refused.IsRunning());
}

int main()
{
	// Unbuffered, so a test that hangs names itself rather than taking its last line
	// down with it.
	std::setvbuf(stdout, nullptr, _IONBF, 0);

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n",
		            static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
	            failedCases,
	            static_cast<int>(ModernTests::Registry().size()),
	            ModernTests::FailureCount());
	return 1;
}
