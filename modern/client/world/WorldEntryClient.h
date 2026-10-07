#pragma once

// WORLD-ENTRY-001 Phase C: the client half of the world-entry flow.
//
// This is the protocol object. It owns NO socket and never sees one - the same
// split LoginServerClient / LoginServerSession established, and for the same
// reason: it is what lets the state machine be tested byte by byte against a
// loopback pair, with the boundaries chosen by the test.
//
// The socket halves are AgentConnection (TCP #1) and FieldConnection (TCP #2).
//
// ---------------------------------------------------------------------------
// TWO CONNECTIONS, AND THE SECOND ONE IS DERIVED FROM A PACKET
// ---------------------------------------------------------------------------
//
// This is the part Phase C exists to prove, so it is worth being exact about what
// the code does:
//
//   1. AgentConnection dials the AGENT endpoint the caller configured.
//   2. WorldEntryClient runs login -> list -> detail -> select over it.
//   3. The server answers 2358. FieldRedirect::FieldEndpoint() returns the address
//      and port the SERVER put in that packet.
//   4. FieldConnection dials THAT address. There is no second endpoint configured
//      anywhere in this client, and no default. If the client had a fallback port
//      it would be a port the test could pass while the redirect was wrong, which
//      is precisely the bug the 2358 requirement exists to prevent.
//
// So a test that changes the Field role's port changes where the client connects,
// with no other edit. That is the proof.
//
// ---------------------------------------------------------------------------
// THE 2358 IS INSIDE A NET_COMPRESS ENVELOPE, AND SO IS EVERYTHING ELSE HERE
// ---------------------------------------------------------------------------
//
// Agent -> client and Field -> client are compressed unconditionally
// (investigation §4.2); client -> anything is raw. So this client unwraps an
// envelope, feeds the inner stream to ConnectionFramer, and sends without an
// envelope of its own.
//
// NetCompress::DecodeServerToClientEnvelope and ConnectionFramer are used, not
// reimplemented. What is written here is the buffering AROUND them - an envelope
// can straddle any read boundary, so raw bytes have to be held until a whole
// envelope is present - and that is the same twenty lines LoginResponseClient
// already needs for the same reason. It is envelope PLUMBING, not a third
// implementation of either.
//
// ---------------------------------------------------------------------------
// NO TERMINATOR AFTER THE CHARACTER LIST
// ---------------------------------------------------------------------------
//
// Legacy completes the list by COUNTING: `m_nStartCharNum == m_nStartCharLoad`
// (DxLobyStage.h:150). This client counts, and `ExpectedDetailCount()` is the value
// it counts toward. A client that waited for an end-of-list message would wait
// forever - which is the difference from LOGIN-001's SND_GAME_SVR_END, and the
// reason the two client types share no phase enum.

