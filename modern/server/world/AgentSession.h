#pragma once

// WORLD-ENTRY-001 Phase B: the Agent role's per-connection state.
//
// ---------------------------------------------------------------------------
// WHAT THIS IS FOR
// ---------------------------------------------------------------------------
//
// A client that asks for something it is not entitled to ask for yet must be
// refused at the boundary, before the request reaches the repository or the wire
// codec. That is the whole reason these states exist.
//
// This is deliberately NOT `Modern::Network::ServerSession`. That class is the
// single-connection state machine from VERTICAL-027 and it already models
// Connected -> Authenticating -> Authenticated -> CharacterSelected -> InWorld.
// It is left untouched and unused here, because:
//
//   - its states are the ones a generic connection needs, and world entry needs
//     two the generic flow has no place for: the character LIST has been asked for
//     and the list has been SENT (a client may select only after the list, and the
//     two are separately observable on the wire), and world entry is AUTHORIZED,
//     which is a distinct step from having selected a character;
//   - it stores a character id with no ownership check attached, which is fine for
//     a transport and wrong for the role that decides who enters the world;
//   - folding world entry into `InWorld` would lose the authorization step, and the
//     authorization is the security-relevant one.
//
// Legacy does not model this as a state machine either - it is implicit in which
// fields `CClientManager` has populated for a slot. The states below are what
// becomes explicit.
//
// ---------------------------------------------------------------------------
// NO STATES THAT THE PROTOCOL DOES NOT HAVE
// ---------------------------------------------------------------------------
//
// Six states, one per step the trace actually passes through. There is no
// "Disconnecting", no "Reconnecting" and no "Idle": nothing in the WORLD-001 path
// produces them, and a state a caller can never reach is a state nobody tests.
//
// The transitions are NOT made legal because they are convenient. `SelectCharacter`
// requires `CharacterListSent` and not `Authenticated`, because a client that
// selects before the list has been sent is either confused or probing, and both are
// refused by the same rule.

#include "types/Result.h"
#include "world/WorldCharacter.h"

#include <cstdint>
#include <string>

namespace Modern::Server::World
{
	enum class AgentState : std::uint8_t
	{
		// TCP accepted, nothing sent. The Agent holds no account yet, so it cannot
		// answer anything character-related.
		Connected = 0,

		// LOGIN_FB (2050) accepted and the account is known. The character list may
		// now be requested.
		Authenticated,

		// 2247 received; the list is being assembled. The client has not been told
		// anything about which characters exist yet.
		CharacterListRequested,

		// 2248 sent. This is the state a selection is legal from, and it is the state
		// that proves the client has been told what it is allowed to choose between.
		CharacterListSent,

		// 2353 received and the named character was proven to be this account's.
		CharacterSelected,

		// The Field role has allocated a gaeaId and the 2358 redirect has been built.
		// The client may now open the second connection.
		WorldEntryAuthorized,

		// Terminal.
		Closed,
	};

	const char* ToString(AgentState state) noexcept;

	// The Agent's half of one client conversation.
	//
	// Holds no socket and opens none. Phase C's listener will own the socket and
	// will call these methods; that is the whole reason this type can be tested
	// without a network.
	class AgentSession
	{
	public:
		explicit AgentSession(Network::WireU64 sessionId = 0) noexcept
			: m_sessionId(sessionId)
		{
		}

		Network::WireU64 SessionId() const noexcept { return m_sessionId; }
		AgentState         State() const noexcept { return m_state; }
		bool               IsClosed() const noexcept { return m_state == AgentState::Closed; }

		// ---- lifecycle ------------------------------------------------------

