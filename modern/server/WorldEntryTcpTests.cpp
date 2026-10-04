// WORLD-ENTRY-001 Phase C: real Winsock TCP integration for world entry.
//
// THIS IS THE TEST THE PHASE EXISTS FOR. Phase A proved the layouts and Phase B
// proved the rules; both ran with no network at all. Neither can show that a
// client really opens a SECOND connection to the endpoint a packet named, and
// that is the claim this file exists to test.
//
// Proven HERE and nowhere else:
//
//   * two INDEPENDENT listeners bind OS-assigned ports, and the 2358 the client
//     receives names the Field role's real port - so the redirect controls where
//     the second connection goes;
//   * connection #1 and connection #2 are genuinely different sockets, evidenced
//     by two different OS-assigned source ports;
//   * 2359 travels on connection #2 and 2333 comes back on connection #2;
//   * the 2333 is decoded and every authoritative field is asserted, plus every
//     reserved region byte;
//   * the whole exchange survives reads and writes fragmented at 1, 2, 3, 4, 8,
//     16, 32, 64, 128 and 4096 bytes, with the boundaries chosen by neither side;
//   * malformed packets are refused with no crash, no hang and NO SPAWN, and the
//     server is still serving afterwards;
//   * a replayed 2359, a wrong gaeaId, a wrong slot, a wrong account and an
//     unknown authorization are all refused over the real socket;
//   * two clients do not share world-entry state in either direction;
//   * start -> use -> stop leaves nothing listening and no thread running.
//
// NOTHING HERE REACHES THE NETWORK. Every bind is 127.0.0.1:0. In particular
// nothing contacts 211.172.252.50:5001 or any other external host.
//
// EVERY BLOCKING CALL IS BOUNDED. A test that cannot finish is a test CI cannot
// trust, so both the server accepts and every client read carry a deadline, and a
// client that connects and then says nothing is refused rather than waited on.
//
// THREADS ARE THE TEST HARNESS'S, NOT THE SERVER'S. WorldServerRuntime is
// synchronous by construction, exactly as LOGIN-002's LoginServerRuntime is. A
// client that connects and then WAITS for a reply cannot be served from the same
// thread, so each role is served on its own thread - standing in for the process
// boundary a real deployment has. The handshake completes in the kernel without
// anyone calling accept(), which is why connect-then-accept from two threads is
// safe and deadlock-free.

#include "TestHarness.h"

#include "CharacterListProtocol.h"
#include "CompressionCodec.h"
#include "NetworkCodec.h"
#include "TcpListener.h"
#include "TcpTransport.h"
#include "WorldEntryProtocol.h"
#include "login/LoginReceiver.h"
#include "login/World001LoginClient.h"
#include "world/AgentRoleRuntime.h"
#include "world/CharacterRepository.h"
#include "world/FieldRoleRuntime.h"
#include "world/WorldCharacter.h"
#include "world/WorldEntryService.h"
#include "world/WorldServerConfig.h"
#include "world/WorldServerRuntime.h"
#include "world/WorldEntryClient.h"
#include "world/WorldEntryConnections.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ModernTests
{
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
	struct Describer<Modern::Server::World::AgentRefusal>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::Server::World::AgentRefusal& value)
		{
			return Modern::Server::World::ToString(value);
		}
	};

	template <>
	struct Describer<Modern::Server::World::FieldRefusal>
	{
		static constexpr bool Known = true;
		static std::string Get(const Modern::Server::World::FieldRefusal& value)
		{
			return Modern::Server::World::ToString(value);
		}
	};
}

namespace
{
	using namespace Modern;
	using namespace Modern::Network;
	using namespace Modern::Server;
	using namespace Modern::Server::World;
	using namespace Modern::Client;

	constexpr const char* kLoopback = "127.0.0.1";

	// Long enough that a loaded machine does not produce a false failure, short
	// enough that a genuine hang fails the test rather than the run.
	constexpr int kDeadline = 8000;

	// The fragmentation sizes the brief asks for. 1 is the important one: it proves
	// ConnectionFramer - not luck - is what reassembles a message.
	constexpr std::size_t kFragmentSizes[] = { 1, 2, 3, 4, 8, 16, 32, 64, 128, 4096 };

	// ---- the world ---------------------------------------------------------

	constexpr WorldAccountId   kAccountA{ 1001 };
	constexpr WorldAccountId   kAccountB{ 2002 };
	constexpr WorldCharacterId kCharA1{ 5001 };
	constexpr WorldCharacterId kCharA2{ 5002 };
	constexpr WorldCharacterId kCharB1{ 6001 };

	constexpr const char* kUserA = "user_alpha";
	constexpr const char* kUserB = "user_beta";
	constexpr const char* kPassA = "pass_alpha";
	constexpr const char* kPassB = "pass_beta";

	WorldCharacter MakeCharacter(WorldCharacterId id, WorldAccountId accountId,
	                             std::string name, std::string userId, WireU16 level)
	{
		WorldCharacter character;
		character.id             = id;
		character.accountId      = accountId;
		character.userId         = std::move(userId);
		character.name           = std::move(name);
		character.characterClass = 3;
		character.school         = 1;
		character.level          = level;

		character.hp = { 300u * level, 300u * level };
		character.mp = { 150u * level, 150u * level };
		character.sp = { 80u * level, 80u * level };

		character.saveMapId.value = 7u;
		character.savePosition   = { 100.5f, -20.25f, 3.75f };
		return character;
	}

	// A World Server on two OS-assigned loopback ports, with both roles served on
	// background threads.
	//
	// The events are COLLECTED rather than printed, which is what lets a test assert
	// that the server did something - a log line nobody reads cannot be asserted on.
	class TestWorldServer
	{
	public:
		TestWorldServer()
			: m_runtime(MakeConfig(), m_authenticator, m_repository)
		{
			// Populated in the body, not in the initialiser list.
			//
			// InMemoryCharacterRepository holds a mutex and is therefore not copyable, so
			// the old `m_repository(MakeRepository())` no longer compiles. Filling the
			// member in place is the fix, and it is also the honest one: the repository is
			// a member whose lifetime the runtime already borrows by reference, so it was
			// never really being copied in - only moved through a return value that
			// happened to elide.
			PopulateRepository(m_repository);

			// The credentials the test client sends.
			//
			// Without these the authenticator refuses every login, the Agent role answers
			// with a LOGIN_FB failure and closes, and the client fails at "await login".
			// That is the CORRECT behaviour and a thoroughly confusing thing to debug -
			// it looks like a framing bug - so the accounts are registered where a reader
			// can see them.
			m_authenticator.AddAccount(kUserA, kPassA);
			m_authenticator.AddAccount(kUserB, kPassB);
		}

		static void PopulateRepository(InMemoryCharacterRepository& repository)
		{
			(void) repository.Add(MakeCharacter(kCharA1, kAccountA, "Alpha", kUserA, 10));
			(void) repository.Add(MakeCharacter(kCharA2, kAccountA, "Alpine", kUserA, 20));
			(void) repository.Add(MakeCharacter(kCharB1, kAccountB, "Beta", kUserB, 30));
		}

		static WorldServerConfig MakeConfig()
		{
			WorldServerConfig config = WorldServerConfig::MakeLoopback();
			config.accounts.push_back({ kUserA, kAccountA });
			config.accounts.push_back({ kUserB, kAccountB });
			return config;
		}

