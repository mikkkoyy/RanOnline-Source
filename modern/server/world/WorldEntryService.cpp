#include "world/WorldEntryService.h"

namespace Modern::Server::World
{
	using namespace Modern::Network;

	// ---------------------------------------------------------------------------
	// FieldEntryRegistry
	// ---------------------------------------------------------------------------

	Result<WorldEntryAuthorization> FieldEntryRegistry::Reserve(
	    ICharacterRepository& repository, WorldAccountId accountId, WorldCharacterId characterId,
	    WireU64 agentSessionId, WireU64 nowMs)
	{
		// Ownership re-proven at the Field, not trusted from the Agent.
		//
		// The Agent already checked, and that check is the one the brief requires.
		// But the Field is where the character will exist, and it is the Field that a
		// client will next present a claim to - so the role that owns the decision
		// proves it for itself. Two checks of one rule is not redundancy for its own
		// sake: it means a bug in the Agent's path cannot become a way in.
		const Result<WorldCharacter> found =
		    repository.FindOwned(accountId, characterId);
		if (found.IsError())
		{
			return Result<WorldEntryAuthorization>(found.GetStatus());
		}

		const WorldCharacter& character = found.GetValue();

		// A NEW gaeaId every time. See the header: re-using it would make a stale
		// authorization accidentally valid.
		if (m_nextGaeaId == kInvalidGaeaId)
		{
			// Refused rather than wrapped. Wrapping to 1 would collide with a live
			// allocation, and a collision here means two characters can present the
			// same entity id.
			return Result<WorldEntryAuthorization>(Status(ErrorCode::InvalidState));
		}
		const WireU32 gaeaId = m_nextGaeaId;

		WorldEntryAuthorization authorization;
		authorization.authorizationId = m_nextAuthorizationId;
		authorization.agentSessionId  = agentSessionId;
		authorization.accountId       = accountId;
		authorization.characterId     = characterId;
		authorization.userId          = character.userId;
		authorization.gaeaId          = gaeaId;
		authorization.slotFieldAgent  = m_nextSlot;
		authorization.createdAtMs     = nowMs;
		authorization.consumed        = false;

		// The repository now owns the entity id, which is what spawn reads and what a
		// later validation compares against. Refusing here means no authorization
		// exists for a character that was not actually placed.
		if (const Status status = repository.AssignGaeaId(characterId, gaeaId);
		    status.IsError())
		{
			return Result<WorldEntryAuthorization>(status);
		}

		m_pending.emplace(gaeaId, authorization);

		++m_nextGaeaId;
		++m_nextSlot;
		++m_nextAuthorizationId;
		return Result<WorldEntryAuthorization>(authorization);
	}

	Result<ValidatedWorldEntry> FieldEntryRegistry::Validate(
	    const ICharacterRepository& repository,
	    const Network::FieldIdentity& identity, WireU64 nowMs)
	{
		// The join type is a plain 4-byte enum on the wire
		// (EMGAME_JOINTYPE, s_NetGlobal.h:4156-4161). Only the three declared values
		// are accepted; a fourth would be a field the protocol has no meaning for.
		if (identity.joinType != Network::WorldEntry::kJoinTypeFirst &&
		    identity.joinType != Network::WorldEntry::kJoinTypeMoveMap &&
		    identity.joinType != Network::WorldEntry::kJoinTypeRebirth)
		{
			return Result<ValidatedWorldEntry>(Status(ErrorCode::InvalidArgument));
		}

		const auto it = m_pending.find(identity.gaeaId);
		if (it == m_pending.end())
		{
			// Nothing was ever authorized with this gaeaId. NotFound, not
			// InvalidArgument: from the Field's side the client named an entity that
			// does not exist, and "no such entity" is the truthful answer.
			return Result<ValidatedWorldEntry>(Status(ErrorCode::NotFound));
		}

		const WorldEntryAuthorization& authorization = it->second;

		// The slot. Checked separately from the gaeaId because a client that has a
		// valid gaeaId and a wrong slot is a different failure from one that has
		// neither, and the two must not be reported identically to a caller that might
		// be probing.
		if (identity.slotFieldAgent != authorization.slotFieldAgent)
		{
			return Result<ValidatedWorldEntry>(Status(ErrorCode::NotFound));
		}

		// Replay. Checked before anything else that could be expensive, and it is the
		// reason a captured 2359 is worthless a second time.
		if (authorization.consumed)
		{
			return Result<ValidatedWorldEntry>(Status(ErrorCode::NotAllowed));
		}

		// Staleness, on the same clock legacy uses for a client it has stopped
		// hearing from (NET_TIME_OUT, kTimeoutMilliseconds).
		if (nowMs - authorization.createdAtMs >= Protocol::kTimeoutMilliseconds)
		{
			return Result<ValidatedWorldEntry>(Status(ErrorCode::NotAllowed));
		}

		// The character must still exist...
		const Result<WorldCharacter> found =
		    repository.Find(authorization.characterId);
		if (found.IsError())
		{
			return Result<ValidatedWorldEntry>(found.GetStatus());
		}
		const WorldCharacter& character = found.GetValue();

		// ...still belong to the authorized account...
		if (character.accountId != authorization.accountId)
		{
			return Result<ValidatedWorldEntry>(Status(ErrorCode::NotFound));
		}

		// ...and still hold THIS gaeaId.
		//
		// This is the check that makes re-entry safe. A character that entered the
		// world again since this authorization was minted has a different gaeaId, so
		// the old pair names a previous appearance and is refused. Without it, an
		// authorization from a first attempt would still admit a second one.
		if (character.gaeaId != authorization.gaeaId)
		{
			return Result<ValidatedWorldEntry>(Status(ErrorCode::NotAllowed));
		}

		ValidatedWorldEntry validated;
		validated.authorization = authorization;
		validated.character     = character;
		return Result<ValidatedWorldEntry>(validated);
	}

