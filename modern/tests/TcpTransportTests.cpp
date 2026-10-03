// NETWORK-001: tests for the real Winsock TCP layer.
//
// WHAT SEPARATES THESE FROM EVERY OTHER TEST IN THIS DIRECTORY.
//
// Every other network test runs against LoopbackTransport and is byte-exact and
// instant. These open actual sockets on 127.0.0.1 and are therefore the first
// tests in the modern tree whose result can depend on the operating system. That
// is the point of them: the abstraction was already proven, and what remained
// unproven was whether a real socket behaves the way the abstraction promises.
//
// SO WHAT IS ACTUALLY PROVEN HERE, AND WHY IT MATTERS:
//
//   * A real connect succeeds against a real listening socket, and the port the
//     OS assigns for a bind to port 0 is readable back. Without this there is no
//     way to write a test that does not first have to agree with the machine on
//     a port number.
//
//   * Receive returns WHAT ARRIVED, not what was asked for. This is the
//     assumption that costs the most when it is wrong: a transport that blocked
//     until maxBytes arrived would be a transport that had invented message
//     boundaries, and ConnectionFramer would then be framing a stream that
//     cannot exist.
//
//   * The send loop really loops. A payload far larger than any socket buffer
//     cannot go out in one call, so a single-call Send would truncate.
//
//   * Every blocking operation has a deadline that actually elapses. A test that
//     can hang is a test CI cannot trust, and these all assert the bounded case.
//
// NO THREADS. A TCP connect completes the handshake in the kernel without anyone
// calling accept(), because the connection waits in the backlog. So the pattern
// throughout is: bind, connect, THEN accept - all on one thread. The alternative,
// a thread per accept, would add nondeterminism to tests whose entire value is
// that they are not flaky.
//
// EVERY TEST BINDS 127.0.0.1:0. Nothing here touches a real interface, a real
// port or any external host, and nothing here can collide with another run.

#include "TestHarness.h"

#include <algorithm>
#include <cstdio>

// ModernNetwork exposes modern/network as its include root.
#include "NetworkConnection.h"
#include "NetworkTransport.h"
#include "NetworkTypes.h"
#include "SocketAddress.h"
#include "TcpListener.h"
#include "TcpTransport.h"
#include "WinSockRuntime.h"

namespace ModernTests
{
	// NETWORK-001: the transport enums, so a failing socket test prints the value it
	// disagreed about instead of "<?>". The generic Describer cannot help here
	// because these are scoped enums, and "expected: <?>" tells nobody anything.
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
	struct Describer<Modern::Network::TransportState>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::Network::TransportState& value)
		{
			return value == Modern::Network::TransportState::Open ? "Open" : "Closed";
		}
	};

	template <>
	struct Describer<Modern::Network::TcpTransportPhase>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::Network::TcpTransportPhase& value)
		{
			return Modern::Network::ToString(value);
		}
	};
}

namespace
{
	using namespace Modern;
	using namespace Modern::Network;

	// ---- helpers ---------------------------------------------------------

	constexpr const char* kLoopback = "127.0.0.1";

	// Deadlines used throughout. Long enough that a loaded CI machine does not
	// produce a false failure, short enough that a genuine hang fails the test
	// rather than the run.
	constexpr int kDeadline = 4000;

	// Ensures Winsock is up for the tests that call into the socket layer without
	// constructing a transport first - SocketAddress::Resolve is the one such call.
	void EnsureRuntime()
	{
		if (WinSock::Acquire().IsError())
		{
			std::printf("    FAIL could not start Winsock\n");
			++ModernTests::FailureCount();
		}
	}

	// A listener bound to an OS-assigned loopback port, plus that port.
	//
	// Returns port 0 on failure, which every caller then fails on rather than
	// connecting to something unintended.
	WireU16 BindEphemeralLoopback(TcpListener& listener)
	{
		Endpoint bind;
		bind.host = kLoopback;
		bind.port = 0;

		// The failure is reported in full rather than swallowed. A bind that fails
		// for a reason the caller cannot see is indistinguishable from a port clash,
		// and guessing between them wastes more time than printing them.
		const Status listened = listener.Listen(bind);
		if (listened.IsError())
		{
			std::printf("    DIAG Listen failed: %s fault=%s native=%d\n",
			            listened.GetMessage(),
			            ToString(listener.Fault()),
			            listener.NativeError());
			std::fflush(stdout);
			return 0;
		}
		return listener.BoundEndpoint().port;
	}

	// Connects a client to a listener, then accepts, on this thread.
	//
	// Both halves live in one helper because that ordering is the whole trick:
	// Connect succeeds on the kernel's handshake, and only then does Accept have
	// something to return. Splitting them across helpers would invite a reader to
	// assume Accept must come first.
	// Why each failure, because "ConnectAndAccept returned false" is not a
	// diagnosis: the four ways it can fail are a refused bind, a refused connect, a
	// timed-out accept and a failed adopt, and they need completely different fixes.
	#define FAIL_CONNECT_AND_ACCEPT(why)                                    \
	    do {                                                                \
	        std::printf("    DIAG ConnectAndAccept: %s\n", (why));          \
	        std::fflush(stdout);                                             \
	        return false;                                                    \
	    } while (false)

