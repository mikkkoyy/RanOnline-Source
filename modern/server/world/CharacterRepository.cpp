#include "world/CharacterRepository.h"

namespace Modern::Server::World
{
	// Every function below takes m_mutex for the whole of its map access and no
	// longer. The pattern is the same in all of them and is worth stating once:
	//
	//   - The record is COPIED out of the map before the lock is released
	//     (Result<WorldCharacter> holds a value, not a reference). That is what lets
	//     the lock cover only a map operation rather than the caller's whole use of
	//     the result, which would otherwise serialise two Field workers for as long as
	//     one of them spent building a spawn packet.
	//   - Validation runs OUTSIDE the lock. It touches only the argument, so holding
	//     the lock across it would be pure contention.
	//   - No I/O happens under the lock.
	//
	// See the m_mutex comment in the header for why this locking exists at all.

	Status InMemoryCharacterRepository::Add(const WorldCharacter& character)
	{
		if (const Status status = character.Validate(); status.IsError())
		{
			return status;
		}

		const std::lock_guard<std::mutex> lock(m_mutex);

		if (m_characters.find(character.id.value) != m_characters.end())
		{
			// Refused rather than overwritten. An overwrite would silently transfer
			// ownership: the same character id would answer to a different account,
			// and the 2248 list an account was already sent would no longer describe
			// what that account owns.
			return Status(ErrorCode::AlreadyExists);
		}

		m_characters.emplace(character.id.value, character);
		return Ok();
	}

	Status InMemoryCharacterRepository::Replace(const WorldCharacter& character)
	{
		if (const Status status = character.Validate(); status.IsError())
		{
			return status;
		}

		const std::lock_guard<std::mutex> lock(m_mutex);

		const auto it = m_characters.find(character.id.value);
		if (it == m_characters.end())
		{
			return Status(ErrorCode::NotFound);
		}

		it->second = character;
		return Ok();
	}

	Status InMemoryCharacterRepository::ListByAccount(
	    WorldAccountId accountId, std::vector<WorldCharacter>& out) const
	{
		// Built locally and swapped in at the end, so a caller passing a vector that is
		// also aliased elsewhere cannot observe it half-filled.
		std::vector<WorldCharacter> found;

		{
			const std::lock_guard<std::mutex> lock(m_mutex);

			// A std::map walk is already in ascending character-id order, so no sort is
			// needed and no insertion order can leak into the result.
			for (const auto& entry : m_characters)
			{
				if (entry.second.accountId == accountId)
				{
					found.push_back(entry.second);
				}
			}
		}

		out = std::move(found);
		return Ok();
	}

	Result<WorldCharacter> InMemoryCharacterRepository::Find(WorldCharacterId id) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);

		const auto it = m_characters.find(id.value);
		if (it == m_characters.end())
		{
			return Result<WorldCharacter>(Status(ErrorCode::NotFound));
		}
		return Result<WorldCharacter>(it->second);
	}

	Result<WorldCharacter> InMemoryCharacterRepository::FindOwned(
	    WorldAccountId accountId, WorldCharacterId characterId) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);

		const auto it = m_characters.find(characterId.value);
		if (it == m_characters.end())
		{
			// Unknown character. Same code as "not yours" - see the header for why the
			// two are deliberately indistinguishable.
			return Result<WorldCharacter>(Status(ErrorCode::NotFound));
		}

		// THE OWNERSHIP CHECK. `character.accountId == authenticatedSession.accountId`,
		// written down where a reviewer can find it.
		//
		// Legacy states this as the SQL predicate
		//   FROM ChaInfo WHERE ChaNum=%d AND UserNum=%d
		// (s_COdbcGameChaGet.cpp:41). It is the same rule; here it is a line of C++
		// that fails to compile if the account id is removed.
		if (it->second.accountId != accountId)
		{
			return Result<WorldCharacter>(Status(ErrorCode::NotFound));
		}

		return Result<WorldCharacter>(it->second);
	}

	Status InMemoryCharacterRepository::AssignGaeaId(WorldCharacterId characterId,
	                                                 Network::WireU32 gaeaId)
	{
		// 0 is the "not in the world" sentinel; assigning it would erase the
		// placement rather than record one. Checked before the lock because it needs
		// no shared state at all.
		if (gaeaId == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::lock_guard<std::mutex> lock(m_mutex);

		const auto it = m_characters.find(characterId.value);
		if (it == m_characters.end())
		{
			return Status(ErrorCode::NotFound);
		}

		it->second.gaeaId = gaeaId;
		return Ok();
	}

	Result<Network::WireU32> InMemoryCharacterRepository::ReadGaeaId(
	    WorldCharacterId characterId) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);

		const auto it = m_characters.find(characterId.value);
		if (it == m_characters.end())
		{
			// NotFound rather than 0, so a caller cannot read "no such character" as
			// "not in the world" and treat the two as the same state.
			return Result<Network::WireU32>(Status(ErrorCode::NotFound));
		}
		return Result<Network::WireU32>(it->second.gaeaId);
	}
}