#include "CharacterListProtocol.h"
#include "GotoProtocol.h"
#include "login/World001LoginClient.h"
#include "NetCompressCodec.h"
#include "math/Vector3.h"
#include "MessageReader.h"
#include "MovementStateProtocol.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"
#include "WorldEntryProtocol.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Client
{
	// The wire types this header names directly.
	//
	// Explicit using-DECLARATIONS rather than a blanket "using namespace
	// Modern::Network". The blanket form would pull every protocol constant in the
	// network layer into Modern::Client's scope, where an id declared in two places
	// would silently become an ambiguity - and an ambiguity in a protocol header is a
	// wire bug waiting to be misdiagnosed as a build problem.
using Network::WireU8;
	using Network::WireU16;
	using Modern::Vector3;
	using Network::WireU32;
using Network::WireI32;
	using Network::Lzo1xCodec;
	using Network::ConnectionFramer;
	// Where the client is in the Agent conversation.
	enum class WorldEntryPhase : std::uint8_t
	{
		Disconnected = 0,

		// LOGIN_2 sent, LOGIN_FB not yet seen.
		LoggingIn,

		// Accepted. The character list may be requested.
		Authenticated,

		// 2248 received; the id list is known.
		CharacterListReady,

		// 2353 sent.
		SelectingCharacter,

		// 2358 received. This client has NOT yet dialled the Field - that is
		// FieldConnection's job, and it must be driven with the endpoint this phase
		// received. A client that treated the redirect as "nothing to do" would be
		// exactly the failure the brief forbids.
		RedirectReceived,

		LoginRejected,
	};

	const char* ToString(WorldEntryPhase phase) noexcept;

	// The authoritative spawn, as received from the Field role.
	//
	// A separate type from WorldEntry::SpawnState so that what the CLIENT believes
	// cannot be confused with what the SERVER was going to send: this one is decoded
	// from bytes off a socket, and the tests assert on it field by field.
	struct WorldSpawnState
	{
		bool        received = false;
		std::string userId;          // szUserID, the account login name
		WireU32     clientId     = 0;   // dwClientID; 0 in this phase, see the header
		WireU32     gaeaId       = 0;
		WireU32     accountId    = 0;
		WireU32     characterId  = 0;
		std::string characterName;
		WireU32     characterClass = 0;
		WireU16     school         = 0;
		WireU16     level          = 0;
		WireU32     hp = 0;
		WireU32     mp = 0;
		WireU32     sp = 0;
		WireU32     mapId      = 0;
		float       positionX = 0.0f;
		float       positionY = 0.0f;
		float       positionZ = 0.0f;
		WireU32     startMapId = 0;
		WireU32     startGate  = 0;

		// The frame as received, so a test can assert its size and inspect its
		// reserved regions without the client having to re-derive them.
		std::vector<WireU8> frame;
	};

	// The authoritative movement state, as received from the Field role in a 3033.
	//
	// Separate from Network::MovementState::MoveStateBroadcast for the same reason
	// WorldSpawnState exists: this one is decoded from bytes off a socket, and the tests
	// assert on it field by field rather than trusting a re-derived value.
	struct WorldMoveStateState
	{
		bool    received = false;

		// Whose move this was, and the AUTHORITATIVE bits for it.
		//
		// gaeaId is the sender's, not the receiver's. That is the whole point of a
		// broadcast: a client watching another player move sees the mover's id here and
		// must not confuse it with its own.
		WireU32 gaeaId   = 0;
		WireU32 actState = 0;

		// The frame as received, so a test can assert its size and offsets.
		std::vector<WireU8> frame;
	};

	// WORLD-ENTRY-002f: an accepted GOTO, as received in a 3035.
	//
	// RAN has NO per-tick authoritative position broadcast - 002c section 10 measured
	// that `GLChar::FrameMove` transmits nothing while a character walks, and the only
	// position corrections are the event-driven 3064 and 3830 - so this arrives ONCE per
	// accepted GOTO and is the only position the server ever sends. A client therefore
	// runs its own predicted walk and uses this to know that the server agreed.
	//
	// `currentPosition` is the SERVER's position, which is how a client that has
	// drifted learns where the server thinks it is. `targetPosition` is the RAW target
	// the mover asked for, not the point the server's vertical probe resolved to, so a
	// client that re-probes against it may land somewhere slightly different - which is
	// exactly what RAN does.
	struct WorldGotoState
	{
		bool received = false;

		// Whose GOTO this was. The sender's, not the receiver's.
		WireU32 gaeaId = 0;

		// The AUTHORITATIVE movement word as of this GOTO.
		WireU32 actState = 0;

		// The server's authoritative position when it accepted.
		float currentPositionX = 0.0f;
		float currentPositionY = 0.0f;
		float currentPositionZ = 0.0f;

		// The requested destination, verbatim.
		float targetPositionX = 0.0f;
		float targetPositionY = 0.0f;
		float targetPositionZ = 0.0f;

		// Dead on the server's GOTO path - always 0.0f - and present because it is four
		// bytes of a fixed-size struct. See GotoProtocol.h.
		float delay = 0.0f;

		// The frame as received, so a test can assert its size and offsets.
		std::vector<WireU8> frame;
	};

	// WORLD-ENTRY-002h: the authoritative resource state (3046).
	//
	// Carries ALL pools (HP/MP/SP/CP) with both current and maximum, plus
	// identity. The client's presented pools are updated from this; the
	// server's derived maxima are the authority and do not change from
	// this packet - the client must NOT overwrite `DerivedStats` with the
	// wire's maxima.
	struct WorldUpdateStateState
	{
		bool received = false;

		WireU32 hpNow = 0, hpMax = 0;
		WireU32 mpNow = 0, mpMax = 0;
		WireU32 spNow = 0, spMax = 0;
		WireU32 cpNow = 0, cpMax = 0;

		std::string characterName;
		WireU32     gaeaId   = 0;
		WireU32     charId   = 0;
		bool        safeTime = false;

		std::vector<WireU8> frame;
	};

	// WORLD-ENTRY-002h: the HP broadcast (3053).
	//
	// Carries only HP (current+max), gaeaId and safeTime. This is the
	// staged approximation of legacy's view-scoped broadcast: all
	// authorized clients receive it, not just those in view/party/PvP.
	struct WorldUpdateStateBrdState
	{
		bool received = false;

		WireU32 gaeaId   = 0;
		WireU32 hpNow    = 0;
		WireU32 hpMax    = 0;
		bool    safeTime = false;

		std::vector<WireU8> frame;
	};
	// Drives the client side of the Agent conversation, over no socket at all.
	//
	// The state machine enforces the same ORDERING the server does, so a client bug
	// is reported as a client bug rather than as a server refusal.
	class WorldEntryClient
	{
	public:
		explicit WorldEntryClient(Lzo1xCodec& codec) noexcept
			: m_codec(codec)
		{
		}

		// ---- Agent conversation --------------------------------------------

		// Builds LOGIN_2 into `request`.
		//
		// The request bytes come from the existing WORLD-001 codec; this class does not
		// re-encode a login packet.
		Status BuildLogin(const LoginRequestData& data, std::vector<WireU8>& request);

		// Builds 2247 - a bare 8-byte header.
		Status BuildRequestCharacterList(std::vector<WireU8>& request);

		// Builds 2244 for one character id.
		Status BuildRequestCharacterDetail(WireU32 characterId, std::vector<WireU8>& request);

		// Builds 2353 for one character id.
		Status BuildSelectCharacter(WireU32 characterId, std::vector<WireU8>& request);

		// Feeds bytes received from the Agent. Unwraps any envelopes and applies every
		// whole message, returning how many were consumed.
		Status FeedAgent(const WireU8* data, std::size_t size, std::size_t& messagesHandled);
		Status FeedAgent(const std::vector<WireU8>& data, std::size_t& messagesHandled)
		{
			return FeedAgent(data.data(), data.size(), messagesHandled);
		}

		// ---- Field conversation ---------------------------------------------

		// Builds 2359 from an identity.
		static Status BuildFieldIdentity(const Network::FieldIdentity& identity,
		                                 std::vector<WireU8>& request);

		// Builds 3032 - the client's own movement state.
		//
		// `actState` is the client's REQUESTED bits, not the answer: legacy sends what
		// the player did and the server decides which of it is allowed
		// (GLCharMsg.cpp:182-219). So this never invents or filters a bit. A client that
		// pre-applied the authority rules would make the server's authority untestable,
		// and would hide exactly the disagreement these tests exist to catch.
		//
		// No gaeaId is sent, because legacy sends none: the server already knows whose
		// connection this is, which is what makes the 3032 authoritative.
		Status BuildMoveState(WireU32 actState, std::vector<WireU8>& request);
// Builds 3034 - the client's own GOTO request.
		//
		// `requestedActState` is what the player is pressing, not what the server
		// will allow, and `claimedCurrent` is where the CLIENT thinks it is. Both are
		// sent verbatim: the server owns the authoritative position, and `vCurPos`
		// exists so the server can DETECT disagreement, not so it can be talked into
		// agreeing. A client that pre-applied the authority rules would make that
		// check untestable.
		//
		// No gaeaId is sent, because legacy sends none (GLContrlPcMsg.h:636-654).
		// The server learns whose character it is from the Field connection, which is
		// what makes the request unforgeable.
		Status BuildGoto(WireU32 requestedActState, Vector3 claimedCurrent,
		                 Vector3 requestedTarget, std::vector<WireU8>& request);

		// Feeds bytes received from the Field.

		// Feeds bytes received from the Field.
		//
		// Separate from FeedAgent because these are DIFFERENT connections with different
		// framer state: mixing them would let a partial message from one be completed
		// by bytes from the other, which is precisely the class of bug two sockets in
		// one process invites.
		Status FeedField(const WireU8* data, std::size_t size, std::size_t& messagesHandled);
		Status FeedField(const std::vector<WireU8>& data, std::size_t& messagesHandled)
		{
			return FeedField(data.data(), data.size(), messagesHandled);
		}

		// ---- results --------------------------------------------------------

		WorldEntryPhase Phase() const noexcept { return m_phase; }

		// The 2248 payload: how many characters, and which ids.
		const Network::CharacterIdList& CharacterIds() const noexcept { return m_ids; }
		std::size_t ExpectedDetailCount() const noexcept { return m_expectedDetails; }

		// The 2332 payloads, one per id, in arrival order.
		const std::vector<Network::CharacterDetail>& CharacterDetails() const noexcept
		{
			return m_details;
		}

		// The 2358 payload. Only meaningful once Phase() is RedirectReceived.
		const Network::FieldRedirect& Redirect() const noexcept { return m_redirect; }
		bool                          HasRedirect() const noexcept { return m_hasRedirect; }

		// The 2333 payload, decoded.
		const WorldSpawnState& Spawn() const noexcept { return m_spawn; }

		// The most recent 3033, decoded.
		//
		// "Most recent" rather than "the" because a client watching two players move
		// receives a stream of these, and keeping only the last is what a real client
		// would do. MoveStateCount() is what a test asserts on when the SEQUENCE
		// matters - that two players moving produced two 3033s, not one.
		const WorldMoveStateState& MoveState() const noexcept { return m_moveState; }
		std::size_t                MoveStateCount() const noexcept { return m_moveStateCount; }

		// WORLD-ENTRY-002f: the most recent 3035, and how many have arrived.
		//
		// "Most recent" and a COUNT for the same reason as MoveState: two players
		// moving produce a stream, and only the count can distinguish "two accepted
		// GOTO broadcasts" from "one broadcast seen twice".
		const WorldGotoState& Goto() const noexcept { return m_goto; }
		std::size_t          GotoCount() const noexcept { return m_gotoCount; }

		// WORLD-ENTRY-002h: the most recent 3046 and its count.
		const WorldUpdateStateState& UpdateState() const noexcept { return m_updateState; }
		std::size_t                 UpdateStateCount() const noexcept { return m_updateStateCount; }

		// WORLD-ENTRY-002h: the most recent 3053 and its count.
		const WorldUpdateStateBrdState& UpdateStateBrd() const noexcept { return m_updateStateBrd; }
		std::size_t                    UpdateStateBrdCount() const noexcept { return m_updateStateBrdCount; }

		// True once the framer has latched an unrecoverable framing error. A
		// desynchronised stream cannot resynchronise, so the caller must drop the
		// connection rather than keep parsing.
		bool IsFailed() const noexcept { return m_agentFailed || m_fieldFailed; }

		void Reset() noexcept;

	private:
		// Holds raw bytes until a whole NET_COMPRESS envelope is present, unwraps it,
		// and feeds the inner message stream to `framer`.
		//
		// Returns the number of complete inner messages now buffered in `framer`.
		Status ConsumeEnvelopes(std::vector<WireU8>& pendingRaw,
		                        ConnectionFramer&  framer,
		                        std::size_t         maxInnerBytes);

		// Pops every complete message currently buffered in the Agent framer.
		Status DrainAgentMessages(std::size_t& messagesHandled);

		// Pops every complete message currently buffered in the Field framer.
		Status DrainFieldMessages(std::size_t& messagesHandled);

		Lzo1xCodec& m_codec;

		ConnectionFramer m_agentFramer;
		ConnectionFramer m_fieldFramer;

		// Raw transport bytes not yet classifiable. Held across Feed calls because an
		// envelope can straddle any read boundary.
		std::vector<WireU8> m_agentRaw;
		std::vector<WireU8> m_fieldRaw;

		WorldEntryPhase m_phase = WorldEntryPhase::Disconnected;

		Network::CharacterIdList              m_ids;
		std::size_t                          m_expectedDetails = 0;
		std::vector<Network::CharacterDetail> m_details;
		Network::FieldRedirect               m_redirect;
		bool                                 m_hasRedirect = false;
		WorldSpawnState                      m_spawn;
		WorldMoveStateState                  m_moveState;
		std::size_t                          m_moveStateCount = 0;
		// WORLD-ENTRY-002f: the most recent 3035 and its count. Same reasoning as the
		// 3033 pair above.
		WorldGotoState                       m_goto{};
		std::size_t                          m_gotoCount = 0;

		bool m_agentFailed = false;
		bool m_fieldFailed = false;

		// WORLD-ENTRY-002h: 3046 and 3053 state.
		WorldUpdateStateState     m_updateState{};
		std::size_t               m_updateStateCount = 0;
		WorldUpdateStateBrdState  m_updateStateBrd{};
		std::size_t               m_updateStateBrdCount = 0;
	};
}