	bool ConnectAndAccept(TcpListener& listener, TcpTransport& client, TcpTransport& server)
	{
		// Binds here rather than requiring every caller to have done it. The helper's
		// contract is "give me a connected client/server pair on loopback", and
		// requiring four call sites to remember a bind step is a way to get a test
		// that silently exercises nothing - which is exactly what happened when this
		// assumed a bound listener and read port 0 from an unbound one.
		if (!listener.IsListening() && BindEphemeralLoopback(listener) == 0)
		{
			FAIL_CONNECT_AND_ACCEPT("could not bind an ephemeral loopback port");
		}

		const WireU16 port = listener.BoundEndpoint().port;

		Endpoint target;
		target.host = kLoopback;
		target.port = port;

		if (client.Connect(target).IsError())
		{
			std::printf("    DIAG connect: fault=%s native=%d\n",
			            ToString(client.Fault()), client.NativeError());
			FAIL_CONNECT_AND_ACCEPT("connect failed");
		}

		std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
		const Status    accepted = listener.Accept(raw, kDeadline);
		if (accepted.IsError())
		{
			std::printf("    DIAG accept: %s fault=%s native=%d\n",
			            accepted.GetMessage(), ToString(listener.Fault()),
			            listener.NativeError());
			FAIL_CONNECT_AND_ACCEPT("accept failed");
		}
		if (raw == TcpListener::kNoAcceptedSocket)
		{
			FAIL_CONNECT_AND_ACCEPT("accept returned the sentinel");
		}

		Result<TcpTransport> adopted = TcpTransport::Adopt(raw);
		if (adopted.IsError())
		{
			std::printf("    DIAG adopt: %s\n", adopted.GetMessage());
			FAIL_CONNECT_AND_ACCEPT("adopt failed");
		}

		server = std::move(adopted.GetValue());
		if (!server.IsConnected())
		{
			FAIL_CONNECT_AND_ACCEPT("adopted transport is not connected");
		}
		return true;
	}

	#undef FAIL_CONNECT_AND_ACCEPT

	std::vector<WireU8> Pattern(std::size_t size, WireU8 start = 0)
	{
		std::vector<WireU8> bytes(size);
		for (std::size_t i = 0; i < size; ++i)
		{
			bytes[i] = static_cast<WireU8>((start + i) & 0xFF);
		}
		return bytes;
	}

	// Drains `transport` until `expected` bytes have arrived or the deadline passes.
	//
	// The deadline is what keeps this a test rather than a hang: a socket that
	// stopped delivering would otherwise loop forever, and a test that can hang is
	// a test nobody trusts.
	bool Drain(TcpTransport& transport, std::vector<WireU8>& out, std::size_t expected)
	{
		out.clear();
		out.reserve(expected);

		std::vector<WireU8> chunk(64 * 1024);
		while (out.size() < expected)
		{
			std::size_t got = 0;
			const Status status = transport.Receive(chunk.data(), chunk.size(), got, 200);
			if (status.IsError())
			{
				return false;
			}
			if (got == 0)
			{
				// Deadline elapsed with the stream incomplete. Returning false rather
				// than looping is the whole reason Receive is bounded.
				return false;
			}
			out.insert(out.end(), chunk.begin(), chunk.begin() + static_cast<std::ptrdiff_t>(got));
		}
		return true;
	}
}

// ---------------------------------------------------------------------------
// Winsock lifetime
// ---------------------------------------------------------------------------

// A transport and a listener each take a reference, and each give it back.
//
// The counter is exposed precisely so this can be asserted: a process that calls
// WSACleanup one time too few still appears to work perfectly, so the imbalance
// is invisible from every other direction.
MODERN_TEST(Tcp_WinsockReferencesAreBalancedByTransportsAndListeners)
{
	EnsureRuntime();

	const std::size_t before = WinSock::ReferenceCount();
	{
		TcpListener listener;
		TcpTransport client;
		CHECK_EQ(WinSock::ReferenceCount(), before + 2);
	}
	CHECK_EQ(WinSock::ReferenceCount(), before);
}

// Explicit subsystem-level acquire composes with the per-socket one rather than
// replacing it. A server that binds once and accepts many holds Winsock up this
// way, across connection lifetimes.
MODERN_TEST(Tcp_ExplicitRuntimeAcquireNestsWithSocketReferences)
{
	EnsureRuntime();

	const std::size_t before = WinSock::ReferenceCount();
	CHECK(TcpTransport::AcquireRuntime().IsOk());
	CHECK_EQ(WinSock::ReferenceCount(), before + 1);
	{
		TcpTransport transport;
		CHECK_EQ(WinSock::ReferenceCount(), before + 2);
	}
	CHECK_EQ(WinSock::ReferenceCount(), before + 1);
	TcpTransport::ReleaseRuntime();
	CHECK_EQ(WinSock::ReferenceCount(), before);
	CHECK(WinSock::IsReady());
}

// ---------------------------------------------------------------------------
// Listener
// ---------------------------------------------------------------------------

