#include "world/CombatStatsProvider.h"

#include "character/CharacterClassTable.h"
#include "equipment/ItemContributionAggregator.h"

#include <cstddef>
#include <cstdint>

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

	namespace
	{
		// Aggregates what the character is wearing.
		//
		// `ItemContributionAggregator` already exists and already carries the
		// legacy numeric semantics (the six stat bonuses are 16-bit and wrap, the
		// damage range is two independent integers), so this is the aggregation
		// `SUM_ITEM` (GLogixExPC.cpp:441-669) does and NOT a second copy of it.
		//
		// A null item provider is "no items are known", which is the deployment
		// without item data. Every equipped slot then names an item nothing
		// defines, which the aggregator reports as `MissingDefinition` - counted,
		// and not contributing anything. That is the honest reading: a character
		// wearing things in a server that has no item table genuinely has no
		// item-derived statistics.
		void AggregateEquipment(const WorldCharacter& character,
		                        const ItemDefinitionProvider* itemDefinitions,
		                        CombatStats& out)
		{
			out.occupiedSlots = 0;
			for (std::size_t slot = 0; slot < Modern::kEquipmentSlotCount; ++slot)
			{
				if (character.equipment.HasEquipped(
				        static_cast<Modern::EquipmentSlot>(
				            static_cast<uint8_t>(slot))))
				{
					++out.occupiedSlots;
				}
			}

			if (itemDefinitions == nullptr)
			{
				// Nothing can be resolved, so nothing contributes. The occupied
				// count above already says so.
				out.unresolvedItems = out.occupiedSlots;
				return;
			}

			const auto aggregated = ItemContributionAggregator::Aggregate(
			    character.equipment, *itemDefinitions);

		if (!aggregated.IsOk())
		{
			// A Status-level refusal is a caller fault (an invalid slot, say) and
			// is reported as one. It does not become a zero contribution by a
			// different route.
			out.unresolvedItems = out.occupiedSlots;
			return;
		}

		const ItemContributionResult& result = aggregated.GetValue();

		// A definition that could not be resolved is reported by the aggregator as
		// `MissingDefinition` rather than as a zero block, because equipment that
		// contributes nothing because nothing knows what it is would hide a data
		// problem behind a plausible number.
		if (result.error != ContributionError::None)
		{
			out.unresolvedItems = out.occupiedSlots;
			return;
		}

		out.equipment         = result.contribution;
		out.contributingSlots = result.contributingSlots;
	}
	} // anonymous namespace

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

		// ---- the equipment this character is wearing ----------------------------
		//
		// Aggregated before the class figures, because a worn item's damage is
		// added to the base range by `Stats::Calculate` and its hit and avoid are
		// added to the derived hit and avoid - both exactly once, in the
		// calculator, not here.
		AggregateEquipment(character, m_itemDefinitions, out);

		Stats::StatCalculationInput input;
		input.characterClass = index;
		input.level          = character.level;
		input.classConstants = row->constants;

		// Equipment comes from the aggregation above. Passives, codex and timed
		// facts remain zero, statelessly so: none of those subsystems exists in
		// the live world server, and RAN's term for "none of those" is zero.
		input.items   = out.equipment;
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
