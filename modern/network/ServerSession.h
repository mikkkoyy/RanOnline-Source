#pragma once

// VERTICAL-027: server-side session abstraction.
//
// A session is one authenticated-or-not client conversation. It owns connection
// lifetime and the state machine that gates what the client is allowed to ask
// for. It owns NO gameplay rules - those stay in modern/core.
//
// Server authority is expressed here rather than assumed: the state machine
// exists so that a message which arrives out of order is refused at the
// boundary, before it can reach gameplay code.
//
// The states are the minimum the brief's target flow needs -
// LOGIN -> CHARACTER LIST -> CHARACTER SELECT -> ENTER WORLD - and each exists
// because a request is only meaningful once the previous step has happened.
//
// Legacy ownership, verified in source:
//   NET_MSG_LOGIN / *_NET_MSG_LOGIN    s_NetGlobal.h:766-790 (LOBBY range)
//   NET_MSG_LOBBY_CHAR_SEL             s_NetGlobal.h:845  (LOBBY + 390)
//   NET_MSG_LOBBY_CHAR_JOIN            s_NetGlobal.h:846  (LOBBY + 391)
//   NET_MSG_LOBBY_CHAR_JOIN_FB         s_NetGlobal.h:848  (LOBBY + 393)
//   NET_MSG_HEARTBEAT_CLIENT_REQ/ANS   s_NetGlobal.h:718-719 (160/161)
//   NET_TIME_OUT                       s_NetGlobal.h:117  (180000 ms)

#include "NetworkTypes.h"

#include "types/Result.h"

#include <cstdint>
#include <string>

namespace Modern::Network
{
	enum class SessionState : uint8_t
	{
		// TCP accepted, nothing sent. Legacy: s_CNetUser after accept.
		Connected = 0,

		// Credentials received, verdict not yet returned.
		Authenticating,

		// Credentials accepted. Character list may be requested.
		Authenticated,

		// A character was named. Character join may be requested.
		CharacterSelected,

		// Character is in the world. Only world-authoritative requests are valid.
		InWorld,

		// Terminal.
		Closed,
	};

	const char* ToString(SessionState state) noexcept;

	// The stages the brief targets, expressed as a minimum set of operations the
	// session will accept. Each returns NotAllowed from the wrong state, which
	// is the entire point of having them.
	class ServerSession
	{
	public:
		explicit ServerSession(WireU64 sessionId = 0) noexcept
			: m_sessionId(sessionId)
		{
		}

		WireU64 SessionId() const noexcept { return m_sessionId; }
		SessionState State() const noexcept { return m_state; }

		// ---- lifecycle -----------------------------------------------------

		// Moves Connected -> Authenticating. Fails from any other state.
		Status BeginAuthentication() noexcept
		{
			if (m_state != SessionState::Connected)
			{
				return Status(ErrorCode::NotAllowed);
			}
			m_state = SessionState::Authenticating;
			return Ok();
		}

		// Moves Authenticating -> Authenticated. Carries the account/session id
		// the client will use for later requests.
		//
		// `rejected` keeps the failure path explicit rather than relying on a
		// missing transition: a caller that never calls either method leaves the
		// session in Authenticating, which is already refused for everything else.
		Status CompleteAuthentication(WireU64 accountId, bool rejected) noexcept
		{
			if (m_state != SessionState::Authenticating)
			{
				return Status(ErrorCode::NotAllowed);
			}
			if (rejected)
			{
				m_state = SessionState::Connected;
				return Status(ErrorCode::NotAllowed);
			}
			m_accountId = accountId;
			m_state = SessionState::Authenticated;
			return Ok();
		}

		// Authenticated -> CharacterSelected.
		Status SelectCharacter(WireU32 characterId) noexcept
		{
			if (m_state != SessionState::Authenticated)
			{
				return Status(ErrorCode::NotAllowed);
			}
			m_characterId = characterId;
			m_state = SessionState::CharacterSelected;
			return Ok();
		}

		// CharacterSelected -> InWorld.
		//
		// From any other state this closes the session rather than refusing:
		// legacy treats a join without a selection as a protocol fault, and
		// keeping a half-entered world is worse than dropping the connection.
		Status EnterWorld() noexcept
		{
			if (m_state == SessionState::InWorld)
			{
				return Ok();
			}
			if (m_state != SessionState::CharacterSelected)
			{
				Close();
				return Status(ErrorCode::InvalidState);
			}
			m_state = SessionState::InWorld;
			return Ok();
		}

		void Close() noexcept { m_state = SessionState::Closed; }
		bool IsClosed() const noexcept { return m_state == SessionState::Closed; }

		// ---- identity ------------------------------------------------------

		WireU64 AccountId() const noexcept { return m_accountId; }
		WireU32 CharacterId() const noexcept { return m_characterId; }

		// ---- authorisation -------------------------------------------------

		// Whether a request of this kind is meaningful yet. The router asks this
		// before dispatching, so an out-of-order request is refused at the
		// boundary instead of reaching gameplay code.
		bool MayRequestCharacterList() const noexcept
		{
			return m_state == SessionState::Authenticated;
		}

		bool MayRequestCharacterSelect() const noexcept
		{
			return m_state == SessionState::Authenticated;
		}

		bool MayRequestEnterWorld() const noexcept
		{
			return m_state == SessionState::CharacterSelected;
		}

		// Only an in-world session may affect the world.
		bool MayAffectWorld() const noexcept { return m_state == SessionState::InWorld; }

		// ---- liveness ------------------------------------------------------

		// Records a heartbeat. Legacy exchanges NET_MSG_HEARTBEAT_CLIENT_ANS (161)
		// and drops a client that goes NET_TIME_OUT milliseconds without one
		// (s_NetGlobal.h:117).
		void NoteHeartbeat() noexcept { m_lastHeartbeatMs = m_clockMs; }
		WireU64 LastHeartbeatMs() const noexcept { return m_lastHeartbeatMs; }

		// Advances the session clock. Called by whoever owns time; the session
		// deliberately reads no clock of its own.
		void AdvanceClock(WireU64 milliseconds) noexcept { m_clockMs += milliseconds; }

		// True once NET_TIME_OUT has elapsed with no heartbeat.
		bool IsTimedOut() const noexcept
		{
			return m_clockMs - m_lastHeartbeatMs >= Protocol::kTimeoutMilliseconds;
		}

	private:
		WireU64      m_sessionId = 0;
		WireU64      m_accountId = 0;
		WireU32      m_characterId = 0;
		SessionState m_state = SessionState::Connected;
		WireU64      m_clockMs = 0;
		WireU64      m_lastHeartbeatMs = 0;
	};
}
