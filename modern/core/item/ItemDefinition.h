#pragma once

#include "../types/Ids.h"

#include <cstdint>
#include <string>

namespace Modern
{
	// What an item is for.
	//
	// RAN's SITEM splits item behaviour across a dozen kind flags with several
	// independent switch statements over the same value. A single tag keeps the
	// core honest; the fine-grained capability flags come back when a system
	// actually needs to query them.
	enum class ItemKind : uint8_t
	{
		None      = 0,
		Weapon    = 1,
		Armor     = 2,
		Accessory = 3,
		Consumable = 4,
		Material  = 5,
		Quest     = 6,
		Misc      = 7,
	};

	const char* ToString(ItemKind kind) noexcept;

	// The base stat block an item definition carries.
	//
	// VERTICAL-002. This is the *definition* half of RAN's item contribution:
	// the values every copy of the item starts from. RAN's per-copy options and
	// refine state are a separate thing and are not here; see
	// ItemInstance, whose own note already defers them, and
	// docs/reference/client/VERTICAL-002_EQUIPMENT_INVESTIGATION.md §7.
	//
	// Every field is traced to `GLCHARLOGIC::SUM_ITEM`
	// (legacy/Lib_Client/G-Logic/GLogixExPC.cpp:441) and to the accessors it
	// calls. The types are the legacy destination types, because they are part
	// of the arithmetic CORE-002 reproduces rather than an implementation
	// detail:
	//
	//   - the six stats are `WORD` and wrap when aggregated (SSUM_ITEM adds
	//     them as `WORD`, and `SCHARSTATS::GetTotal` sums in `WORD`);
	//   - damage is a low/high pair (`GLPADATA` / `GLDWDATA`);
	//   - resistances are the five `SRESIST` elements;
	//   - the hit and avoid percentages are `float` and are applied as
	//     `int(value * (100 + percent) * 0.01f)`.
	//
	// A definition is the right home for these: they are shared by every
	// instance of the item, and putting them on an instance would make two
	// copies of the same sword disagree about what a sword does.
	struct ItemStatBlock
	{
		// Flat bonuses to the six base stats. `wPow`, `wStr`, `wSpi`, `wDex`,
		// `wInt`, `wSta` in RAN, from `EMADD_STATS_*` and `GET_STAT_*`.
		uint16_t pow   = 0;
		uint16_t str   = 0;
		uint16_t spi   = 0;
		uint16_t dex   = 0;
		uint16_t intel = 0;
		uint16_t sta   = 0;

		// Flat resource bonuses: nHP, nMP, nSP.
		int32_t hp = 0;
		int32_t mp = 0;
		int32_t sp = 0;

		// Additive recovery rates: fIncR_HP, fIncR_MP, fIncR_SP. A rate, not an
		// amount per second, for the same reason CORE-002's derived recovery
		// rate is a rate.
		float hpRecoveryRate = 0.0f;
		float mpRecoveryRate = 0.0f;
		float spRecoveryRate = 0.0f;

		// VERTICAL-005: the *second* recovery term, fInc_HP / fInc_MP / fInc_SP.
		//
		// These are a different field from the rates above and are not an
		// alternative spelling of them. RAN's recovery formula adds a
		// percentage-of-maximum term and an absolute term:
		//
		//   GLogixExPC.cpp:3020
		//   fINC_HP = fElap * ( m_sHP.dwMax * fINCR_HP
		//                      + GLCONST_CHAR::fHP_INC
		//                      + m_sSUMITEM.fInc_HP );
		//
		// `fINCR_HP` (the class constant plus fIncR_HP plus the passive) scales
		// with the maximum; `fInc_HP` does not. An item that recovers a flat
		// amount per unit stays equally useful on a character with a small
		// maximum, which is the whole reason RAN carries both.
		//
		// These were absent through CORE-002 and VERTICAL-002, and the comment
		// on `Stats::ItemContribution` claimed the legacy field "is not part of
		// the stat pipeline". That was wrong: it is the absolute term of the
		// same expression. VERTICAL-005 corrects the claim and models the field,
		// because a resource system that only knew the rate would silently drop
		// half of every character's item-granted regeneration.
		float hpRecoveryFlat = 0.0f;
		float mpRecoveryFlat = 0.0f;
		float spRecoveryFlat = 0.0f;

		// Attack power: GETADDPA (melee), GETADDSA (ranged), GETADDENERGY
		// (magic). RAN has no GETADDMA; magic attack arrives as "energy".
		int32_t meleePower  = 0;
		int32_t shootPower  = 0;
		int32_t magicAttack = 0;

		// GETHITRATE / GETAVOIDRATE, and their percentage forms
		// GETHITRATE_PER / GETAVOIDRATE_PER. The base `SSUIT::nHitRate` and
		// `nAvoidRate` are percents, which is why they are separate from the
		// percentage fields below rather than merged into them.
		int32_t hit        = 0;
		int32_t avoid      = 0;
		float   hitPercent   = 0.0f;
		float   avoidPercent = 0.0f;

