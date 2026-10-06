#pragma once

// WORLD-ENTRY-001 Phase C: the two client sockets.
//
// ---------------------------------------------------------------------------
// TWO CLASSES, TWO OWNED TRANSPORTS, NO SHARED STATE
// ---------------------------------------------------------------------------
//
// AgentConnection owns TCP #1. FieldConnection owns TCP #2. Neither borrows the
// other's transport, neither can read the other's bytes, and neither closes a
// socket the other opened.
//
// That is not a style preference. It is the property the phase exists to prove:
// legacy's client opens a genuinely new connection for world entry (the
// NET_STATE_FIELD branch of s_NetClient.cpp), and a client that re-pointed one
// TcpTransport - or moved the first connection's descriptor into a "field session" -
// would pass every protocol assertion while testing nothing about the topology.
//
// LocalEndpoint() is exposed on both so a test can read the OS-assigned source
// ports and assert they differ. Two connections from one client to one host get
// two different source ports; that is the cheapest available evidence that two
// sockets exist, and it is evidence rather than assertion about intent.
//
// ---------------------------------------------------------------------------
// DEADLINES ARE PASSED IN, AND EVERY LOOP IS BOUNDED
// ---------------------------------------------------------------------------
//
// Neither class reads a clock of its own. A client that connects and then says
// nothing must not hang a test suite, so every read is bounded, and the budget is
// passed by the caller so a test can tighten it.
//
// `maxChunkBytes` exists for the same reason it does in LOGIN-002's
// LoginServerSession: a test has to be able to ask for a 1-byte read to prove that
// ConnectionFramer - not luck - is what reassembles a message. A real caller passes
// 0 and the transport decides.