// Binding port 0 must yield a real, non-zero port that the test can then connect
// to. This is what makes a real-socket test possible at all: without it every
// such test would have to agree with the machine on a port number in advance.
MODERN_TEST(Tcp_ListenerBindsLoopbackAndReportsTheAssignedPort)
{
	TcpListener listener;
	// A constructed listener is not yet listening: it owns a socket but has
	// bound nothing. That distinction is the reason IsListening exists
	// separately from construction.
	CHECK(!listener.IsListening());

	const WireU16 port = BindEphemeralLoopback(listener);
	CHECK_NE(port, 0);
	CHECK(listener.IsListening());

	const Endpoint bound = listener.BoundEndpoint();
	CHECK_EQ(bound.host, std::string(kLoopback));
	CHECK_EQ(static_cast<int>(bound.port), static_cast<int>(port));
}

// An empty bind host cannot be bound, and must be reported rather than becoming
// 0.0.0.0 - a listener that quietly listened on every interface would be a very
// unpleasant surprise for a caller who meant to restrict it.
MODERN_TEST(Tcp_ListenerRejectsAnEmptyBindHost)
{
	TcpListener listener;
	Endpoint bind;
	CHECK_EQ(listener.Listen(bind).GetCode(), ErrorCode::InvalidArgument);
	CHECK(!listener.IsListening());
}

// SO_REUSEADDR is deliberately not set, so a second bind on a port already being
// listened on must fail rather than silently splitting traffic between two
// servers.
MODERN_TEST(Tcp_SecondBindOnALivePortIsRefused)
{
	TcpListener first;
	const WireU16 port = BindEphemeralLoopback(first);
	CHECK_NE(port, 0);

	TcpListener second;
	Endpoint bind;
	bind.host = kLoopback;
	bind.port = port;
	CHECK(second.Listen(bind).IsError());
	CHECK(!second.IsListening());
}

// Closing twice must be a no-op rather than a second closesocket, which would
// free a handle the OS may already have handed to something else.
MODERN_TEST(Tcp_ListenerCloseIsIdempotent)
{
	TcpListener listener;
	CHECK_NE(BindEphemeralLoopback(listener), 0);

	listener.Close();
	CHECK(!listener.IsListening());

	listener.Close();
	CHECK(!listener.IsListening());
}

// A destructor must close the socket too, or a server that binds and exits leaks
// the port and every test after it collides.
MODERN_TEST(Tcp_ListenerDestructionReleasesThePort)
{
	WireU16 port = 0;
	{
		TcpListener listener;
		port = BindEphemeralLoopback(listener);
		CHECK_NE(port, 0);
	}

	TcpListener again;
	Endpoint bind;
	bind.host = kLoopback;
	bind.port = port;
	CHECK(again.Listen(bind).IsOk());
}

// An accept with nobody knocking must return within its deadline and report that
// nothing arrived - not block forever, and not report an error.
MODERN_TEST(Tcp_AcceptTimesOutWithoutBlockingForever)
{
	TcpListener listener;
	CHECK_NE(BindEphemeralLoopback(listener), 0);

	std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
	CHECK(listener.Accept(raw, 150).IsOk());
	CHECK_EQ(raw, TcpListener::kNoAcceptedSocket);
	CHECK_EQ(listener.Fault(), TransportFault::WouldBlock);
}

// Accepting when not listening is a caller error, and is reported by value rather
// than than asserted or thrown.
MODERN_TEST(Tcp_AcceptOnAClosedListenerIsReportedNotIgnored)
{
	TcpListener listener;
	std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
	CHECK_EQ(listener.Accept(raw, 100).GetCode(), ErrorCode::InvalidState);
	CHECK_EQ(raw, TcpListener::kNoAcceptedSocket);
}

// ---------------------------------------------------------------------------
// Connect
// ---------------------------------------------------------------------------

MODERN_TEST(Tcp_ConnectSucceedsAgainstALiveListener)
{
	TcpListener listener;
	CHECK_NE(BindEphemeralLoopback(listener), 0);

	Endpoint target;
	target.host = kLoopback;
	target.port = listener.BoundEndpoint().port;

	TcpTransport client;
	CHECK(client.Connect(target).IsOk());
	CHECK(client.IsConnected());
	CHECK_EQ(client.State(), TransportState::Open);
	CHECK_EQ(client.Phase(), TcpTransportPhase::Connected);
	CHECK(client.IsValid());

	// RemoteEndpoint records the RESOLVED NUMERIC peer, because RAN's wire can
	// only carry a numeric address even when a name was used to connect.
	const Endpoint remote = client.RemoteEndpoint();
	CHECK_EQ(remote.host, std::string(kLoopback));
	CHECK_EQ(static_cast<int>(remote.port), static_cast<int>(listener.BoundEndpoint().port));

	// LocalEndpoint must be a real local address, never a zeroed 0.0.0.0:0 that
	// would read as "connected to nowhere".
	CHECK(!client.LocalEndpoint().host.empty());
	CHECK_NE(static_cast<int>(client.LocalEndpoint().port), 0);
}

// A host that cannot resolve is reported. Legacy's equivalent failure was a name
// silently becoming INADDR_NONE, and this is the difference.
MODERN_TEST(Tcp_ConnectToAnUnresolvableHostIsReported)
{
	TcpTransport client;
	Endpoint target;
	target.host = "no-such-host.invalid";
	target.port = 5001;

	CHECK(client.Connect(target).GetCode() == ErrorCode::InvalidArgument);
	CHECK(!client.IsConnected());
}

