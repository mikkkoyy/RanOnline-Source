#include "world/CharacterSelectService.h"

#include <algorithm>

namespace Modern::Server::World
{
	using namespace Modern::Network;

	Status CharacterSelectService::BuildCharacterList(AgentSession& session,
	                                                 CharacterListResult& out)
	{
		out.frame.clear();
		out.ids.clear();
		out.expectedDetailCount = 0;

		if (!session.MayRequestCharacterList())
		{
			return Status(ErrorCode::NotAllowed);
		}

		// Checked before the state advances, so a refusal leaves the session exactly
		// where it was.
		if (const Status status = session.BeginCharacterList(); status.IsError())
		{
			return status;
		}

		std::vector<WorldCharacter> owned;
		if (const Status status = m_repository.ListByAccount(session.Account(), owned);
		    status.IsError())
		{
			return status;
		}

		// This build can announce at most kLocalSlots characters. An account with more
		// is a repository/server mismatch, not something to paper over by sending a
		// partial list: a client shown three of five characters would select one and
		// never learn the other two existed.
		if (owned.size() > CharacterList::kLocalSlots)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.ids.reserve(owned.size());
		for (const WorldCharacter& character : owned)
		{
			out.ids.push_back(character.id.value);
		}

		// Straight through to the Phase A codec. It emits this build's FIXED width and
		// zero-fills the unused slots; nothing here reformats or re-lengths it.
		if (const Status status = CharacterListCodec::AppendIdList(out.frame, out.ids);
		    status.IsError())
		{
			out.frame.clear();
			out.ids.clear();
			return status;
		}

		// Asked through the Phase A helper rather than written as ids.size(), so the
		// number the client is told to expect and the number the decoder would report
		// come from one definition.
		CharacterIdList asList;
		asList.ids = out.ids;
		out.expectedDetailCount = CharacterListCodec::ExpectedDetailCount(asList);

		// An account with no characters is a legitimate state: it still gets a
		// well-formed 28-byte 2248 with nChaSNum = 0, and the client shows an empty
		// list and stops. There is no error and no special packet.
		return session.CompleteCharacterList();
	}

	Status CharacterSelectService::BuildCharacterDetail(AgentSession& session,
	                                                    WireU32 characterId,
	                                                    std::vector<WireU8>& out)
	{
		out.clear();

		if (!session.MayRequestCharacterSelect())
		{
			return Status(ErrorCode::NotAllowed);
		}

		// Ownership-authoritative. A 2244 for another account's character is
		// NotFound, not an empty record and not a permission error that would confirm
		// the character exists.
		const Result<WorldCharacter> found =
		    m_repository.FindOwned(session.Account(), WorldCharacterId{ characterId });
		if (found.IsError())
		{
			return found.GetStatus();
		}

		const WorldCharacter& character = found.GetValue();

		// The 2332 subset. Note what is NOT here: mp, sp and the position. 2332 does
		// not carry them - SCHARINFO_LOBBY has m_sHP but no MP or SP, and no position
		// at all (the investigation §3.2 lists the fields, and position is absent).
		// Mapping them in would be inventing wire fields.
		CharacterDetail detail;
		detail.characterId   = character.id.value;
		detail.name          = character.name;
		detail.characterClass = character.characterClass;
		detail.school        = character.school;
		detail.level         = character.level;
		detail.hp            = character.hp;
		detail.saveMapId     = character.saveMapId;

		return CharacterListCodec::AppendCharacterDetail(out, detail);
	}

	Status CharacterSelectService::SelectCharacter(AgentSession& session,
	                                               WireU32 characterId)
	{
		// The state rule first. An unauthenticated session must not reach the
		// repository at all: a lookup for an unauthenticated caller would be a
		// character-existence oracle.
		if (!session.MayRequestCharacterSelect())
		{
			return Status(ErrorCode::NotAllowed);
		}

		// Character id 0 is refused here rather than being allowed to miss in the
		// repository. WorldCharacter::Validate forbids a stored id of 0, so this can
		// only ever be NotFound, and refusing it directly states the intent: 0 is
		// not a character.
		if (characterId == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// THE OWNERSHIP-AUTHORITATIVE LOOKUP.
		//
		// Legacy's equivalent is the SQL predicate
		//   FROM ChaInfo WHERE ChaNum=%d AND UserNum=%d
		// (s_COdbcGameChaGet.cpp:41), with UserNum bound to the session's own account
		// at :650. Here the account is AgentSession::Account(), which the Agent set
		// from the login result and no client message can change.
		//
		// NotFound covers BOTH "no such character" and "not yours", so a client cannot
		// use the response to discover which character ids exist elsewhere.
		const Result<WorldCharacter> found =
		    m_repository.FindOwned(session.Account(), WorldCharacterId{ characterId });
		if (found.IsError())
		{
			return found.GetStatus();
		}

		// The session performs the second check and owns the state change. It is
		// given the RESOLVED record, so nothing downstream ever sees the client's id
		// without having been resolved first.
		return session.SelectCharacter(found.GetValue());
	}
}