		// GETDEFENSE, and the damage range from GETDAMAGE.
		int32_t defense    = 0;
		int32_t damageLow  = 0;
		int32_t damageHigh = 0;

		// GETRESIST_FIRE / _ICE / _ELECTRIC / _POISON / _SPIRIT, in SRESIST
		// order. Kept as five named fields rather than an array so the meaning
		// of each is visible at the call site.
		int32_t resistFire     = 0;
		int32_t resistIce      = 0;
		int32_t resistElectric = 0;
		int32_t resistPoison   = 0;
		int32_t resistSpirit   = 0;

		// VERTICAL-007: combat modifiers from items.
		//
		// Legacy: m_sSUMITEM.fIncR_Critical, m_sSUMITEM.fIncR_CrushingBlow,
		// m_sSUMITEM.fInc_Critical, m_sSUMITEM.fInc_CrushingBlow
		// (GLogixExPC.cpp:570-574, 595-596, 624-628)
		//
		// These are rates (floats), not percentage points. The legacy
		// conversion to percentage points happens at the point of use:
		//   nPercentCri += (int)(m_sSUMITEM.fIncR_Critical * 100)
		//   nCrushingBlow = (int)(m_sSUMITEM.fIncR_CrushingBlow * 100)
		float criticalRate = 0.0f;
		float crushingBlow = 0.0f;

		// VERTICAL-007: damage reduction and reflection from items.
		//
		// Legacy: sDamageSpec.m_fPsyDamageReduce, m_fPsyDamageReflection,
		// m_fPsyDamageReflectionRate (GLogixExPC.cpp:1409-1411)
		float damageReduce = 0.0f;
		float damageReflection = 0.0f;
		float damageReflectionRate = 0.0f;

		// True when the block contributes nothing, which lets the aggregator
		// skip a definition cheaply without changing the result.
		bool IsZero() const noexcept;

		bool IsFinite() const noexcept;

		constexpr bool operator==(const ItemStatBlock& other) const noexcept
		{
			return pow == other.pow && str == other.str && spi == other.spi &&
			       dex == other.dex && intel == other.intel && sta == other.sta &&
			       hp == other.hp && mp == other.mp && sp == other.sp &&
			       hpRecoveryRate == other.hpRecoveryRate &&
			       mpRecoveryRate == other.mpRecoveryRate &&
			       spRecoveryRate == other.spRecoveryRate &&
			       hpRecoveryFlat == other.hpRecoveryFlat &&
			       mpRecoveryFlat == other.mpRecoveryFlat &&
			       spRecoveryFlat == other.spRecoveryFlat &&
			       meleePower == other.meleePower && shootPower == other.shootPower &&
			       magicAttack == other.magicAttack && hit == other.hit &&
			       avoid == other.avoid && hitPercent == other.hitPercent &&
			       avoidPercent == other.avoidPercent && defense == other.defense &&
			       damageLow == other.damageLow && damageHigh == other.damageHigh &&
			       resistFire == other.resistFire && resistIce == other.resistIce &&
			       resistElectric == other.resistElectric &&
			       resistPoison == other.resistPoison &&
			       resistSpirit == other.resistSpirit &&
			       criticalRate == other.criticalRate &&
			       crushingBlow == other.crushingBlow &&
			       damageReduce == other.damageReduce &&
			       damageReflection == other.damageReflection &&
			       damageReflectionRate == other.damageReflectionRate;
		}
	};

	// The definition of an item type: the shared, immutable description that
	// many instances refer to.
	//
	// CORE-001 kept this to identity. VERTICAL-002 adds the stat block above,
	// which is the field set the legacy investigation proved feeds
	// `SSUM_ITEM`. RAN's definition also carries price, durability, attack type,
	// attack range and upgrade paths; none of those reach the stat pipeline
	// (see the investigation §9) and none is here.
	struct ItemDefinition
	{
		ItemId      id = ItemId::MakeInvalid();
		ItemKind    kind = ItemKind::None;
		std::string name;
		uint32_t    maxStack = 1;

		// What this item contributes when equipped. Zero for items that are not
		// equipment, which is the default and therefore the common case.
		ItemStatBlock stats;

		bool IsValid() const
		{
			return id.IsValid() && kind != ItemKind::None && !name.empty() && maxStack > 0;
		}

		bool CanStack() const { return maxStack > 1; }

		// Whether equipping this definition could change a character's derived
		// statistics. Lets an aggregator skip consumables and materials
		// entirely, which is most of any inventory.
		bool IsEquipment() const
		{
			return kind == ItemKind::Weapon || kind == ItemKind::Armor ||
			       kind == ItemKind::Accessory;
		}

		friend bool operator==(const ItemDefinition& lhs, const ItemDefinition& rhs)
		{
			return lhs.id == rhs.id;
		}

		friend bool operator!=(const ItemDefinition& lhs, const ItemDefinition& rhs)
		{
			return !(lhs == rhs);
		}
	};
}