MODERN_TEST(Tcp_ConnectRejectsAnEmptyHost)
{
	TcpTransport client;
	Endpoint target;
	target.port = 5001;
	CHECK(client.Connect(target).GetCode() == ErrorCode::InvalidArgument);
}

// Nothing is listening, so the handshake is refused. This must be distinguishable
// from a timeout: "nothing is listening" and "nothing answered" are different
// diagnoses with different fixes.
// A port with nothing on it must not hang, and must be attributable.
//
// WHY THIS DOES NOT ASSERT ConnectionRefused SPECIFICALLY, which is the
// interesting part. Closing a listening socket and immediately connecting to the
// port it had is NOT guaranteed to produce a reset on Windows: the local port can
// stay reserved for a short while after the listener closes, and a SYN sent into
// that reservation is dropped rather than answered. The connect then waits out its
// deadline and reports Timeout instead of ConnectionRefused.
//
// Both outcomes are genuine, well-diagnosed failures and both are bounded, which
// is the property that actually matters - a connect that hangs forever, or that
// reports success, is the bug. Asserting one specific value would make this test
// pass or fail according to a Windows port-lifetime timing detail that has nothing
// to do with the transport, which is the flakiness this suite exists to avoid.
MODERN_TEST(Tcp_ConnectToAPortWithNothingOnItFailsWithinItsDeadline)
{
	WireU16 port = 0;
	{
		// Bind, learn the port, then close: the port is reserved by nothing.
		TcpListener listener;
		port = BindEphemeralLoopback(listener);
		CHECK_NE(port, 0);
	}

	TcpTransport client;
	Endpoint target;
	target.host = kLoopback;
	target.port = port;

	CHECK(client.Connect(target).IsError());
	CHECK(!client.IsConnected());

	const TransportFault fault = client.Fault();
	CHECK(fault == TransportFault::ConnectionRefused || fault == TransportFault::Timeout);

	// Whichever of the two it was, the transport is reusable: a caller whose connect
	// failed must be able to try again rather than construct a new object.
	CHECK_EQ(client.Phase(), TcpTransportPhase::Initialized);
	CHECK(client.IsValid());
}

// Connecting twice re-points the transport rather than leaving one handle with two
// connections' worth of state.
MODERN_TEST(Tcp_SecondConnectRePointsTheSameTransport)
{
	TcpListener listener;
	CHECK_NE(BindEphemeralLoopback(listener), 0);

	Endpoint target;
	target.host = kLoopback;
	target.port = listener.BoundEndpoint().port;

	TcpTransport client;
	CHECK(client.Connect(target).IsOk());
	CHECK(client.Connect(target).IsOk());
	CHECK(client.IsConnected());

	std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
	CHECK(listener.Accept(raw, kDeadline).IsOk());
	CHECK_NE(raw, TcpListener::kNoAcceptedSocket);
}

// ---------------------------------------------------------------------------
// Send and receive
// ---------------------------------------------------------------------------

MODERN_TEST(Tcp_SendAndReceiveRoundTripOverARealSocket)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	const std::vector<WireU8> sent = Pattern(64, 7);
	CHECK(client.Send(sent.data(), sent.size()).IsOk());
	CHECK_EQ(client.LastSendCount(), sent.size());

	// 64 KiB, not 64 bytes: a receive asked for less than was sent proves the
	// transport truncates to the caller's buffer, and one asked for more proves it
	// does not invent a message boundary.
	std::vector<WireU8> got;
	CHECK(Drain(server, got, sent.size()));
	CHECK_EQ(got.size(), sent.size());
	CHECK(got == sent);
}

// A receive must return what ARRIVED. If it blocked for the full request instead,
// the caller could not tell an idle connection from a slow one.
MODERN_TEST(Tcp_ReceiveReturnsWhatArrivedRatherThanBlockingForAll)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	const std::vector<WireU8> sent = Pattern(10);
	CHECK(client.Send(sent.data(), sent.size()).IsOk());

	WireU8     buffer[4096] = {};
	std::size_t got         = 0;
	CHECK(server.Receive(buffer, sizeof(buffer), got, kDeadline).IsOk());
	CHECK_GT(got, 0);
	CHECK_LE(got, sent.size());
	for (std::size_t i = 0; i < got; ++i)
	{
		CHECK_EQ(static_cast<int>(buffer[i]), static_cast<int>(sent[i]));
	}
}

// The send loop must actually loop. 512 KiB cannot leave in one call once the
// socket buffer fills, so a transport that called send() once would truncate
// here - and the truncation would look like a lost message on the wire.
MODERN_TEST(Tcp_LargePayloadCrossesTheSocketIntact)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	const std::size_t size = 512 * 1024;
	const std::vector<WireU8> sent = Pattern(size, 3);
	CHECK(client.Send(sent.data(), sent.size()).IsOk());
	CHECK_EQ(client.LastSendCount(), size);

	std::vector<WireU8> got;
	CHECK(Drain(server, got, size));
	CHECK_EQ(got.size(), size);
	CHECK(got == sent);
}