		// Connected -> Authenticated.
		//
		// `rejected` returns the session to Connected rather than to a
		// "Rejected" state: the legacy Agent's response to a failed
		// `IsAccountPass` is `CloseClient(dwClient)`
		// (s_CAgentServerMsg.cpp:625-630), and a session with no account cannot
		// answer a character request, so leaving it Connected models "still
		// nothing has been established" without inventing a state.
		//
		// The account id and user id are the SERVER's, read from the login result -
		// never anything the client supplied about itself beyond the credential.
		Status CompleteAuthentication(WorldAccountId accountId, std::string userId,
		                              bool rejected);

		// Authenticated -> CharacterListRequested.
		Status BeginCharacterList() noexcept;

		// CharacterListRequested -> CharacterListSent.
		Status CompleteCharacterList() noexcept;

		// CharacterListSent -> CharacterSelected.
		//
		// Takes the AUTHORITATIVE character, not a client id and not a client's idea
		// of the character's state. The service layer has already resolved and
		// ownership-checked it; storing the resolved record is what stops a later
		// step from re-reading the client's selection and trusting it.
		//
		// Legal only from CharacterListSent, which is what refuses both
		// "Connected -> select" and "Authenticated -> select" and a duplicate
		// re-selection of the same character.
		Status SelectCharacter(const WorldCharacter& character);

		// CharacterSelected -> WorldEntryAuthorized.
		//
		// Legal only from CharacterSelected. This is the transition that makes
		// "AUTHENTICATED -> world entry without a selection" impossible, and it is
		// the one the Field role's 2359 validation ultimately depends on.
		Status AuthorizeWorldEntry() noexcept;

		void Close() noexcept { m_state = AgentState::Closed; }

		// ---- identity -------------------------------------------------------

		// Named `Account()` rather than `AccountId()`, and deliberately so: a member
		// function called `AccountId` would shadow the WorldAccountId TYPE for the
		// rest of the class, and the `WorldAccountId m_accountId;` member below
		// would stop compiling. FieldSession uses the same name for the same reason.
		WorldAccountId            Account() const noexcept { return m_accountId; }
		const std::string&    UserId() const noexcept { return m_userId; }
		bool                  IsAuthenticated() const noexcept;

		// The selected character, or nullptr when none has been selected. A pointer
		// rather than an optional copy so that "no selection" cannot be confused with
		// "a selection whose default-constructed value happens to be valid".
		const WorldCharacter* SelectedCharacter() const noexcept;

		// ---- authorisation --------------------------------------------------

		// Whether a request of this kind is meaningful yet. The Agent's dispatcher
		// asks these before routing, so an out-of-order request never reaches a
		// service.
		bool MayRequestCharacterList() const noexcept
		{
			return m_state == AgentState::Authenticated;
		}
		bool MayRequestCharacterSelect() const noexcept
		{
			return m_state == AgentState::CharacterListSent;
		}
		bool MayRequestWorldEntry() const noexcept
		{
			return m_state == AgentState::CharacterSelected;
		}

		// ---- liveness -------------------------------------------------------

		// Time is passed in, never read. The session owns no clock, which is what
		// lets a test drive a timeout deterministically.
		void AdvanceClock(Network::WireU64 milliseconds) noexcept
		{
			m_clockMs += milliseconds;
		}
		Network::WireU64 NowMs() const noexcept { return m_clockMs; }
		void            NoteHeartbeat() noexcept { m_lastHeartbeatMs = m_clockMs; }

		// True once NET_TIME_OUT has elapsed with no heartbeat
		// (kTimeoutMilliseconds, s_NetGlobal.h:117).
		bool IsTimedOut() const noexcept
		{
			return m_clockMs - m_lastHeartbeatMs >= Network::Protocol::kTimeoutMilliseconds;
		}

	private:
		Network::WireU64 m_sessionId = 0;
		Network::WireU64 m_clockMs = 0;
		Network::WireU64 m_lastHeartbeatMs = 0;

		WorldAccountId     m_accountId;
		std::string   m_userId;
		WorldCharacter m_selected;
		bool          m_hasSelection = false;

		AgentState m_state = AgentState::Connected;
	};
}