		Status Start()
		{
			const Status status = m_runtime.Start();
			if (status.IsOk())
			{
				StartServing();
			}
			return status;
		}

		// Serves the Agent role until Stop. One thread, because the Agent conversation
		// is still short-lived: 2358 then 2359 then close.
		//
		// There is deliberately NO Field thread here. WORLD-ENTRY-002a made the Field
		// connection long-lived, so FieldRoleRuntime owns its own accept thread and one
		// worker per connection from Start(). A pump loop driving it from here would be
		// both redundant and wrong - a single-threaded pump could not serve two clients
		// that are each holding a Field connection open waiting for the other's 3033.
		void StartServing()
		{
			m_agentThread = std::thread([this] {
				while (!m_stop.load(std::memory_order_acquire))
				{
					if (m_runtime.ServeOneAgentClient(200).IsError())
					{
						return;
					}
				}
			});
		}

		// Joins the Agent thread and then stops the runtime, which is what joins the
		// Field role's accept and worker threads. Called before any assertion on the
		// final counters, because those counters are written by the server threads and
		// reading them while those run would be reading a value mid-update.
		void Stop()
		{
			m_stop.store(true, std::memory_order_release);
			if (m_agentThread.joinable())
			{
				m_agentThread.join();
			}
			m_runtime.Stop();
		}

		~TestWorldServer() { Stop(); }

		EndpointAddress AgentAddress() const
		{
			EndpointAddress address;
			address.ip   = m_runtime.AgentEndpoint().host;
			address.port = m_runtime.AgentEndpoint().port;
			return address;
		}

		// The Field endpoint the SERVER published, for the 2358-controls-connection-2
		// assertion.
		FieldRedirect ExpectedRedirect() const
		{
			FieldRedirect redirect;
			redirect.fieldIp     = m_runtime.FieldBoundEndpoint().host;
			redirect.servicePort = static_cast<WireI32>(m_runtime.FieldBoundEndpoint().port);
			return redirect;
		}

		WorldServerRuntime& Runtime() noexcept { return m_runtime; }
		InMemoryCharacterRepository& Repository() noexcept { return m_repository; }

		// Read only after Stop() has joined the server thread - see the test.
		std::size_t FieldSpawnedCount() noexcept
		{
			return m_runtime.Field().ServedClientCount();
		}

	private:
		InMemoryLoginAuthenticator m_authenticator;
		InMemoryCharacterRepository m_repository;
		WorldServerRuntime          m_runtime;

		std::thread m_agentThread;
		std::atomic<bool> m_stop{false};
	};

	// A full client flow over two real sockets.
	//
	// `fragment` is the read size both connections use; 0 means "whatever the
	// transport decides".
	struct ClientOutcome
	{
		bool          agentOk = false;
		bool          fieldOk = false;
		std::string   failure;

		WireU16 agentSourcePort = 0;
		WireU16 fieldSourcePort = 0;

		WireU32 gaeaId      = 0;
		WireU16 agentCount  = 0;
		bool    sawRedirect = false;
		bool    spawnSeen   = false;
		std::size_t detailsSeen = 0;
		WorldSpawnState spawn;
	};

	// Drives one client from login to spawn.
	//
	// `selectId` is the character this client claims, which may deliberately be one
	// it does not own.
	ClientOutcome RunClientFlow(TestWorldServer& server, const std::string& userId,
	                             const std::string& password, WireU32 selectId,
	                             std::size_t fragment, bool requestDetail = true)
	{
		ClientOutcome outcome;

		MinLzo1xCodec codec;
		WorldEntryClient protocol(codec);

		// ---- connection #1: the Agent ------------------------------------
		AgentConnection agent(protocol);
		if (const Status status = agent.Connect(server.AgentAddress(), kDeadline);
		    status.IsError())
		{
			outcome.failure = "agent connect: ";
			outcome.failure += status.GetMessage();
			return outcome;
		}
		outcome.agentSourcePort = agent.LocalEndpoint().port;

		LoginRequestData login;
		login.userId   = userId;
		login.password = password;

		std::vector<WireU8> request;
		if (const Status status = protocol.BuildLogin(login, request); status.IsError())
		{
			outcome.failure = "build login: ";
			outcome.failure += status.GetMessage();
			return outcome;
		}
		if (const Status status = agent.Send(request); status.IsError())
		{
			outcome.failure = "send login: ";
			outcome.failure += status.GetMessage();
			return outcome;
		}
		if (const Status status = agent.PumpUntil(WorldEntryPhase::Authenticated,
		                                          kDeadline, fragment);
		    status.IsError())
		{
			outcome.failure = "await login: ";
			outcome.failure += status.GetMessage();
			return outcome;
		}
		if (protocol.Phase() == WorldEntryPhase::LoginRejected)
		{
			outcome.failure = "login rejected by server";
			return outcome;
		}

		std::vector<WireU8> listRequest;
		if (const Status status = protocol.BuildRequestCharacterList(listRequest);
		    status.IsError())
		{
			outcome.failure = "build 2247: ";
			outcome.failure += status.GetMessage();
			return outcome;
		}
		if (const Status status = agent.Send(listRequest); status.IsError())
		{
			outcome.failure = "send 2247";
			return outcome;
		}
		if (const Status status = agent.PumpUntil(WorldEntryPhase::CharacterListReady,
		                                          kDeadline, fragment);
		    status.IsError())
		{
			outcome.failure = "await 2248: ";
			outcome.failure += status.GetMessage();
			return outcome;
		}
		outcome.agentCount = static_cast<WireU16>(protocol.ExpectedDetailCount());

		// One 2332, so the 1176-byte per-character packet really crosses a socket
		// rather than being assumed to.
		//
		// No read loop here: a 2332 does not change the client's phase, so it is
		// applied by the LATER PumpUntil(RedirectReceived) below, when its bytes arrive
		// on this same connection. Reading for it separately would need a second way to
		// say "wait for a message that is not a state change", and one pump is enough.
		if (requestDetail && !protocol.CharacterIds().ids.empty())
		{
			std::vector<WireU8> detailRequest;
			if (protocol.BuildRequestCharacterDetail(protocol.CharacterIds().ids[0],
			                                         detailRequest)
			        .IsOk())
			{
				const Status sentDetail = agent.Send(detailRequest);
			}
		}
		std::vector<WireU8> select;
		if (const Status status = protocol.BuildSelectCharacter(selectId, select);
		    status.IsError())
		{
			outcome.failure = "build 2353: ";
			outcome.failure += status.GetMessage();
			return outcome;
		}
		if (const Status status = agent.Send(select); status.IsError())
		{
			outcome.failure = "send 2353";
			return outcome;
		}
		if (const Status status =
		        agent.PumpUntil(WorldEntryPhase::RedirectReceived, kDeadline, fragment);
		    status.IsError())
		{
			outcome.failure = "await 2358: ";
			outcome.failure += status.GetMessage();

			return outcome;
		}

		outcome.sawRedirect = protocol.HasRedirect();
		outcome.gaeaId      = protocol.Redirect().gaeaId;
		outcome.agentOk     = true;

		// The Agent role closes after the 2358, so the first connection is done. It is
		// NOT reused and its transport is not moved anywhere.
		agent.Disconnect();

		// ---- connection #2: the Field ------------------------------------
		FieldConnection field(protocol);
		if (const Status status = field.Connect(protocol.Redirect(), kDeadline);
		    status.IsError())
		{
			outcome.failure = "field connect: ";
			outcome.failure += status.GetMessage();
			return outcome;
		}
		outcome.fieldSourcePort = field.LocalEndpoint().port;

		FieldIdentity identity;
		identity.joinType       = WorldEntry::kJoinTypeFirst;
		identity.gaeaId         = protocol.Redirect().gaeaId;
		identity.slotFieldAgent = protocol.Redirect().slotFieldAgent;
		identity.cryptKey       = RanWire::DefaultCryptKey();

		if (const Status status = field.SendIdentity(identity); status.IsError())
		{
			outcome.failure = "send 2359: ";
			outcome.failure += status.GetMessage();
			return outcome;
		}
		if (const Status status = field.PumpUntilSpawn(kDeadline, fragment); status.IsError())
		{
			outcome.failure = "await 2333: ";
			outcome.failure += status.GetMessage();
			return outcome;
		}

		outcome.detailsSeen = protocol.CharacterDetails().size();
		outcome.spawn     = protocol.Spawn();
		outcome.spawnSeen = outcome.spawn.received;
		outcome.fieldOk   = true;

		field.Disconnect();
		return outcome;
	}