MODERN_TEST(Tcp_SendAndReceiveInBothDirectionsOnOneConnection)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	const std::vector<WireU8> request = Pattern(32, 1);
	const std::vector<WireU8> reply   = Pattern(48, 2);

	CHECK(client.Send(request.data(), request.size()).IsOk());
	CHECK(server.Send(reply.data(), reply.size()).IsOk());

	std::vector<WireU8> atClient;
	std::vector<WireU8> atServer;
	CHECK(Drain(client, atClient, reply.size()));
	CHECK(Drain(server, atServer, request.size()));
	CHECK(atClient == reply);
	CHECK(atServer == request);
}

// A zero-length send is a legitimate no-op, not a failure: callers build messages
// by appending, and an empty one is a thing they can hold.
MODERN_TEST(Tcp_ZeroLengthSendSucceedsAndDoesNothing)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	CHECK(client.Send(nullptr, 0).IsOk());
	CHECK_EQ(client.LastSendCount(), static_cast<std::size_t>(0));
}

// Nothing to read must be reported as "nothing yet", not as an error. An accept
// loop or a poll loop that had to distinguish the two would need a spurious error
// for every idle pass.
MODERN_TEST(Tcp_ReceiveWithNothingAvailableIsNotAnError)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	WireU8      buffer[64] = {};
	std::size_t got        = 0;
	CHECK(server.Receive(buffer, sizeof(buffer), got, 150).IsOk());
	CHECK_EQ(got, static_cast<std::size_t>(0));
	CHECK_EQ(server.Fault(), TransportFault::WouldBlock);
	// Still connected: nothing arrived, which is not a disconnect.
	CHECK(server.IsConnected());
}

// `received` must be set even when the receive is refused outright, so a caller
// reading it after an error gets zero rather than stack garbage.
MODERN_TEST(Tcp_ReceiveSetsTheLengthEvenWhenItFails)
{
	TcpTransport transport;
	WireU8      buffer[16] = {};
	std::size_t got        = 12345;  // deliberately pre-poisoned

	CHECK_EQ(transport.Receive(buffer, sizeof(buffer), got).GetCode(), ErrorCode::InvalidState);
	CHECK_EQ(got, static_cast<std::size_t>(0));
}

MODERN_TEST(Tcp_OperationsOnAnUnconnectedTransportAreRejected)
{
	TcpTransport transport;

	WireU8 buffer[8] = {};
	std::size_t got  = 0;
	CHECK_EQ(transport.Receive(buffer, sizeof(buffer), got).GetCode(), ErrorCode::InvalidState);
	CHECK_EQ(got, static_cast<std::size_t>(0));

	const WireU8 data[4] = { 1, 2, 3, 4 };
	CHECK_EQ(transport.Send(data, 4).GetCode(), ErrorCode::InvalidState);
	CHECK(!transport.IsConnected());
}

MODERN_TEST(Tcp_ReceiveRejectsANullDestinationWithANonZeroLength)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	std::size_t got = 0;
	CHECK_EQ(server.Receive(nullptr, 16, got).GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(got, static_cast<std::size_t>(0));
}

// ---------------------------------------------------------------------------
// Disconnect
// ---------------------------------------------------------------------------

// A peer's orderly close is zero bytes, and the transport must say WHY - because
// "the connection ended" and "nothing arrived yet" are both zero bytes and mean
// opposite things to a caller.
MODERN_TEST(Tcp_PeerDisconnectIsObservedAsZeroBytesWithPeerClosed)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	client.Disconnect();

	WireU8      buffer[64] = {};
	std::size_t got        = 0;
	CHECK(server.Receive(buffer, sizeof(buffer), got, kDeadline).IsOk());
	CHECK_EQ(got, static_cast<std::size_t>(0));
	CHECK_EQ(server.Fault(), TransportFault::PeerClosed);
	// The peer is gone, so this transport is no longer connected regardless of what
	// our own handle still says. Reporting Connected here would invite a send that
	// fails with a reset instead of a clean refusal.
	CHECK(!server.IsConnected());
}

MODERN_TEST(Tcp_DisconnectIsIdempotent)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	client.Disconnect();
	CHECK(!client.IsConnected());
	client.Disconnect();
	CHECK(!client.IsConnected());
}

// Repeated connect and disconnect on one object must neither leak handles nor
// hang. A transport that could only be used once would be useless to a client
// that reconnects after a dropped link.
MODERN_TEST(Tcp_RepeatedConnectAndDisconnectStaysUsable)
{
	TcpListener listener;
	const WireU16 port = BindEphemeralLoopback(listener);
	CHECK_NE(port, 0);

	Endpoint target;
	target.host = kLoopback;
	target.port = port;

	for (int attempt = 0; attempt < 5; ++attempt)
	{
		TcpTransport client;
		CHECK(client.Connect(target).IsOk());

		std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
		CHECK(listener.Accept(raw, kDeadline).IsOk());
		CHECK_NE(raw, TcpListener::kNoAcceptedSocket);

		Result<TcpTransport> adopted = TcpTransport::Adopt(raw);
		CHECK(adopted.IsOk());

		TcpTransport server = std::move(adopted.GetValue());
		CHECK(server.IsConnected());

		client.Disconnect();
		server.Disconnect();
	}
}

