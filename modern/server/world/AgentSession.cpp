#include "world/AgentSession.h"

#include <utility>

namespace Modern::Server::World
{
	const char* ToString(AgentState state) noexcept
	{
		switch (state)
		{
		case AgentState::Connected:               return "Connected";
		case AgentState::Authenticated:           return "Authenticated";
		case AgentState::CharacterListRequested:  return "CharacterListRequested";
		case AgentState::CharacterListSent:       return "CharacterListSent";
		case AgentState::CharacterSelected:       return "CharacterSelected";
		case AgentState::WorldEntryAuthorized:    return "WorldEntryAuthorized";
		case AgentState::Closed:                  return "Closed";
		}

		// Unreachable: every enumerator is handled above, and a new one would be a
		// compile-warning-free addition that silently printed this. Naming it is
		// better than returning a plausible-looking wrong state name in a log.
		return "<invalid AgentState>";
	}

	Status AgentSession::CompleteAuthentication(WorldAccountId accountId, std::string userId,
	                                            bool rejected)
	{
		if (m_state != AgentState::Connected)
		{
			return Status(ErrorCode::NotAllowed);
		}

		if (rejected)
		{
			// Nothing is recorded. A rejected login must leave no account id behind,
			// or a later step could authenticate a session that never was.
			return Status(ErrorCode::NotAllowed);
		}

		// An account id of 0 is refused: WorldCharacter::Validate rejects it for the
		// same reason, and an Agent session holding 0 would make every ownership
		// check answer "not yours" for reasons that look like a repository bug.
		if (accountId.value == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (userId.empty())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		m_accountId = accountId;
		m_userId    = std::move(userId);
		m_state     = AgentState::Authenticated;
		return Ok();
	}

	Status AgentSession::BeginCharacterList() noexcept
	{
		if (m_state != AgentState::Authenticated)
		{
			return Status(ErrorCode::NotAllowed);
		}
		m_state = AgentState::CharacterListRequested;
		return Ok();
	}

	Status AgentSession::CompleteCharacterList() noexcept
	{
		if (m_state != AgentState::CharacterListRequested)
		{
			return Status(ErrorCode::NotAllowed);
		}
		m_state = AgentState::CharacterListSent;
		return Ok();
	}

	Status AgentSession::SelectCharacter(const WorldCharacter& character)
	{
		if (m_state != AgentState::CharacterListSent)
		{
			// Refuses: Connected -> select, Authenticated -> select (before the list
			// was sent), CharacterListRequested -> select, a duplicate re-selection,
			// and any selection after world entry was authorized. All of them reach
			// the same rule, which is the point of having one rule.
			return Status(ErrorCode::NotAllowed);
		}

		// The record is re-validated here, not only in the repository. A session is
		// constructible by a caller that never went through CharacterSelectService,
		// and an unvalidated record stored here would be authoritative by accident.
		if (const Status status = character.Validate(); status.IsError())
		{
			return status;
		}

		// And the session's own account must own it. This is the second of the two
		// ownership checks in the flow, and it is the one that survives a service
		// that forgets: the session knows what it authenticated as.
		if (character.accountId != m_accountId)
		{
			return Status(ErrorCode::NotAllowed);
		}

		m_selected     = character;
		m_hasSelection = true;
		m_state        = AgentState::CharacterSelected;
		return Ok();
	}

	Status AgentSession::AuthorizeWorldEntry() noexcept
	{
		// The transition the whole Agent role exists to make. From any other state
		// this is refused rather than forced, so "world entry without a selection" has
		// no path at all.
		if (m_state != AgentState::CharacterSelected)
		{
			return Status(ErrorCode::NotAllowed);
		}
		m_state = AgentState::WorldEntryAuthorized;
		return Ok();
	}

	bool AgentSession::IsAuthenticated() const noexcept
	{
		return m_accountId.value != 0 && m_state != AgentState::Connected &&
		       m_state != AgentState::Closed;
	}

	const WorldCharacter* AgentSession::SelectedCharacter() const noexcept
	{
		return m_hasSelection ? &m_selected : nullptr;
	}
}