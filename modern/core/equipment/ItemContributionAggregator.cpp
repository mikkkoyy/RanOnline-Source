// VERTICAL-002: item contribution aggregation. See ItemContributionAggregator.h.
//
// Field provenance for every line below is in
// docs/reference/client/VERTICAL-002_EQUIPMENT_INVESTIGATION.md §6, which
// traces each one to `GLCHARLOGIC::SUM_ITEM`.

#include "equipment/ItemContributionAggregator.h"

namespace Modern
{
	Result<ItemContributionResult> ItemContributionAggregator::Aggregate(
		const EquipmentState& equipment, const ItemDefinitionProvider& provider)
	{
		ItemContributionResult result;

		// Slot order, so the accumulation order is a property of the container
		// rather than of a caller's iteration.
		for (const EquipmentEntry& entry : equipment.GetSlots())
		{
			if (!entry.HasItem())
			{
				continue;
			}

			// SUM_ITEM skips a bound item's contribution implicitly by the slot
			// being empty; here the equivalent is refusing an instance that is
			// already bound, so one copy cannot be counted twice.
			if (entry.item.boundTo.IsValid() && entry.item.boundTo != entry.item.definition)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			const ItemDefinition* definition = provider.Find(entry.item.definition);
			if (definition == nullptr)
			{
				result.error = ContributionError::MissingDefinition;
				return result;
			}
			if (!definition->stats.IsFinite())
			{
				result.error = ContributionError::NonFinite;
				return result;
			}

			// An item that is not equipment contributes nothing, which is how a
			// consumable in a slot behaves rather than an error.
			if (!definition->IsEquipment() || definition->stats.IsZero())
			{
				continue;
			}
			++result.contributingSlots;

			const ItemStatBlock& block = definition->stats;

			// The six base stats, added as the 16-bit values RAN adds them as.
			// The wrap happens in Calculate, not here, so there is one rule.
			result.contribution.stats.pow   = static_cast<uint16_t>(result.contribution.stats.pow + block.pow);
			result.contribution.stats.str   = static_cast<uint16_t>(result.contribution.stats.str + block.str);
			result.contribution.stats.spi   = static_cast<uint16_t>(result.contribution.stats.spi + block.spi);
			result.contribution.stats.dex   = static_cast<uint16_t>(result.contribution.stats.dex + block.dex);
			result.contribution.stats.intel = static_cast<uint16_t>(result.contribution.stats.intel + block.intel);
			result.contribution.stats.sta   = static_cast<uint16_t>(result.contribution.stats.sta + block.sta);

			// Flat resources: nHP, nMP, nSP.
			result.contribution.hp += block.hp;
			result.contribution.mp += block.mp;
			result.contribution.sp += block.sp;

			// Recovery rates: fIncR_HP, fIncR_MP, fIncR_SP.
			result.contribution.hpRecoveryRate += block.hpRecoveryRate;
			result.contribution.mpRecoveryRate += block.mpRecoveryRate;
			result.contribution.spRecoveryRate += block.spRecoveryRate;

			// Attack power: GETADDPA, GETADDSA, GETADDENERGY.
			result.contribution.meleePower  += block.meleePower;
			result.contribution.shootPower  += block.shootPower;
			result.contribution.magicAttack += block.magicAttack;

			// Hit and avoid, and their percentage forms.
			result.contribution.hit   += block.hit;
			result.contribution.avoid += block.avoid;
			result.contribution.hitRatePercent   += block.hitPercent;
			result.contribution.avoidRatePercent += block.avoidPercent;

			// Defence and the damage range.
			result.contribution.defense    += block.defense;
			result.contribution.damageLow  += block.damageLow;
			result.contribution.damageHigh += block.damageHigh;

			// The five SRESIST elements, element by element.
			result.contribution.resistances.fire     += block.resistFire;
			result.contribution.resistances.ice      += block.resistIce;
			result.contribution.resistances.electric += block.resistElectric;
			result.contribution.resistances.poison   += block.resistPoison;
			result.contribution.resistances.spirit   += block.resistSpirit;
		}

		return result;
	}
}
