#include "world/CombatStatsProvider.h"

#include "character/CharacterClassTable.h"

namespace Modern::Server::World
{
	bool ClassConstantCombatStats::TryResolveClassIndex(const WorldCharacter& character,
	                                                   Stats::CharClassIndex& out) noexcept
	{
		// `characterClass` is carried raw and its numeric space is RAN's
		// `EMCHARCLASS`, which `Modern::CharacterClass` reproduces 1:1 (Brawler
		// == 1 through Extreme == 8). Values outside that range cannot be
		// constructed safely - the enum's underlying type is uint8_t and the
		// field is wider - so the range is checked before the cast rather than
		// trusted after it. Same conversion and same reason as
		// CharacterClassMovementSpeed.cpp:15-29.
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
		    static_cast<CharacterClass>(static_cast<uint8_t>(character.characterClass)),
		    static_cast<CharacterGender>(static_cast<uint8_t>(character.characterGender)),
		    out);
	}

	bool ClassConstantCombatStats::TryResolve(const WorldCharacter& character,
	                                          CombatStats& out) const
	{
		Stats::CharClassIndex index{};
		if (!TryResolveClassIndex(character, index))
		{
			return false;
		}

		const Stats::ClassConstantRow* row = Stats::ClassConstantTable::Verified().Find(index);
		if (row == nullptr)
		{
			// A class that resolves but has no row is a table gap, not a
			// character fault. Refused rather than answered.
			return false;
		}

		if (row->source != Stats::CoefficientSource::Recovered)
		{
			// The honest answer today. Reported through the out parameter so a
			// caller that wants to log WHY can, but `out` is not written as a
			// usable answer: a caller must check the return value.
			out.source = row->source;
			out.resolvedIndex = index;
			return false;
		}

		// The level is the character's, and it is authoritative - 002I already
		// ignores any client-supplied level. RAN's level term is (level - 1),
		// which `Stats::Calculate` applies itself.
		if (!Stats::IsValidLevel(character.level))
		{
			return false;
		}

		Stats::StatCalculationInput input;
		input.characterClass = index;
		input.level          = character.level;
		input.classConstants = row->constants;

		// Zero equipment, passives, codex and timed facts. Each is a statement
		// about this runtime: none of those subsystems exists in the live world
		// server yet, and RAN's contribution for "nothing equipped" is zero.
		// When equipment lands it arrives as an ItemContribution here, and the
		// formula does not change.
		input.items   = Stats::ItemContribution{};
		input.passives = Stats::PassiveContribution{};
		input.codex   = Stats::CodexContribution{};
		input.facts   = Stats::FactContribution{};

		// 1.0f is "no configuration adjustment", which is the honest value when
		// no configuration point rate has been loaded.
		input.confPointRate = 1.0f;

		const Modern::Result<Stats::DerivedStats> calculated = Stats::Calculate(input);
		if (!calculated.IsOk())
		{
			// A refusal from the calculator is passed on rather than worked
			// around: the calculator refuses for a reason this layer cannot fix.
			out.source = row->source;
			out.resolvedIndex = index;
			return false;
		}

		out.derived      = calculated.GetValue();
		out.resolvedIndex = index;
		out.source       = row->source;
		return true;
	}

} // namespace Modern::Server::World