// Closing the listener must not disturb a connection it already accepted: a
// server shutting down stops accepting new work without cutting off work in
// progress.
MODERN_TEST(Tcp_ClosingTheListenerLeavesAcceptedConnectionsAlone)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	listener.Close();
	CHECK(!listener.IsListening());
	CHECK(client.IsConnected());
	CHECK(server.IsConnected());

	const std::vector<WireU8> sent = Pattern(16, 5);
	CHECK(client.Send(sent.data(), sent.size()).IsOk());

	std::vector<WireU8> got;
	CHECK(Drain(server, got, sent.size()));
	CHECK(got == sent);
}

// ---------------------------------------------------------------------------
// Ownership
// ---------------------------------------------------------------------------

// A moved-from transport owns nothing, so its destructor cannot close a socket the
// destination is now using - the double-close that makes a moved handle one of the
// two worst possible outcomes, the other being a leak.
MODERN_TEST(Tcp_MoveTransfersTheSocketAndLeavesTheSourceDisconnected)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	const std::uintptr_t before = client.LastSendCount();
	(void)before;

	TcpTransport moved = std::move(client);
	CHECK(moved.IsConnected());
	CHECK(moved.IsValid());
	CHECK(!client.IsConnected());
	CHECK(!client.IsValid());
	CHECK_EQ(client.Phase(), TcpTransportPhase::Disconnected);
	CHECK(server.IsConnected());
}

// Adopt must refuse the sentinel rather than producing a transport that reports
// Connected and then fails every operation.
MODERN_TEST(Tcp_AdoptRejectsTheSentinelHandle)
{
	Result<TcpTransport> adopted = TcpTransport::Adopt(TcpListener::kNoAcceptedSocket);
	CHECK(adopted.IsError());
	CHECK_EQ(adopted.GetError(), ErrorCode::InvalidArgument);
}

// An adopted socket is already connected, so it must report Connected - not
// Initialized, which would make State() report Closed and make Send refuse a
// perfectly good connection.
MODERN_TEST(Tcp_AdoptedTransportIsImmediatelyConnected)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	CHECK(server.IsConnected());
	CHECK_EQ(server.Phase(), TcpTransportPhase::Connected);
	CHECK(server.LocalEndpoint().host == std::string(kLoopback) ||
	      server.LocalEndpoint().host == std::string("127.0.0.1"));
}

// ---------------------------------------------------------------------------
// Multiple connections
// ---------------------------------------------------------------------------

// Three connections on one listener must not corrupt each other. Each carries a
// distinct payload, so a mix-up shows up as a mismatch rather than as a hang.
// Three connections on one listener must not corrupt each other. Each carries a
// distinct payload, so a mix-up shows up as a mismatch rather than as a hang.
MODERN_TEST(Tcp_ThreeConcurrentConnectionsKeepSeparateStreams)
{
	TcpListener listener;

	// Each pair owns BOTH of its sockets. The earlier version of this test connected
	// three transports in local variables and then sent through a different,
	// default-constructed transport inside each pair - so every send failed and the
	// test was asserting that three unconnected sockets do not interoperate.
	struct Pair
	{
		TcpTransport client;
		TcpTransport server;
		std::vector<WireU8> payload;
	};

	std::vector<Pair> pairs(3);
	{
		CHECK_NE(BindEphemeralLoopback(listener), 0);

		Endpoint target;
		target.host = kLoopback;
		target.port = listener.BoundEndpoint().port;

		for (int index = 0; index < 3; ++index)
		{
			pairs[index].payload = Pattern(64 + index * 32, static_cast<WireU8>(index + 1));
			CHECK(pairs[index].client.Connect(target).IsOk());
		}

		for (int index = 0; index < 3; ++index)
		{
			std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
			CHECK(listener.Accept(raw, kDeadline).IsOk());
			CHECK_NE(raw, TcpListener::kNoAcceptedSocket);

			Result<TcpTransport> adopted = TcpTransport::Adopt(raw);
			CHECK(adopted.IsOk());
			pairs[index].server = std::move(adopted.GetValue());
		}
	}

	// Every client sends on its own connection first, and only then is anything read.
	// Interleaving is what would expose a transport that shared one buffer between
	// connections.
	for (int index = 0; index < 3; ++index)
	{
		CHECK(pairs[index].client.Send(pairs[index].payload.data(),
		                               pairs[index].payload.size()).IsOk());
	}

	for (int index = 0; index < 3; ++index)
	{
		std::vector<WireU8> got;
		CHECK(Drain(pairs[index].server, got, pairs[index].payload.size()));
		CHECK(got == pairs[index].payload);
	}
}