#include "GameServerListProtocol.h"
#include "MessageReader.h"
#include "NetworkConnection.h"
#include "NetworkTransport.h"
#include "NetworkTypes.h"
#include "TcpTransport.h"
#include "types/Result.h"
#include "world/WorldEntryClient.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Client
{
	// The first connection: the Agent conversation.
	//
	// Owns the socket and drives WorldEntryClient through it. Mirrors
	// LoginServerSession's split exactly - the session decides WHEN to read, the
	// protocol object never sees a socket - so the two roles stay independently
	// testable.
	class AgentConnection
	{
	public:
		explicit AgentConnection(WorldEntryClient& protocol) noexcept
			: m_protocol(protocol)
		{
		}

		~AgentConnection();

		AgentConnection(const AgentConnection&)            = delete;
		AgentConnection& operator=(const AgentConnection&) = delete;

		// Connects to the Agent at `endpoint`. `ip` must be a numeric dotted-quad:
		// legacy calls ::inet_addr directly (s_NetClient.cpp:474) with its
		// gethostbyname branch commented out, so a name would silently become
		// INADDR_NONE.
		Status Connect(const Network::EndpointAddress& endpoint, int timeoutMilliseconds = 0);

		// Sends `bytes` - already a whole NET_MSG_GENERIC - RAW.
		//
		// Raw is correct and not an omission: client -> anything is never batched and
		// never enveloped (CNetClient::SendBuffer2, s_NetClient.cpp:815-831). Adding
		// an envelope here would produce something the server's framer could not read.
		Status Send(const std::vector<Network::WireU8>& bytes);

		// Reads until the protocol has moved past `untilPhase`, the budget runs out,
		// the peer closes, or something faults.
		//
		// Returns the outcome rather than only a Status, because "the server said
		// nothing" and "the server sent something wrong" are different things to a
		// caller and Status has no member for the first.
		//
		// Passing `untilPhase` as a target rather than looping until "enough" is what
		// makes a hang impossible: the loop has a termination condition the CALLER
		// states, not one it infers.
		Status PumpUntil(WorldEntryPhase untilPhase, int timeoutMilliseconds,
		                 std::size_t maxChunkBytes = 0);

		// How many messages the last Pump consumed, across both connections.
		std::size_t MessagesHandled() const noexcept { return m_handled; }

		void      Disconnect() noexcept;
		bool      IsConnected() const noexcept { return m_transport.IsConnected(); }
		Network::Endpoint RemoteEndpoint() const noexcept { return m_transport.RemoteEndpoint(); }

		// The OS-assigned SOURCE port of this connection.
		//
		// The proof that connection #1 is a real socket distinct from connection #2:
		// two connections to one host get two source ports.
		Network::Endpoint LocalEndpoint() const noexcept { return m_transport.LocalEndpoint(); }

		Network::TransportFault Fault() const noexcept { return m_transport.Fault(); }

	private:
		WorldEntryClient& m_protocol;
		Network::TcpTransport m_transport;
		std::size_t m_handled = 0;
	};

	// The second connection: the Field conversation.
	//
	// Constructed from a 2358 and nothing else. There is no other constructor and no
	// default endpoint, so a client physically cannot dial a Field it was not
	// redirected to - which is what makes "the client does not ignore 2358" a
	// structural property rather than a promise.
	class FieldConnection
	{
	public:
		explicit FieldConnection(WorldEntryClient& protocol) noexcept
			: m_protocol(protocol)
		{
		}

		~FieldConnection();

		FieldConnection(const FieldConnection&)            = delete;
		FieldConnection& operator=(const FieldConnection&) = delete;

		// Opens TCP #2 to the endpoint the 2358 carried.
		//
		// `redirect` is the SERVER's packet, used verbatim. `connectTimeout` bounds the
		// connect only.
		Status Connect(const Network::FieldRedirect& redirect, int connectTimeout = 0);

		// Sends the 2359 for `identity`, built by the shared codec.
		//
		// Separate from Connect so a caller that cannot send has a different problem
		// from one that sent and is waiting.
		Status SendIdentity(const Network::FieldIdentity& identity);

		// Reads until a 2333 has arrived, the budget runs out, the peer closes, or
		// something faults.
		Status PumpUntilSpawn(int timeoutMilliseconds, std::size_t maxChunkBytes = 0);

		// Sends a 3032 for the client's own movement state.
		//
		// `actState` is what the player DID, not what the player is allowed to do. See
		// WorldEntryClient::BuildMoveState: the bits go out verbatim and the server
		// decides. Raw, like every other client->server message here.
		Status SendMoveState(Network::WireU32 actState);

		// Reads until `wantedCount` 3033s have arrived in total, the budget runs out,
		// the peer closes, or something faults.
		//
		// Takes a COUNT rather than "until the next one", and that is forced by the
		// protocol: a client watching another player move cannot know in advance
		// whether the server will answer at all. Legacy sends NOTHING when the
		// authoritative state did not change (GLCharMsg.cpp:203), so "wait for the next
		// 3033" would hang forever on a no-op move. Passing the count the test already
		// knows lets the SAME call express both cases: the expected count for a change,
		// and `thisCount + 0` for a move that must stay silent - which then correctly
		// times out instead of inventing an answer.
		Status PumpUntilMoveCount(std::size_t wantedCount, int timeoutMilliseconds,
		                          std::size_t maxChunkBytes = 0);

	// Sends a 3034 for the client's own GOTO.
		//
		// `claimedCurrent` is where the CLIENT believes it is and `requestedTarget` is
		// where it wants to go. Both are sent verbatim - see
		// WorldEntryClient::BuildGoto: the server owns the position, and vCurPos exists
		// so it can DETECT disagreement, not so it can be argued with. Raw, like every
		// other client->server message here.
		Status SendGoto(WireU32 requestedActState, Vector3 claimedCurrent,
		                Vector3 requestedTarget);

		// Reads until `wantedCount` 3035s have arrived in total, the budget runs out,
		// the peer closes, or something faults.
		//
		// A COUNT, and for the same reason `PumpUntilMoveCount` takes one: legacy sends
		// NOTHING when a GOTO is refused - an unreachable destination, a dead character
		// and a desynchronised client are all silent - so a client cannot wait for "the
		// next one" or it would block forever on a refused move. Passing the count the
		// caller already knows lets the same call express both cases.
		Status PumpUntilGotoCount(std::size_t wantedCount, int timeoutMilliseconds,
		                          std::size_t maxChunkBytes = 0);

		const WorldSpawnState& Spawn() const noexcept { return m_protocol.Spawn(); }

		// The most recent 3033, and how many have arrived.
		const WorldMoveStateState& MoveState() const noexcept { return m_protocol.MoveState(); }
		std::size_t                MoveStateCount() const noexcept
		{
			return m_protocol.MoveStateCount();
		}
		// The most recent 3035, and how many have arrived.
		const WorldGotoState& Goto() const noexcept { return m_protocol.Goto(); }
		std::size_t GotoCount() const noexcept { return m_protocol.GotoCount(); }

		std::size_t MessagesHandled() const noexcept { return m_handled; }

		void      Disconnect() noexcept;
		bool      IsConnected() const noexcept { return m_transport.IsConnected(); }
		Network::Endpoint RemoteEndpoint() const noexcept { return m_transport.RemoteEndpoint(); }

		// The OS-assigned source port. Compared against AgentConnection's in the
		// integration test to prove two distinct sockets.
		Network::Endpoint LocalEndpoint() const noexcept { return m_transport.LocalEndpoint(); }

		Network::TransportFault Fault() const noexcept { return m_transport.Fault(); }

		// Sends and receives RAW bytes, for the malformed-packet tests.
		//
		// A client that could only ever send well-formed messages could not prove the
		// server refuses the malformed ones - and teaching the production client a
		// "send garbage" mode would be worse than a raw socket in a test.
		Status SendRaw(const std::vector<Network::WireU8>& bytes);

	private:
		WorldEntryClient& m_protocol;
		Network::TcpTransport m_transport;
		std::size_t m_handled = 0;
	};
}