	Result<ValidatedWorldEntry> FieldEntryRegistry::Claim(
	    const ICharacterRepository& repository,
	    const Network::FieldIdentity& identity, WireU64 nowMs)
	{
		Result<ValidatedWorldEntry> validated =
		    Validate(repository, identity, nowMs);
		if (validated.IsError())
		{
			return validated;
		}

		// Consume on success only. A failed validation must leave the authorization
		// claimable, or a client that mistyped its slot would have spent the one
		// attempt it legitimately had.
		m_pending[identity.gaeaId].consumed = true;
		return validated;
	}

	void FieldEntryRegistry::Discard(WireU32 gaeaId) noexcept
	{
		const auto it = m_pending.find(gaeaId);
		if (it == m_pending.end())
		{
			return;
		}

		// Zero is refused by AssignGaeaId, so the entity id is left in place rather
		// than cleared: the character was placed, and pretending otherwise would let
		// a second entry look like a first. The authorization simply stops being
		// claimable.
		it->second.consumed = true;
	}

	std::size_t FieldEntryRegistry::PendingCount() const noexcept
	{
		std::size_t pending = 0;
		for (const auto& entry : m_pending)
		{
			if (!entry.second.consumed)
			{
				++pending;
			}
		}
		return pending;
	}

	// ---------------------------------------------------------------------------
	// WorldEntryService
	// ---------------------------------------------------------------------------

	Result<Network::FieldRedirect> WorldEntryService::AuthorizeWorldEntry(
	    AgentSession& session)
	{
		m_hasLastAuthorization = false;

		// The state rule. Only CharacterSelected may authorize, which is what makes
		// "AUTHENTICATED -> world entry without a selection" unreachable.
		if (!session.MayRequestWorldEntry())
		{
			return Result<Network::FieldRedirect>(Status(ErrorCode::NotAllowed));
		}

		const WorldCharacter* selected = session.SelectedCharacter();
		if (selected == nullptr)
		{
			// Unreachable while the state machine holds, and checked anyway: a session
			// claiming CharacterSelected with no character would otherwise be built
			// from a null dereference.
			return Result<Network::FieldRedirect>(Status(ErrorCode::InvalidState));
		}

		// The endpoint must be real. An unset endpoint is refused rather than
		// defaulted, because a default here becomes a 2358 carrying an address the
		// client will actually try to dial in Phase C.
		if (m_endpoint.address.empty() ||
		    m_endpoint.address.size() >= Network::WorldEntry::kAddressFieldSize)
		{
			return Result<Network::FieldRedirect>(Status(ErrorCode::InvalidArgument));
		}
		if (m_endpoint.servicePort <= 0 || m_endpoint.servicePort > 65535)
		{
			return Result<Network::FieldRedirect>(Status(ErrorCode::InvalidArgument));
		}

		// Re-prove ownership. The character was owned when it was selected; a
		// repository that moved it to another account in between must not have that
		// selection authorized. This is the check that makes the authorization a
		// statement about NOW rather than about a moment ago.
		const Result<WorldCharacter> owned = m_repository.FindOwned(
		    session.Account(), selected->id);
		if (owned.IsError())
		{
			return Result<Network::FieldRedirect>(owned.GetStatus());
		}

		// The in-process 2356/2357: ask the Field role to place the character and
		// hand back a claimable authorization.
		const Result<WorldEntryAuthorization> authorization = m_registry.Reserve(
		    m_repository, session.Account(), selected->id, session.SessionId(),
		    session.NowMs());
		if (authorization.IsError())
		{
			return Result<Network::FieldRedirect>(authorization.GetStatus());
		}

		// The state change last, so a refused authorization never leaves the Agent
		// claiming the world entry succeeded.
		if (const Status status = session.AuthorizeWorldEntry(); status.IsError())
		{
			// Give the claim back: the client never learned this gaeaId, so leaving it
			// pending would keep a usable pair alive that nothing references.
			m_registry.Discard(authorization.GetValue().gaeaId);
			return Result<Network::FieldRedirect>(status);
		}

		m_lastAuthorization     = authorization.GetValue();
		m_hasLastAuthorization = true;

		// The 2358 the client will use. Every field is the value the Field role just
		// allocated or the configured endpoint - nothing is a constant standing in
		// for a value the client is expected to ignore.
		Network::FieldRedirect redirect;
		redirect.joinType       = Network::WorldEntry::kJoinTypeFirst;
		redirect.gaeaId         = m_lastAuthorization.gaeaId;
		redirect.slotFieldAgent = m_lastAuthorization.slotFieldAgent;
		redirect.servicePort    = m_endpoint.servicePort;
		redirect.fieldIp        = m_endpoint.address;

		return Result<Network::FieldRedirect>(redirect);
	}
}