	// ---------------------------------------------------------------------------

	// The headline test: the whole flow, over two real sockets, with every
	// authoritative field of the 2333 inspected.
	MODERN_TEST(WorldEntry_PhaseC_FullFlowOverTwoRealSockets)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		// The two roles bound DIFFERENT OS-assigned ports. If they collided the second
		// bind would have failed, so this is an assertion that the topology is two
		// listeners rather than one.
		CHECK_NE(server.Runtime().AgentEndpoint().port,
		         server.Runtime().FieldBoundEndpoint().port);
		CHECK_NE(server.AgentAddress().port, server.ExpectedRedirect().servicePort);

		const ClientOutcome outcome =
		    RunClientFlow(server, kUserA, kPassA, kCharA1.value, 0);
		CHECK_EQ(outcome.failure, std::string());
		CHECK(outcome.agentOk);
		CHECK(outcome.fieldOk);

		// ---- the 2358 named the Field role's REAL endpoint ---------------
		//
		// Not "the client connected somewhere" - the endpoint in the packet IS the port
		// this process bound for its Field role. If the client had a hard-coded or
		// default port, this would fail.
		CHECK(outcome.sawRedirect);
		CHECK(outcome.gaeaId != 0);

		// ---- TWO REAL CONNECTIONS ----------------------------------------
		//
		// Two connections from one client to one host get two OS-assigned source ports.
		// Equal ports would mean one socket was reused, which is precisely what the
		// brief forbids.
		CHECK_NE(outcome.agentSourcePort, outcome.fieldSourcePort);
		CHECK(outcome.agentSourcePort != 0);
		CHECK(outcome.fieldSourcePort != 0);

		// ---- the character list and one detail crossed a socket ------------
		CHECK_EQ(static_cast<int>(outcome.agentCount), 2);
		CHECK_EQ(outcome.detailsSeen, static_cast<std::size_t>(1));

		// ---- the 2333, decoded and inspected ------------------------------
		CHECK(outcome.spawnSeen);
		CHECK_EQ(outcome.spawn.frame.size(), static_cast<std::size_t>(1022));
		CHECK_EQ(outcome.spawn.gaeaId, outcome.gaeaId);
		CHECK_EQ(outcome.spawn.characterId, kCharA1.value);
		CHECK_EQ(outcome.spawn.characterName, std::string("Alpha"));
		CHECK_EQ(outcome.spawn.accountId, kAccountA.value);
		CHECK_EQ(outcome.spawn.characterClass, 3u);
		CHECK_EQ(static_cast<int>(outcome.spawn.school), 1);
		CHECK_EQ(static_cast<int>(outcome.spawn.level), 10);
		CHECK_EQ(outcome.spawn.hp, 3000u);
		CHECK_EQ(outcome.spawn.mp, 1500u);
		CHECK_EQ(outcome.spawn.sp, 800u);
		CHECK_EQ(outcome.spawn.mapId, 7u);
		CHECK_EQ(outcome.spawn.positionX, 100.5f);
		CHECK_EQ(outcome.spawn.positionY, -20.25f);
		CHECK_EQ(outcome.spawn.positionZ, 3.75f);
		CHECK_EQ(outcome.spawn.userId, std::string(kUserA));

		// ---- reserved regions, byte by byte, from real socket bytes -------
		const std::vector<WireU8>& frame = outcome.spawn.frame;
		const std::size_t record = WorldEntry::kSpawnOffsetData;
		for (std::size_t i = 0; i < WorldEntry::kSpawnReservedSlotArraySize; ++i)
		{
			CHECK_EQ(static_cast<int>(frame[WorldEntry::kSpawnReservedSlotArrayOffset + i]),
			         0);
		}
		for (std::size_t i = 0; i < WorldEntry::kSpawnReservedCountsSize; ++i)
		{
			CHECK_EQ(static_cast<int>(frame[WorldEntry::kSpawnReservedCountsOffset + i]), 0);
		}
		for (std::size_t i = 0; i < WorldEntry::kRecordReservedMidSize; ++i)
		{
			CHECK_EQ(static_cast<int>(frame[record + WorldEntry::kRecordReservedMidOffset + i]),
			         0);
		}

		// The server is stopped - and therefore JOINED - before any counter is read.
		// Those counters are written by the server threads; reading one while a thread
		// is still running it is a data race, and a reallocation-free read is no
		// consolation when the value itself may be mid-update.
		server.Stop();

		// Exactly one spawn: the Field role served one connection and sent one 2333.
		CHECK_EQ(server.FieldSpawnedCount(), static_cast<std::size_t>(1));
		CHECK_EQ(server.Runtime().Field().RefusedClientCount(), static_cast<std::size_t>(0));
	}