// Dropping one client must not disturb the others.
MODERN_TEST(Tcp_ClosingOneConnectionLeavesTheOthersWorking)
{
	TcpListener listener;
	CHECK_NE(BindEphemeralLoopback(listener), 0);

	Endpoint target;
	target.host = kLoopback;
	target.port = listener.BoundEndpoint().port;

	TcpTransport clientOne;
	TcpTransport clientTwo;
	CHECK(clientOne.Connect(target).IsOk());
	CHECK(clientTwo.Connect(target).IsOk());

	std::uintptr_t rawOne = TcpListener::kNoAcceptedSocket;
	std::uintptr_t rawTwo = TcpListener::kNoAcceptedSocket;
	CHECK(listener.Accept(rawOne, kDeadline).IsOk());
	CHECK(listener.Accept(rawTwo, kDeadline).IsOk());

	Result<TcpTransport> adoptedOne = TcpTransport::Adopt(rawOne);
	Result<TcpTransport> adoptedTwo = TcpTransport::Adopt(rawTwo);
	CHECK(adoptedOne.IsOk());
	CHECK(adoptedTwo.IsOk());
	TcpTransport serverOne = std::move(adoptedOne.GetValue());
	TcpTransport serverTwo = std::move(adoptedTwo.GetValue());

	clientOne.Disconnect();

	const std::vector<WireU8> sent = Pattern(24, 9);
	CHECK(clientTwo.Send(sent.data(), sent.size()).IsOk());

	std::vector<WireU8> got;
	CHECK(Drain(serverTwo, got, sent.size()));
	CHECK(got == sent);
}

// ---------------------------------------------------------------------------
// Fragmentation, through the real socket
// ---------------------------------------------------------------------------

// One byte at a time, through a real socket, into the real ConnectionFramer.
//
// The loopback transport can already deliver a byte at a time deterministically,
// so what this adds is proof that a REAL socket's boundaries - which the OS picks,
// not the test - are handled by the same framer without a second parser appearing.
MODERN_TEST(Tcp_RealSocketFragmentationIsReassembledByTheFramer)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	MessageHeader header;
	header.type = 1542;  // NET_MSG_REQ_GAME_SVR
	const std::vector<WireU8> payload = Pattern(56, 0xA5);

	std::vector<WireU8> message;
	CHECK(Codec::EncodeMessage(header, payload, message).IsOk());
	CHECK_EQ(message.size(), kMessageHeaderSize + payload.size());

	// Deliberately unequal slices: a 1-byte prefix, then a header split across a
	// boundary, then the body in awkward pieces. Nothing here aligns with a message
	// boundary on purpose.
	const std::size_t slices[] = { 1, 3, 4, 8, 2, 16, 5, 32, 7 };
	std::size_t       offset  = 0;
	std::size_t       index   = 0;

	ConnectionFramer framer;
	Message          decoded;
	int              messagesFound = 0;

	while (offset < message.size() && index < 64)
	{
		const std::size_t take = std::min(slices[index % 9],
		                                  message.size() - offset);
		CHECK(client.Send(message.data() + offset, take).IsOk());
		offset += take;

		// Read back only what was just written, in one receive, so the framer is
		// genuinely fed in fragments rather than being handed the whole message at
		// the end.
		WireU8      buffer[64] = {};
		std::size_t got        = 0;
		CHECK(server.Receive(buffer, take, got, kDeadline).IsOk());
		CHECK_GT(got, static_cast<std::size_t>(0));

		CHECK_EQ(framer.Feed(buffer, got), FrameStatus::Ok);
		while (framer.Next(decoded) == FrameStatus::Ok)
		{
			++messagesFound;
		}
		CHECK(!framer.IsFailed());

		++index;
	}

	CHECK_EQ(messagesFound, 1);
	CHECK_EQ(static_cast<int>(decoded.header.type), 1542);
	CHECK_EQ(decoded.payload.size(), payload.size());
	CHECK(decoded.payload == payload);
	CHECK_EQ(framer.Buffered(), static_cast<std::size_t>(0));
}

// Three whole messages written as one buffer must arrive as three messages. This
// is the mirror of the fragmentation case: a parser that assumes one recv() is one
// message fails here, and a framer that is correct does not notice the difference.
// Three whole messages written as ONE buffer must arrive as three messages. This is
// the mirror of the fragmentation case: a parser that assumes one recv() is one
// message fails here, and a framer that is correct does not notice the difference.
//
// Note how the stream is built. Codec::EncodeMessage CLEARS its output vector
// before writing - it encodes one message, it does not append - so calling it three
// times into the same vector leaves only the last message. The earlier version of
// this test did exactly that and found 1 message where it expected 3, which reads
// like a framing bug and was not one.
MODERN_TEST(Tcp_SeveralMessagesInOneWriteAreFramedSeparately)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	std::vector<WireU8> stream;
	const MessageId     ids[] = { 1552, 1552, 1562 };
	for (MessageId id : ids)
	{
		MessageHeader header;
		header.type = id;
		const std::vector<WireU8> payload = Pattern(16, static_cast<WireU8>(id & 0xFF));

		std::vector<WireU8> encoded;
		CHECK(Codec::EncodeMessage(header, payload, encoded).IsOk());
		stream.insert(stream.end(), encoded.begin(), encoded.end());
	}
	CHECK_EQ(stream.size(), static_cast<std::size_t>(3 * (kMessageHeaderSize + 16)));

	CHECK(client.Send(stream.data(), stream.size()).IsOk());

	ConnectionFramer framer;
	Message          decoded;
	int              messagesFound = 0;

	std::vector<WireU8> chunk(4096);
	while (messagesFound < 3)
	{
		std::size_t got = 0;
		CHECK(server.Receive(chunk.data(), chunk.size(), got, 500).IsOk());
		if (got == 0)
		{
			break;
		}
		CHECK_EQ(framer.Feed(chunk.data(), got), FrameStatus::Ok);
		while (framer.Next(decoded) == FrameStatus::Ok)
		{
			// The ids must arrive in order, which is the part a shared buffer or a
			// swapped connection would break while the COUNT stayed at three.
			CHECK_EQ(static_cast<int>(decoded.header.type), static_cast<int>(ids[messagesFound]));
			++messagesFound;
		}
	}

	CHECK_EQ(messagesFound, 3);
}

