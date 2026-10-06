#include "world/CharacterClassMovementSpeed.h"

#include "MovementStateProtocol.h"

namespace Modern::Server::World
{
	bool CharacterClassMovementSpeed::TryResolveClassIndex(const WorldCharacter& character,
	                                                      Stats::CharClassIndex& out) const noexcept
	{
		// `characterClass` is carried raw and its numeric space is RAN's
		// `EMCHARCLASS`, which `Modern::CharacterClass` reproduces 1:1 (Brawler == 1
		// through Extreme == 8). Values outside that range cannot be constructed
		// safely - the enum's underlying type is uint8_t and the field is wider - so
		// the range is checked before the cast rather than trusted after it.
		if (character.characterClass > static_cast<Network::WireU32>(
		                                static_cast<std::uint8_t>(CharacterClass::Extreme)))
		{
			return false;
		}
		if (character.characterGender >
		    static_cast<Network::WireU8>(static_cast<std::uint8_t>(CharacterGender::Female)))
		{
			return false;
		}

		return TryToCharClassIndex(
		    static_cast<CharacterClass>(static_cast<std::uint8_t>(character.characterClass)),
		    static_cast<CharacterGender>(static_cast<std::uint8_t>(character.characterGender)),
		    out);
	}

	bool CharacterClassMovementSpeed::TryMaxSpeed(const WorldCharacter& character,
	                                             float& out) const noexcept
	{
		Stats::CharClassIndex index{};
		if (!TryResolveClassIndex(character, index))
		{
			out = 0.0f;
			return false;
		}

		// The only bit that chooses the branch is EM_ACT_RUN, and it is read from the
		// AUTHORITATIVE word - GLChar.cpp:4968 reads `IsSTATE(EM_ACT_RUN)`, the
		// server's own state, never anything the client sent in a 3034 directly.
		const bool running =
		    (character.actState & Network::MovementState::kActRun) != 0;

		return TryBaseVelocity(index, running, out);
	}

	float CharacterClassMovementSpeed::MaxSpeedFor(const WorldCharacter& character) const
	{
		float speed = 0.0f;
		(void)TryMaxSpeed(character, speed);
		return speed;
	}
}