// ---------------------------------------------------------------------------
	// A raw client, for the tests that must send what a correct client never would.
	// ---------------------------------------------------------------------------
	//
	// Teaching the production client a "send garbage" mode would be worse than a raw
	// socket in a test, so malformed input gets a socket of its own.
	class RawClient
	{
	public:
		Status Connect(const EndpointAddress& target, int timeout = kDeadline)
		{
			Endpoint endpoint;
			endpoint.host = target.ip;
			endpoint.port = target.port;
			return m_transport.Connect(endpoint, timeout);
		}

		Status Send(const std::vector<WireU8>& bytes)
		{
			return m_transport.Send(bytes.data(), bytes.size());
		}

		// Closes mid-message, which is what a client that gave up looks like on the wire.
		void HangUp() noexcept { m_transport.Disconnect(); }

		~RawClient() { m_transport.Disconnect(); }

	private:
		TcpTransport m_transport;
	};

	// The endpoint a 2358 names, as a connectable address.
	//
	// The CLIENT side of this conversion is FieldConnection::Connect, which reads the
	// same two fields straight out of the redirect. Having a test helper do it too
	// keeps the raw-client tests dialing exactly where a real client would.
	EndpointAddress ToEndpointAddress(const FieldRedirect& redirect)
	{
		EndpointAddress address;
		address.ip   = redirect.fieldIp;
		address.port = static_cast<WireU16>(redirect.servicePort);
		return address;
	}

	// The identity a client builds from a 2358.
	FieldIdentity IdentityFrom(const FieldRedirect& redirect)
	{
		FieldIdentity identity;
		identity.joinType       = WorldEntry::kJoinTypeFirst;
		identity.gaeaId         = redirect.gaeaId;
		identity.slotFieldAgent = redirect.slotFieldAgent;
		identity.cryptKey       = RanWire::DefaultCryptKey();
		return identity;
	}

	// Drives the Agent conversation only, and hands back the 2358.
	//
	// Split out from RunClientFlow because the Field-side tests need a VALID redirect
	// before they can send an INVALID identity - and a test that manufactured its own
	// redirect would not be exercising the Agent's authorization at all.
	struct AgentOnlyResult
	{
		bool          ok = false;
		std::string   failure;
		FieldRedirect redirect;
		WireU32       gaeaId = 0;
		WireU16       characterCount = 0;
	};

	AgentOnlyResult RunAgentConversation(TestWorldServer& server, const std::string& userId,
	                                     const std::string& password, WireU32 selectId,
	                                     std::size_t fragment = 0)
	{
		AgentOnlyResult result;

		MinLzo1xCodec    codec;
		WorldEntryClient protocol(codec);
		AgentConnection  agent(protocol);

		if (const Status status = agent.Connect(server.AgentAddress(), kDeadline);
		    status.IsError())
		{
			result.failure = "agent connect: ";
			result.failure += status.GetMessage();
			return result;
		}

		LoginRequestData login;
		login.userId   = userId;
		login.password = password;

		std::vector<WireU8> request;
		if (const Status status = protocol.BuildLogin(login, request); status.IsError())
		{
			result.failure = "build login";
			return result;
		}
		if (const Status status = agent.Send(request); status.IsError())
		{
			result.failure = "send login";
			return result;
		}
		if (const Status status =
		        agent.PumpUntil(WorldEntryPhase::Authenticated, kDeadline, fragment);
		    status.IsError() || protocol.Phase() == WorldEntryPhase::LoginRejected)
		{
			result.failure = "await login";
			return result;
		}

		std::vector<WireU8> listRequest;
		if (const Status status = protocol.BuildRequestCharacterList(listRequest);
		    status.IsError())
		{
			result.failure = "build 2247";
			return result;
		}
		if (const Status status = agent.Send(listRequest); status.IsError())
		{
			result.failure = "send 2247";
			return result;
		}
		if (const Status status =
		        agent.PumpUntil(WorldEntryPhase::CharacterListReady, kDeadline, fragment);
		    status.IsError())
		{
			result.failure = "await 2248";
			return result;
		}
		result.characterCount = static_cast<WireU16>(protocol.ExpectedDetailCount());

		std::vector<WireU8> select;
		if (const Status status = protocol.BuildSelectCharacter(selectId, select);
		    status.IsError())
		{
			result.failure = "build 2353";
			return result;
		}
		if (const Status status = agent.Send(select); status.IsError())
		{
			result.failure = "send 2353";
			return result;
		}
		if (const Status status =
		        agent.PumpUntil(WorldEntryPhase::RedirectReceived, kDeadline, fragment);
		    status.IsError())
		{
			result.failure = "await 2358";
			return result;
		}

		result.redirect = protocol.Redirect();
		result.gaeaId   = protocol.Redirect().gaeaId;
		result.ok       = true;

		agent.Disconnect();
		return result;
	}

	// =========================================================================
	// Fragmentation
	// =========================================================================

	// The whole flow at every requested read size.
	//
	// 1 byte is the important one: it proves ConnectionFramer - and not luck, and not
	// a message that happened to arrive whole - is what reassembles a 28-byte list and
	// a 1022-byte spawn. The boundaries are chosen by NEITHER side: the client asks
	// for at most N bytes and the kernel decides what it gets.
	MODERN_TEST(WorldEntry_PhaseC_WholeFlowSurvivesEveryFragmentSize)
	{
		for (const std::size_t fragment : kFragmentSizes)
		{
			TestWorldServer server;
			CHECK(server.Start().IsOk());


			const ClientOutcome outcome =
			    RunClientFlow(server, kUserA, kPassA, kCharA1.value, fragment);


			if (!outcome.failure.empty())
			{
				// The fragment size is in the message, because "it failed" without saying
				// at which size is the least useful possible failure report.
				CHECK_EQ("fragment " + std::to_string(fragment) + ": " + outcome.failure,
				         std::string());
			}

			CHECK(outcome.agentOk);
			CHECK(outcome.fieldOk);
			CHECK(outcome.spawnSeen);
			CHECK_EQ(outcome.spawn.frame.size(), static_cast<std::size_t>(1022));
			CHECK_EQ(outcome.spawn.characterId, kCharA1.value);
			CHECK_EQ(outcome.spawn.characterName, std::string("Alpha"));

			server.Stop();
		}
	}

	// =========================================================================
	// Malformed packets on the Agent path
	// =========================================================================

	// Every refusal below must be safe: no crash, no hang, no spawn, and the server
	// still serving afterwards.
	//
	// "Still serving" is the part a crash-only test would miss - a server that dies on
	// malformed input has refused nothing, it has simply stopped existing.
	MODERN_TEST(WorldEntry_PhaseC_AgentRefusesMalformedInputAndKeepsServing)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		const EndpointAddress agent = server.AgentAddress();

		// A 2247 before authentication. The session state forbids it, and that is a
		// refusal rather than a crash or a character list.
		{
			RawClient raw;
			CHECK(raw.Connect(agent).IsOk());
			std::vector<WireU8> request;
			CHECK(CharacterListCodec::AppendRequestAll(request).IsOk());
			CHECK(raw.Send(request).IsOk());
			raw.HangUp();
		}

		// A 2353 with no prior login at all.
		{
			RawClient raw;
			CHECK(raw.Connect(agent).IsOk());
			std::vector<WireU8> request;
			CHECK(WorldEntryCodec::AppendGameJoin(request, static_cast<WireI32>(kCharA1.value))
			          .IsOk());
			CHECK(raw.Send(request).IsOk());
			raw.HangUp();
		}

		// Zero-length payload: a header promising nothing.
		{
			RawClient raw;
			CHECK(raw.Connect(agent).IsOk());
			std::vector<WireU8> frame;
			(void)Codec::WriteU32(frame, 0);
			(void)Codec::WriteU32(frame, 0);
			CHECK(raw.Send(frame).IsOk());
			raw.HangUp();
		}

		// Truncated header: half of an 8-byte header, then hang up.
		{
			RawClient raw;
			CHECK(raw.Connect(agent).IsOk());
			const std::vector<WireU8> partial = { 0x08, 0x00, 0x00 };
			CHECK(raw.Send(partial).IsOk());
			raw.HangUp();
		}

		// Truncated payload: a 2248 header promising 76 bytes with 12 delivered.
		{
			RawClient raw;
			CHECK(raw.Connect(agent).IsOk());
			std::vector<WireU8> frame;
			(void)Codec::WriteU32(frame, 76);
			(void)Codec::WriteU32(frame, CharacterList::kAllInfoId);
			(void)Codec::WriteU32(frame, 1);
			CHECK(raw.Send(frame).IsOk());
			raw.HangUp();
		}

		// Oversized declared packet. The framer refuses it, and the server drops the
		// connection rather than growing a buffer to whatever a peer claims.
		{
			RawClient raw;
			CHECK(raw.Connect(agent).IsOk());
			std::vector<WireU8> frame;
			(void)Codec::WriteU32(frame, 0x00FFFFFF);
			(void)Codec::WriteU32(frame, CharacterList::kAllInfoId);
			CHECK(raw.Send(frame).IsOk());
			raw.HangUp();
		}

		// A wrong message id entirely: not this conversation.
		{
			RawClient raw;
			CHECK(raw.Connect(agent).IsOk());
			std::vector<WireU8> frame;
			(void)Codec::WriteU32(frame, 8);
			(void)Codec::WriteU32(frame, 1555);
			CHECK(raw.Send(frame).IsOk());
			raw.HangUp();
		}

		// A 2247 with the wrong length: right id, wrong shape.
		{
			RawClient raw;
			CHECK(raw.Connect(agent).IsOk());
			std::vector<WireU8> frame;
			(void)Codec::WriteU32(frame, 12);
			(void)Codec::WriteU32(frame, CharacterList::kRequestAllId);
			(void)Codec::WriteU32(frame, 0);
			CHECK(raw.Send(frame).IsOk());
			raw.HangUp();
		}

		// STILL SERVING. A correct client must complete the whole flow after all of the
		// above, which is the assertion that matters: a server that merely avoided
		// crashing is not the same as a server that refused.
		const ClientOutcome after = RunClientFlow(server, kUserA, kPassA, kCharA1.value, 0);
		CHECK_EQ(after.failure, std::string());
		CHECK(after.fieldOk);
		CHECK(after.spawnSeen);
		CHECK_EQ(after.spawn.characterName, std::string("Alpha"));

		server.Stop();
	}

	// =========================================================================
	// Field path: malformed, replay, and every wrong identity
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseC_FieldRefusesEveryWrongIdentityOverRealSockets)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		// A REAL authorization, produced by a real Agent conversation. Every refusal
		// below is a variation on it, so each proves the Field is checking against
		// something genuine rather than against a hand-built value.
		const AgentOnlyResult authorized =
		    RunAgentConversation(server, kUserA, kPassA, kCharA1.value);
		CHECK(authorized.ok);
		CHECK_EQ(authorized.failure, std::string());
		CHECK(authorized.redirect.gaeaId != 0);

		// THE 2358 NAMES THE FIELD ROLE'S REAL ENDPOINT. Asserted against the port the
		// process actually bound, not against a constant - which is what makes "2358
		// controls the second connection" a claim rather than a hope.
		CHECK_EQ(authorized.redirect.fieldIp, server.ExpectedRedirect().fieldIp);
		CHECK_EQ(static_cast<int>(authorized.redirect.servicePort),
		         static_cast<int>(server.ExpectedRedirect().servicePort));

		const EndpointAddress field = ToEndpointAddress(authorized.redirect);

		// ---- an identity the Agent never authorized ------------------------
		{
			RawClient raw;
			CHECK(raw.Connect(field).IsOk());
			FieldIdentity identity = IdentityFrom(authorized.redirect);
			identity.gaeaId = authorized.redirect.gaeaId + 999u;
			std::vector<WireU8> request;
			CHECK(WorldEntryCodec::AppendFieldIdentity(request, identity).IsOk());
			CHECK(raw.Send(request).IsOk());
			raw.HangUp();
		}

		// ---- a valid gaeaId with the WRONG SLOT ---------------------------
		//
		// The pair is the credential, so both halves must match. A client holding one
		// correct half and guessing the other gets nothing.
		{
			RawClient raw;
			CHECK(raw.Connect(field).IsOk());
			FieldIdentity identity = IdentityFrom(authorized.redirect);
			identity.slotFieldAgent = authorized.redirect.slotFieldAgent + 1u;
			std::vector<WireU8> request;
			CHECK(WorldEntryCodec::AppendFieldIdentity(request, identity).IsOk());
			CHECK(raw.Send(request).IsOk());
			raw.HangUp();
		}

		// ---- an unknown join type -----------------------------------------
		{
			RawClient raw;
			CHECK(raw.Connect(field).IsOk());
			FieldIdentity identity = IdentityFrom(authorized.redirect);
			identity.joinType = 77;
			std::vector<WireU8> request;
			CHECK(WorldEntryCodec::AppendFieldIdentity(request, identity).IsOk());
			CHECK(raw.Send(request).IsOk());
			raw.HangUp();
		}

		// ---- a 2359 of the wrong LENGTH ------------------------------------
		//
		// Right id, 12 bytes instead of 24. Phase A's decoder requires exactly 24, so
		// the Field role refuses before reaching any validation.
		{
			RawClient raw;
			CHECK(raw.Connect(field).IsOk());
			std::vector<WireU8> frame;
			(void)Codec::WriteU32(frame, 12);
			(void)Codec::WriteU32(frame, WorldEntry::kJoinFieldId);
			(void)Codec::WriteU32(frame, 0);
			(void)Codec::WriteU32(frame, 0);
			CHECK(raw.Send(frame).IsOk());
			raw.HangUp();
		}

		// ---- a non-2359 message on the Field connection --------------------
		{
			RawClient raw;
			CHECK(raw.Connect(field).IsOk());
			std::vector<WireU8> frame;
			(void)Codec::WriteU32(frame, 8);
			(void)Codec::WriteU32(frame, WorldEntry::kCharacterJoinId);
			CHECK(raw.Send(frame).IsOk());
			raw.HangUp();
		}

		// ---- zero-length payload on the Field connection -------------------
		{
			RawClient raw;
			CHECK(raw.Connect(field).IsOk());
			const std::vector<WireU8> nothing;
			CHECK(raw.Send(nothing).IsOk());
			raw.HangUp();
		}

		// ---- THE REAL ONE USES IT ------------------------------------------
		//
		// After every refusal above, the genuine authorization still works exactly once.
		// If any of those refusals had CONSUMED it this would fail - which is what makes
		// "a refused attempt must not spend the client's one claim" testable.
		{
			MinLzo1xCodec    codec;
			WorldEntryClient protocol(codec);
			FieldConnection  connection(protocol);

			CHECK(connection.Connect(authorized.redirect, kDeadline).IsOk());
			CHECK(connection.SendIdentity(IdentityFrom(authorized.redirect)).IsOk());
			CHECK(connection.PumpUntilSpawn(kDeadline).IsOk());

			CHECK(protocol.Spawn().received);
			CHECK_EQ(protocol.Spawn().frame.size(), static_cast<std::size_t>(1022));
			CHECK_EQ(protocol.Spawn().characterId, kCharA1.value);
			CHECK_EQ(protocol.Spawn().characterName, std::string("Alpha"));
			CHECK_EQ(protocol.Spawn().gaeaId, authorized.redirect.gaeaId);
			connection.Disconnect();
		}

		// ---- REPLAY: the same pair again, on a fresh connection ------------
		//
		// The whole point of consuming an authorization. A captured 2359 is worth
		// nothing a second time.
		{
			MinLzo1xCodec    codec;
			WorldEntryClient protocol(codec);
			FieldConnection  replay(protocol);

			CHECK(replay.Connect(authorized.redirect, kDeadline).IsOk());
			CHECK(replay.SendIdentity(IdentityFrom(authorized.redirect)).IsOk());

			// PumpUntilSpawn fails: the server sends nothing at all, so there is no spawn
			// to receive. Asserting the ABSENCE of a spawn is the assertion - a test that
			// only checked "it returned" would pass against a server that sent a second
			// spawn.
			CHECK(replay.PumpUntilSpawn(2000).IsError());
			CHECK(!protocol.Spawn().received);
			replay.Disconnect();
		}

		server.Stop();

		// Exactly ONE Field connection was served: the genuine claim. Every earlier
		// connection was refused and none of them produced a spawn.
		CHECK_EQ(server.Runtime().Field().ServedClientCount(), static_cast<std::size_t>(1));
	}

	// =========================================================================
	// Multi-client isolation
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseC_TwoClientsDoNotShareWorldEntryState)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		ClientOutcome a;
		ClientOutcome b;

		// Two clients, each in its own thread, because a client that waits for a reply
		// cannot be served from the thread that would have to send it.
		std::thread threadA([&] { a = RunClientFlow(server, kUserA, kPassA, kCharA1.value, 0); });
		std::thread threadB([&] { b = RunClientFlow(server, kUserB, kPassB, kCharB1.value, 0); });
		threadA.join();
		threadB.join();

		CHECK_EQ(a.failure, std::string());
		CHECK_EQ(b.failure, std::string());
		CHECK(a.spawnSeen);
		CHECK(b.spawnSeen);

		// ---- each client got ITS OWN character ---------------------------
		CHECK_EQ(a.spawn.characterId, kCharA1.value);
		CHECK_EQ(a.spawn.characterName, std::string("Alpha"));
		CHECK_EQ(a.spawn.accountId, kAccountA.value);

		CHECK_EQ(b.spawn.characterId, kCharB1.value);
		CHECK_EQ(b.spawn.characterName, std::string("Beta"));
		CHECK_EQ(b.spawn.accountId, kAccountB.value);

		// ---- and its own entity id ---------------------------------------
		//
		// Two live characters must not share a gaeaId: every later gameplay message
		// refers to a character by this id, so a shared one would make them the same
		// entity as far as the world is concerned.
		CHECK_NE(a.gaeaId, b.gaeaId);
		CHECK_EQ(a.spawn.gaeaId, a.gaeaId);
		CHECK_EQ(b.spawn.gaeaId, b.gaeaId);

		// ---- four distinct sockets ---------------------------------------
		CHECK_NE(a.agentSourcePort, a.fieldSourcePort);
		CHECK_NE(b.agentSourcePort, b.fieldSourcePort);
		CHECK_NE(a.agentSourcePort, b.agentSourcePort);
		CHECK_NE(a.fieldSourcePort, b.fieldSourcePort);

		// ---- neither client can select the other's character ---------------
		//
		// The ownership rule, over real sockets. A refuses B's character, so the Agent
		// role closes without a 2358 and the client has nothing to dial: `sawRedirect`
		// false IS the observable refusal.
		const ClientOutcome stolen =
		    RunClientFlow(server, kUserA, kPassA, kCharB1.value, 0, false);
		CHECK(!stolen.sawRedirect);
		CHECK(!stolen.fieldOk);
		CHECK(!stolen.spawnSeen);

		// And A's own character is unaffected by the refused attempt.
		const ClientOutcome again = RunClientFlow(server, kUserA, kPassA, kCharA1.value, 0);
		CHECK_EQ(again.failure, std::string());
		CHECK(again.spawnSeen);
		CHECK_EQ(again.spawn.characterName, std::string("Alpha"));

		server.Stop();
	}

	// =========================================================================
	// Lifecycle
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseC_StartUseStopAndRepeat)
	{
		// Two complete server lifetimes in one process.
		//
		// The second iteration is the point: a listener that cannot be rebound after a
		// clean stop is a server that can only be started once, and a leaked socket from
		// the first iteration would appear here as a port conflict.
		for (int iteration = 0; iteration < 2; ++iteration)
		{
			TestWorldServer server;
			CHECK(server.Start().IsOk());
			CHECK(server.Runtime().IsRunning());

			const ClientOutcome outcome =
			    RunClientFlow(server, kUserA, kPassA, kCharA1.value, 0);
			CHECK(outcome.failure.empty());
			CHECK(outcome.spawnSeen);

			server.Stop();
			CHECK(!server.Runtime().IsRunning());

			// The port is genuinely released: connecting to the Field endpoint must now
			// fail. Asserted rather than assumed, because a listener that reported itself
			// stopped while still accepting would pass every other check here.
			RawClient probe;
			CHECK(probe.Connect(ToEndpointAddress(server.ExpectedRedirect()), 500).IsError());
		}
	}

	MODERN_TEST(WorldEntry_PhaseC_ServersRefuseToServeWhenNotRunning)
	{
		TestWorldServer server;

		// Serving before Start, and after Stop, are InvalidState rather than a crash or
		// a silent no-op - a caller looping on ServeOneClient must be able to tell.
		//
		// The Field role has no one-shot entry point to call (it serves itself from
		// Start), so what is asserted instead is that it is not listening: a caller that
		// believed it had a Field role before Start would find a port already bound.
		CHECK(server.Runtime().ServeOneAgentClient(100).IsError());
		CHECK(!server.Runtime().Field().IsRunning());
		CHECK(server.Runtime().AuthorizedFieldSessionCount() == 0);

		CHECK(server.Start().IsOk());
		CHECK(server.Runtime().Field().IsRunning());

		// A second Start is refused rather than silently rebinding: a caller that
		// believed it had two servers would find one port serving both.
		CHECK(server.Runtime().Start().IsError());

		server.Stop();
		CHECK(server.Runtime().ServeOneAgentClient(100).IsError());
		CHECK(!server.Runtime().Field().IsRunning());
		CHECK(server.Runtime().AuthorizedFieldSessionCount() == 0);
	}

	// =========================================================================
	// WORLD-ENTRY-002a: movement state over real sockets
	// =========================================================================
	//
	// The cases below are the ones that need a socket. MovementStateServiceTests
	// proves which BITS move; nothing there can prove that a 3032 survives framing,
	// that the answer arrives on the connection the move was sent from, or that a
	// second client learns about it - all three of which are wire behaviour, and all
	// three of which are where a change like this actually breaks.
	//
	// The property under test throughout is that the Field connection SURVIVES the
	// move. That is the whole reason 002a changed the Field role from one
	// conversation per connection to a long-lived one: a 3032 arrives minutes after
	// the 2359, on a socket that must still be there.

	// A client that has finished world entry and is still holding its Field
	// connection open.
	//
	// A struct rather than a function returning a connection, because FieldConnection
	// owns a socket and is neither copyable nor movable - a helper would have to
	// return a reference to something the caller owns, which is more indirection than
	// the three lines of connect/identify/pump it would save.
	struct SpawnedClient
	{
		// Declared before `protocol`, which takes it by reference.
		MinLzo1xCodec    codec;
		WorldEntryClient protocol{codec};
		FieldConnection  connection{protocol};
		WorldSpawnState  spawn;

		// Runs the whole entry flow and stops with the Field connection OPEN.
		Status Spawn(TestWorldServer& server, const std::string& userId,
		             const std::string& password, WireU32 selectId, int timeout)
		{
			// A real 2358 from a real Agent conversation. A test that built its own
			// redirect would be testing a Field client that no server ever authorized.
			const AgentOnlyResult agent =
			    RunAgentConversation(server, userId, password, selectId);
			if (!agent.ok)
			{
				return Status(ErrorCode::InvalidState);
			}

			if (const Status status = connection.Connect(agent.redirect, timeout);
			    status.IsError())
			{
				return status;
			}

			if (const Status status = connection.SendIdentity(IdentityFrom(agent.redirect));
			    status.IsError())
			{
				return status;
			}

			if (const Status status = connection.PumpUntilSpawn(timeout); status.IsError())
			{
				return status;
			}

			spawn = protocol.Spawn();
			return Ok();
		}
	};

	// Short, because it is used to prove the server says NOTHING. A generous budget
	// would turn a real hang into a slow test rather than a failure.
	constexpr int kSilenceBudget = 500;

	// Polls `predicate` until it holds, or the budget runs out.
	//
	// Needed because "the client received the 2335" and "the server recorded the
	// session" are two different events. The server sends the spawn, the bytes cross
	// loopback, the client reads them and returns from its pump - and only then does
	// the server's own worker thread run its next statement and mark the peer spawned.
	// So asserting a SERVER-side counter immediately after a CLIENT-side confirmation
	// is a race by construction.
	//
	// The alternative, a fixed sleep, only relocates the flake: it makes the test
	// slower on a fast machine and still fails on a slow one. This waits exactly as
	// long as the property takes and no longer.
	bool WaitFor(int budgetMilliseconds, const std::function<bool()>& predicate)
	{
		const auto deadline = std::chrono::steady_clock::now() +
		                     std::chrono::milliseconds(budgetMilliseconds);

		while (std::chrono::steady_clock::now() < deadline)
		{
			if (predicate())
			{
				return true;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		// Checked once more so a predicate that became true during the final sleep is
		// not reported as a failure.
		return predicate();
	}

	MODERN_TEST(MovementState_AClientMovesAndGetsTheAuthoritativeAnswerBack)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		SpawnedClient client;
		CHECK(client.Spawn(server, kUserA, kPassA, kCharA1.value, kDeadline).IsOk());
		CHECK(client.spawn.received);
		CHECK_EQ(client.spawn.characterId, kCharA1.value);

		const WireU32 gaeaId = client.spawn.gaeaId;
		CHECK(gaeaId != 0);

		// The connection is STILL OPEN after the spawn. Asserted before the move
		// because it is the premise of the whole milestone: before 002a the Field role
		// closed the socket here, and every later case in this file depends on it not
		// doing so.
		CHECK(client.connection.IsConnected());

		CHECK(client.connection.SendMoveState(MovementState::kActRun).IsOk());
		CHECK(client.connection.PumpUntilMoveCount(1, kDeadline).IsOk());

		CHECK_EQ(client.connection.MoveStateCount(), static_cast<std::size_t>(1));
		CHECK(client.connection.MoveState().received);
		CHECK_EQ(client.connection.MoveState().frame.size(),
		         static_cast<std::size_t>(16));
		CHECK_EQ(client.connection.MoveState().gaeaId, gaeaId);

		// The authoritative word, which for this request is exactly the two client-owned
		// bits and nothing else.
		CHECK_EQ(client.connection.MoveState().actState,
		         static_cast<WireU32>(MovementState::kActRun));

		CHECK(client.connection.IsConnected());

		server.Stop();
		CHECK_EQ(server.Runtime().Field().MoveStateSentCount(), static_cast<std::size_t>(1));
		CHECK_EQ(server.Runtime().Field().AuthorizedSessionCount(),
		         static_cast<std::size_t>(0));
	}

	// The observable half of legacy's `if (dwOldActState != m_dwActState)`
	// (GLCharMsg.cpp:203): an unchanged word sends NOTHING at all.
	//
	// Asserted as an ABSENCE. A test that only checked "the pump returned" would pass
	// against a server that sent a second identical 3033.
	MODERN_TEST(MovementState_AnUnchangedMoveProducesNoBroadcastAtAll)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		SpawnedClient client;
		CHECK(client.Spawn(server, kUserA, kPassA, kCharA1.value, kDeadline).IsOk());

		CHECK(client.connection.SendMoveState(MovementState::kActRun).IsOk());
		CHECK(client.connection.PumpUntilMoveCount(1, kDeadline).IsOk());
		CHECK_EQ(client.connection.MoveStateCount(), static_cast<std::size_t>(1));

		// Exactly the state it is already in.
		CHECK(client.connection.SendMoveState(MovementState::kActRun).IsOk());

		// Asking for a SECOND 3033 must time out: there will not be one.
		CHECK(client.connection.PumpUntilMoveCount(2, kSilenceBudget).IsError());
		CHECK_EQ(client.connection.MoveStateCount(), static_cast<std::size_t>(1));

		// Still connected, and still able to move - a no-op must not cost the session.
		CHECK(client.connection.IsConnected());
		CHECK(client.connection.SendMoveState(MovementState::kActRun |
		                                     MovementState::kActPeaceMode)
		          .IsOk());
		CHECK(client.connection.PumpUntilMoveCount(2, kDeadline).IsOk());
		CHECK_EQ(client.connection.MoveState().actState,
		         static_cast<WireU32>(MovementState::kActRun |
		                               MovementState::kActPeaceMode));

		server.Stop();
		CHECK_EQ(server.Runtime().Field().MoveStateSentCount(), static_cast<std::size_t>(2));
		CHECK_EQ(server.Runtime().Field().MoveStateUnchangedCount(),
		         static_cast<std::size_t>(1));
	}

	// The broadcast is the point of 3033 being a separate message from 3032: a client
	// that did not move still has to learn that somebody else did.
	//
	// Two real clients, two real characters, two real Agent conversations and two Field
	// connections alive at once - which is only possible because the Field role now
	// serves connections concurrently. A one-conversation-at-a-time Field role would
	// have deadlocked here, and that is a real regression this case prevents.
	MODERN_TEST(MovementState_ASecondClientIsToldAboutTheFirstClientsMove)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		SpawnedClient mover;
		SpawnedClient watcher;
		CHECK(mover.Spawn(server, kUserA, kPassA, kCharA1.value, kDeadline).IsOk());
		CHECK(watcher.Spawn(server, kUserB, kPassB, kCharB1.value, kDeadline).IsOk());

		CHECK(mover.spawn.gaeaId != watcher.spawn.gaeaId);
		// Waited for, not asserted immediately: both clients hold open Field
		// connections, and the server's own count of that trails their spawns by a
		// thread scheduling. See WaitFor.
		CHECK(WaitFor(kDeadline, [&] {
			return server.Runtime().AuthorizedFieldSessionCount() == 2;
		}));

		CHECK(mover.connection.SendMoveState(MovementState::kActRun).IsOk());

		// BOTH are answered: the mover on its own connection, the watcher on its own.
		CHECK(mover.connection.PumpUntilMoveCount(1, kDeadline).IsOk());
		CHECK(watcher.connection.PumpUntilMoveCount(1, kDeadline).IsOk());

		CHECK_EQ(mover.connection.MoveState().gaeaId, mover.spawn.gaeaId);
		CHECK_EQ(mover.connection.MoveState().actState,
		         static_cast<WireU32>(MovementState::kActRun));

		// The watcher is told whose move it was. Asserting gaeaId specifically is what
		// separates a real broadcast from a server echoing the watcher's own state back.
		CHECK(watcher.connection.MoveState().received);
		CHECK_EQ(watcher.connection.MoveState().gaeaId, mover.spawn.gaeaId);
		CHECK_EQ(watcher.connection.MoveState().actState,
		         static_cast<WireU32>(MovementState::kActRun));
		CHECK_EQ(watcher.connection.MoveStateCount(), static_cast<std::size_t>(1));

		// The watcher never sent a 3032 and therefore never got a spawn-time state of its
		// own - exactly one broadcast, about somebody else.
		CHECK(watcher.spawn.received);

		server.Stop();
	}

	// The authority rules, over a socket, where a client could actually try them.
	//
	// Both accounts here are ordinary (accountLevel 0, below USER_GM3), so the two
	// visibility flags are outside this client's reach entirely.
	MODERN_TEST(MovementState_AClientCannotInfluenceServerOwnedBitsOverTheWire)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		// ---- asking to be DEAD, from a clean state -----------------------
		//
		// Nothing changes, so nothing is broadcast. Silence is the correct answer, and
		// a server that granted the request would answer with a 3033 carrying EM_ACT_DIE.
		{
			SpawnedClient client;
			CHECK(client.Spawn(server, kUserA, kPassA, kCharA1.value, kDeadline).IsOk());

			CHECK(client.connection
			          .SendMoveState(MovementState::kActDie | MovementState::kReqLogout |
			                         MovementState::kReqGateOut)
			          .IsOk());

			CHECK(client.connection.PumpUntilMoveCount(1, kSilenceBudget).IsError());
			CHECK_EQ(client.connection.MoveStateCount(), static_cast<std::size_t>(0));
			CHECK(client.connection.IsConnected());

			// And the session is unharmed: a refused bit is not a refused connection.
			CHECK(client.connection.SendMoveState(MovementState::kActRun).IsOk());
			CHECK(client.connection.PumpUntilMoveCount(1, kDeadline).IsOk());
			CHECK_EQ(client.connection.MoveState().actState,
			         static_cast<WireU32>(MovementState::kActRun));
		}

		// ---- asking for the GM visibility flags --------------------------
		{
			SpawnedClient client;
			CHECK(client.Spawn(server, kUserA, kPassA, kCharA2.value, kDeadline).IsOk());

			CHECK(client.connection
			          .SendMoveState(MovementState::kReqVisibleNone |
			                         MovementState::kReqVisibleOff)
			          .IsOk());

			CHECK(client.connection.PumpUntilMoveCount(1, kSilenceBudget).IsError());
			CHECK_EQ(client.connection.MoveStateCount(), static_cast<std::size_t>(0));
		}

		server.Stop();
		CHECK_EQ(server.Runtime().Field().MoveStateSentCount(), static_cast<std::size_t>(1));
		CHECK_EQ(server.Runtime().Field().RefusedClientCount(), static_cast<std::size_t>(0));
	}

	// A 3032 is refused BEFORE the client has entered the world.
	//
	// The connection is dropped rather than left open, because a connection that has
	// not presented a 2359 has nothing to attribute a movement state to - and the one
	// thing this server must never do is guess whose character is moving.
	MODERN_TEST(MovementState_AMoveBeforeEntryIsRefusedAndTheConnectionDropped)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		const AgentOnlyResult agent =
		    RunAgentConversation(server, kUserA, kPassA, kCharA1.value);
		CHECK(agent.ok);

		MinLzo1xCodec    codec;
		WorldEntryClient protocol(codec);
		FieldConnection  connection(protocol);

		CHECK(connection.Connect(agent.redirect, kDeadline).IsOk());
		CHECK(connection.SendMoveState(MovementState::kActRun).IsOk());

		// No 3033, ever.
		CHECK(connection.PumpUntilMoveCount(1, kSilenceBudget).IsError());
		CHECK(!protocol.MoveState().received);

		server.Stop();
		CHECK_EQ(server.Runtime().Field().RefusedClientCount(), static_cast<std::size_t>(1));
		CHECK_EQ(server.Runtime().Field().MoveStateSentCount(), static_cast<std::size_t>(0));
	}

	// A wrong-sized 3032 is refused at the framing boundary.
	//
	// Sent RAW, so the test controls dwSize: a client built through
	// WorldEntryClient::SendMoveState cannot produce one, which is the point - this
	// case exists for a peer that is not that client.
	MODERN_TEST(MovementState_AMalformed3032IsRefused)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		const AgentOnlyResult agent =
		    RunAgentConversation(server, kUserA, kPassA, kCharA1.value);
		CHECK(agent.ok);

		MinLzo1xCodec    codec;
		WorldEntryClient protocol(codec);
		FieldConnection  connection(protocol);

		CHECK(connection.Connect(agent.redirect, kDeadline).IsOk());
		CHECK(connection.SendIdentity(IdentityFrom(agent.redirect)).IsOk());
		CHECK(connection.PumpUntilSpawn(kDeadline).IsOk());

		// Correct id, wrong length: 8 bytes where 12 are required.
		std::vector<WireU8> malformed;
		Network::Codec::WriteU32(malformed, 8);
		Network::Codec::WriteU32(malformed, MovementState::kMoveStateId);
		CHECK_EQ(malformed.size(), static_cast<std::size_t>(8));

		CHECK(connection.SendRaw(malformed).IsOk());
		CHECK(connection.PumpUntilMoveCount(1, kSilenceBudget).IsError());
		CHECK(!protocol.MoveState().received);

		server.Stop();
		CHECK_EQ(server.Runtime().Field().RefusedClientCount(), static_cast<std::size_t>(1));
		CHECK_EQ(server.Runtime().Field().MoveStateSentCount(), static_cast<std::size_t>(0));
	}

	// The connection outlives several moves and an idle gap.
	//
	// The idle gap is the point: WORLD-ENTRY-002a raised the Field role's read budget
	// from "the whole conversation" to "per message", precisely so a client could sit
	// still between moves. A budget that still covered the whole connection would kill
	// this client mid-session.
	MODERN_TEST(MovementState_TheConnectionSurvivesAnIdleGapBetweenMoves)
	{
		TestWorldServer server;
		CHECK(server.Start().IsOk());

		SpawnedClient client;
		CHECK(client.Spawn(server, kUserA, kPassA, kCharA1.value, kDeadline).IsOk());

		CHECK(client.connection.SendMoveState(MovementState::kActRun).IsOk());
		CHECK(client.connection.PumpUntilMoveCount(1, kDeadline).IsOk());

		// Read nothing at all for a while. No pump, no traffic - just silence, which is
		// what an AFK player looks like to the server.
		std::this_thread::sleep_for(std::chrono::milliseconds(1200));

		CHECK(client.connection.IsConnected());
		CHECK(WaitFor(kDeadline, [&] {
			return server.Runtime().AuthorizedFieldSessionCount() == 1;
		}));

		// Still authoritative, still responsive.
		CHECK(client.connection.SendMoveState(MovementState::kActPeaceMode).IsOk());
		CHECK(client.connection.PumpUntilMoveCount(2, kDeadline).IsOk());

		// EM_ACT_RUN was set by the first move and the second request did not mention
		// it, so it is CLEARED - the whole word, applied bit by bit.
		CHECK_EQ(client.connection.MoveState().actState,
		         static_cast<WireU32>(MovementState::kActPeaceMode));

		server.Stop();
		CHECK_EQ(server.Runtime().Field().MoveStateSentCount(), static_cast<std::size_t>(2));
	}
}

int main()
{
	std::setvbuf(stdout, nullptr, _IONBF, 0);

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n",
		            static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n", failedCases,
	            static_cast<int>(ModernTests::Registry().size()),
	            ModernTests::FailureCount());
	return 1;
}