// A header that cannot be valid must latch the framer into failure, so the owner
// drops the connection instead of resynchronising onto garbage.
MODERN_TEST(Tcp_MalformedHeaderLatchesTheFramerThroughARealSocket)
{
	TcpListener listener;
	TcpTransport client;
	TcpTransport server;
	CHECK(ConnectAndAccept(listener, client, server));

	// dwSize below the 8-byte header: impossible for any real message.
	WireU8 nonsense[kMessageHeaderSize] = { 0x01, 0x00, 0x00, 0x00,
	                                         0x62, 0x06, 0x00, 0x00 };
	CHECK(client.Send(nonsense, sizeof(nonsense)).IsOk());

	ConnectionFramer framer;
	Message          decoded;

	WireU8      buffer[64] = {};
	std::size_t got        = 0;
	CHECK(server.Receive(buffer, sizeof(buffer), got, kDeadline).IsOk());
	CHECK_GT(got, static_cast<std::size_t>(0));

	CHECK_EQ(framer.Feed(buffer, got), FrameStatus::Ok);
	CHECK_EQ(framer.Next(decoded), FrameStatus::InvalidLength);
	CHECK(framer.IsFailed());
}

// ---------------------------------------------------------------------------
// Address resolution
// ---------------------------------------------------------------------------

MODERN_TEST(Tcp_ResolveAcceptsNumericAndNamedLoopback)
{
	EnsureRuntime();

	SocketAddress::Ipv4Endpoint address;

	Endpoint numeric;
	numeric.host = "127.0.0.1";
	numeric.port = 5001;
	CHECK(SocketAddress::Resolve(numeric, address).IsOk());
	CHECK_EQ(SocketAddress::ToText(address), std::string("127.0.0.1"));

	Endpoint named;
	named.host = "localhost";
	named.port = 5001;
	CHECK(SocketAddress::Resolve(named, address).IsOk());
	// Whatever localhost meant, the answer must be a numeric IPv4 the RAN wire
	// could carry - never the name that was asked for.
	CHECK_EQ(SocketAddress::ToText(address), std::string("127.0.0.1"));
}

MODERN_TEST(Tcp_ResolveRejectsGarbageAndEmptyHosts)
{
	EnsureRuntime();

	SocketAddress::Ipv4Endpoint address;

	Endpoint empty;
	empty.port = 5001;
	CHECK(SocketAddress::Resolve(empty, address).GetCode() == ErrorCode::InvalidArgument);

	Endpoint garbage;
	garbage.host = "no-such-host.invalid";
	garbage.port = 5001;
	CHECK(SocketAddress::Resolve(garbage, address).GetCode() == ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// Diagnosis
// ---------------------------------------------------------------------------

// Every fault must render as a distinct, stable name. These strings end up in logs
// and in test output, so a name that changed or a fault that borrowed another's
// would be a quiet loss of diagnostic value.
MODERN_TEST(Tcp_EveryFaultHasADistinctName)
{
	const TransportFault all[] = {
		TransportFault::None,
		TransportFault::WouldBlock,
		TransportFault::Timeout,
		TransportFault::PeerClosed,
		TransportFault::ConnectionRefused,
		TransportFault::HostUnreachable,
		TransportFault::ConnectionReset,
		TransportFault::NotConnected,
		TransportFault::AddressInvalid,
		TransportFault::NetworkUnavailable,
		TransportFault::NotSupported,
		TransportFault::PartialSend,
		TransportFault::Unexpected,
	};

	for (std::size_t i = 0; i < sizeof(all) / sizeof(all[0]); ++i)
	{
		const std::string name = ToString(all[i]);
		CHECK(!name.empty());
		CHECK_NE(name, std::string("Unrecognised"));
		for (std::size_t j = i + 1; j < sizeof(all) / sizeof(all[0]); ++j)
		{
			CHECK_NE(name, std::string(ToString(all[j])));
		}
	}

	// A value outside the set must not borrow a real fault's name.
	CHECK_EQ(std::string(ToString(static_cast<TransportFault>(200))), std::string("Unrecognised"));
}

MODERN_TEST(Tcp_PhaseNamesAreDistinctAndStable)
{
	CHECK_EQ(std::string(ToString(TcpTransportPhase::Disconnected)), std::string("Disconnected"));
	CHECK_EQ(std::string(ToString(TcpTransportPhase::Initialized)), std::string("Initialized"));
	CHECK_EQ(std::string(ToString(TcpTransportPhase::Connecting)), std::string("Connecting"));
	CHECK_EQ(std::string(ToString(TcpTransportPhase::Connected)), std::string("Connected"));
	CHECK_EQ(std::string(ToString(TcpTransportPhase::Closing)), std::string("Closing"));
}

int main()
{
	// Unbuffered, so that a test which hangs names itself in the output rather
	// than taking its last line down with it. stdbuf does the same for a shell
	// pipeline, but a test binary should not depend on how it was launched.
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
