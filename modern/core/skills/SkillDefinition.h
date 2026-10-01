#pragma once

// VERTICAL-003: passive skill definition — what a passive skill contributes.
//
// This is the definition half: the immutable data a skill carries, shared by
// every character who learns it. A character's learned skills are tracked in
// SkillState; this type is what the aggregator reads.
//
// Legacy provenance: maps to the passive-relevant subset of SKILL::SAPPLY
// (GLSkillApply.h) — the basic type, per-level values, impacts, specs, and
// the weapon requirements from SKILL::SSKILLBASIC (GLSkillBasic.h).
//
// Only fields that feed the stat pipeline (SUM_PASSIVE -> SPASSIVE_SKILL_DATA
// -> SUM_ADDITION) are carried. Movement speed, attack speed, critical,
// crushing blow, damage reduction, pierce, skill ranges, and summon time are
// not part of the verified stat pipeline and are not here.

#include "item/ItemDefinition.h"
#include "stats/Contributions.h"
#include "status/StatusEffectTypes.h"
#include "types/Ids.h"

#include <array>
#include <cstdint>
#include <string>

namespace Modern
{
	// The maximum skill level RAN uses. Matches SKILL::MAX_LEVEL.
	constexpr uint8_t kMaxSkillLevel = 9;

	// The maximum number of impact entries per skill. Matches SKILL::MAX_IMPACT.
	constexpr uint8_t kMaxSkillImpacts = 5;

	// The maximum number of spec entries per skill. Matches SKILL::MAX_SPEC.
	constexpr uint8_t kMaxSkillSpecs = 5;

	// A skill's native identity: class index + skill index within that class.
	// Matches SNATIVEID (wMainID, wSubID).
	struct SkillId
	{
		uint16_t classIndex = 0;   // wMainID: skill class/category
		uint16_t skillIndex = 0;   // wSubID: skill index within class

		constexpr bool IsValid() const noexcept
		{
			return classIndex != 0xFFFF && skillIndex != 0xFFFF;
		}

		constexpr bool operator==(const SkillId& other) const noexcept
		{
			return classIndex == other.classIndex && skillIndex == other.skillIndex;
		}
		constexpr bool operator!=(const SkillId& other) const noexcept { return !(*this == other); }

		// Ordering for use as map key.
		constexpr bool operator<(const SkillId& other) const noexcept
		{
			return classIndex < other.classIndex ||
			       (classIndex == other.classIndex && skillIndex < other.skillIndex);
		}
	};

	// Weapon type a passive skill may require. Matches SKILL::GLSKILL_ATT
	// (GLSkillBasic.h). Only the values actually used by passive skill
	// weapon checks are carried.
	enum class SkillWeaponType : uint8_t
	{
		None            = 0,   // SKILLATT_NOTHING
		Sword           = 1,
		Blade           = 2,
		SwordBlade      = 3,
		Dagger          = 4,
		Spear           = 5,
		Stick           = 6,
		Gauntlet        = 7,
		Bow             = 8,
		Throw           = 9,
		DualGun         = 10,
		RailGun         = 11,
		PortalGun       = 12,
		Gun             = 13,
		Shotgun         = 14,
		Scythe          = 15,
		DualSpear       = 16,
		Shuriken        = 17,
		Fist            = 18,
		Wand            = 19,
		Cube            = 20,
		Whip            = 21,
		NoCare          = 22,  // SKILLATT_NOCARE — no weapon requirement
		Shield          = 23,
		Hammer          = 24,
		Umbrella        = 25,
	};

	// Which hand slot a weapon requirement applies to.
	enum class SkillWeaponSlot : uint8_t
	{
		LeftHand  = 0,
		RightHand = 1,
	};

	// Passive skill basic apply type. Directly mirrors SKILL::EMTYPES
	// (GLSkillApply.h) for the types that actually feed the stat pipeline.
	enum class PassiveApplyType : uint8_t
	{
		// Flat resource bonuses (DWORD cast in legacy).
		Hp      = 0,   // EMFOR_HP
		Mp      = 1,   // EMFOR_MP
		Sp      = 2,   // EMFOR_SP

		// Recovery rate additions (float).
		VarHp   = 3,   // EMFOR_VARHP
		VarMp   = 4,   // EMFOR_VARMP
		VarSp   = 5,   // EMFOR_VARSP

		// Flat defense.
		Defense = 6,   // EMFOR_DEFENSE

		// Flat hit/avoid.
		HitRate   = 8,  // EMFOR_HITRATE
		AvoidRate = 9,  // EMFOR_AVOIDRATE

		// All three resource rates at once.
		VarAp     = 10, // EMFOR_VARAP

		// Flat damage/attack power.
		VarDamage   = 11, // EMFOR_VARDAMAGE
		VarDefense  = 12, // EMFOR_VARDEFENSE

		// Flat attack powers.
		Pa    = 13, // EMFOR_PA
		Sa    = 14, // EMFOR_SA
		Ma    = 15, // EMFOR_MA

		// Resource maximum multiplicative rates.
		HpRate = 16, // EMFOR_HP_RATE
		MpRate = 17, // EMFOR_MP_RATE
		SpRate = 18, // EMFOR_SP_RATE

		// Resistances (all five elements equally).
		Resist = 30, // EMFOR_RESIST

		// Summon time — not in stat pipeline, kept for completeness.
		SummonTime = 31, // EMFOR_SUMMONTIME

		// VERTICAL-007: combat modifiers from passive skills.
		CriticalRate = 32,   // EMFOR_CRITICAL_RATE
		CrushingBlow = 33,   // EMFOR_CRUSHING_BLOW
		DamageReduce = 34,   // EMFOR_DAMAGE_REDUCE
		DamageReflection = 35, // EMFOR_DAMAGE_REFLECTION
		DamageReflectionRate = 36, // EMFOR_DAMAGE_REFLECTION_RATE
	};

	// Passive skill impact (addon) type. Mirrors SKILL::EMIMPACT_ADDON
	// (GLSkillApply.h) for the types that feed the stat pipeline.
	enum class PassiveImpactType : uint8_t
	{
		None           = 0,
		HitRate        = 1,  // EMIMPACTA_HITRATE
		AvoidRate      = 2,  // EMIMPACTA_AVOIDRATE
		Damage         = 3,  // EMIMPACTA_DAMAGE
		Defense        = 4,  // EMIMPACTA_DEFENSE
		VarHp          = 5,  // EMIMPACTA_VARHP
		VarMp          = 6,  // EMIMPACTA_VARMP
		VarSp          = 7,  // EMIMPACTA_VARSP
		VarAp          = 8,  // EMIMPACTA_VARAP
		DamageRate     = 9,  // EMIMPACTA_DAMAGE_RATE
		DefenseRate    = 10, // EMIMPACTA_DEFENSE_RATE
		Pa             = 11, // EMIMPACTA_PA
		Sa             = 12, // EMIMPACTA_SA
		Ma             = 13, // EMIMPACTA_MA
		HpRate         = 14, // EMIMPACTA_HP_RATE
		MpRate         = 15, // EMIMPACTA_MP_RATE
		SpRate         = 16, // EMIMPACTA_SP_RATE
		Resist         = 17, // EMIMPACTA_RESIST

		// VERTICAL-007: combat modifiers from passive skill impacts.
		CriticalRate   = 18, // EMIMPACTA_CRITICAL_RATE
		CrushingBlow   = 19, // EMIMPACTA_CRUSHING_BLOW
		DamageReduce   = 20, // EMIMPACTA_DAMAGE_REDUCE
		DamageReflection = 21, // EMIMPACTA_DAMAGE_REFLECTION
		DamageReflectionRate = 22, // EMIMPACTA_DAMAGE_REFLECTION_RATE
	};

	// Passive skill spec (special) type. Mirrors SKILL::EMSPEC_ADDON
	// (GLSkillApply.h) for the types that feed the stat pipeline.
	// Note: Most specs (pierce, range, velocity, delay, damage reduce)
	// are NOT part of the verified stat pipeline. Only those that
	// contribute to Stats::PassiveContribution are carried.
	enum class PassiveSpecType : uint8_t
	{
		None = 0,
	};

	// One impact entry: a type and its per-level value.
	struct SkillImpact
	{
		PassiveImpactType type = PassiveImpactType::None;
		// Per-level values (index = skill level, 1..kMaxSkillLevel).
		// Index 0 is unused for 1-based skill levels.
		std::array<float, kMaxSkillLevel + 1> values{};

		constexpr bool IsValid() const noexcept
		{
			return type != PassiveImpactType::None;
		}
	};

	// One spec entry: a type and its per-level value.
	struct SkillSpec
	{
		PassiveSpecType type = PassiveSpecType::None;
		std::array<float, kMaxSkillLevel + 1> values{};

		constexpr bool IsValid() const noexcept
		{
			return type != PassiveSpecType::None;
		}
	};

	// Per-level basic apply data. Mirrors SKILL::CDATA_LVL.fBASIC_VAR.
	struct SkillLevelData
	{
		// The basic value for this level. Meaning depends on PassiveApplyType.
		//
		// VERTICAL-011: for an *active* skill the meaning of the same field
		// differs and the sign carries it. Legacy GLChar.cpp:3077-3090:
		//
		//   case SKILL::EMFOR_HP:
		//     if ( sSKILL_DATA.fBASIC_VAR < 0.0f )   -> deal damage
		//     else                                    -> heal
		//
		// A passive ignores the sign and reads the magnitude as a stat delta
		// (GLogixExPC.cpp:921-1000). One field, two readings, selected by role.
		float basicVar = 0.0f;

		// VERTICAL-011: per-level resource costs. Mirrors SKILL::CDATA_LVL
		// (GLSkillApply.h:259-261) `wUSE_SP` / `wUSE_HP` / `wUSE_MP`.
		//
		// Only the cost is carried. The arrow, talisman and bullet counts
		// (wUSE_ARROWNUM, wUSE_CHARMNUM, wUSE_BULLETNUM) need an inventory and
		// are not part of this slice.
		uint16_t useSp = 0;
		uint16_t useHp = 0;
		uint16_t useMp = 0;

		// VERTICAL-011: base cooldown. Mirrors SKILL::CDATA_LVL.fDELAYTIME
		// (GLSkillApply.h:247). The real delay is derived from it together with
		// `grade` and the caster's level; see ActiveSkillResolver.
		float delayTime = 0.0f;

		// VERTICAL-014: the state blow ("ailment") this level applies. Mirrors
		// SKILL::CDATA_LVL::fLIFE (GLSkillApply.h) and the per-level
		// SSTATE_BLOW entry `sAPPLY.sSTATE_BLOW[level]`
		// (GLChar.cpp:3355-3361):
		//
		//   sBLOW.fRATE = sSKILL_BLOW.fRATE;
		//   sBLOW.fLIFE = pSkill->m_sAPPLY.sDATA_LVL[lev].fLIFE;
		//   sBLOW.fVAR1 = sSKILL_BLOW.fVAR1;
		//   sBLOW.fVAR2 = sSKILL_BLOW.fVAR2;
		//
		// The blow TYPE itself is per-skill, not per-level, and lives on
		// SkillDefinition::stateBlow.
		float blowRate = 0.0f;   // fRATE
		float blowVar1 = 0.0f;   // fVAR1 -> fSTATE_VAR1
		float blowVar2 = 0.0f;   // fVAR2 -> fSTATE_VAR2
		float life     = 0.0f;   // fLIFE, in seconds
	};

	// VERTICAL-011: what a skill does. Mirrors SKILL::EMROLE
	// (GLSkillBasic.h:130-135).
	enum class SkillRole : uint8_t
	{
		Normal  = 0,   // EMROLE_NORMAL - castable
		Passive = 1,   // EMROLE_PASSIVE - learned, never cast
	};

	// VERTICAL-011: the damage channel. Mirrors SKILL::EMAPPLY
	// (GLSkillBasic.h:137-144). Only PhysicalMelee is executed by this slice.
	enum class SkillApply : uint8_t
	{
		PhysicalMelee  = 0,   // EMAPPLY_PHY_SHORT
		PhysicalRanged = 1,   // EMAPPLY_PHY_LONG  (VERTICAL-012)
		Magic          = 2,   // EMAPPLY_MAGIC     (VERTICAL-013)
	};

	// VERTICAL-013: the elemental channel of a magic skill. Mirrors EMELEMENT
	// (GLCharDefine.h). Core does not include the legacy enum; this is the
	// modern translation of the same values.
	enum class SkillElement : uint8_t
	{
		Spirit = 0,   // EMELEMENT_SPIRIT - also legacy's default
		Fire   = 1,   // EMELEMENT_FIRE
		Ice    = 2,   // EMELEMENT_ICE
		Electric = 3, // EMELEMENT_ELECTRIC
		Stone  = 4,   // EMELEMENT_STONE
		Mad    = 5,   // EMELEMENT_MAD
		Poison = 6,   // EMELEMENT_POISON
		Curse  = 7,   // EMELEMENT_CURSE
		Zen    = 8,   // EMELEMENT_ZEN

		// EMELEMENT_ARM: "use whatever element the attacker's weapon inflicts".
		// This is a selector, not an element - GLogixExPC.cpp:1505-1513 resolves
		// it to a real element from the right-hand item's blow type before the
		// resistance lookup, falling back to Spirit when there is no weapon.
		ArmWeapon = 9, // EMELEMENT_ARM
	};

	// VERTICAL-011: which entity the skill resolves against. Mirrors
	// SKILL::EMIMPACT_TAR (GLCharDefine.h:859-868).
	enum class SkillTargetKind : uint8_t
	{
		Self      = 0,   // TAR_SELF
		Spec      = 1,   // TAR_SPEC
		SelfToSpec = 2,  // TAR_SELF_TOSPEC - pierce line, needs a world
		Zone      = 3,   // TAR_ZONE       - needs a world
		Specific  = 4,   // TAR_SPECIFIC
	};

	// VERTICAL-011: which side the skill lands on. Mirrors SKILL::EMIMPACT_SIDE
	// (GLCharDefine.h:880-887). Note there is no SIDE_SELF in legacy; the
	// friendly case is SIDE_OUR.
	enum class SkillImpactSide : uint8_t
	{
		Our      = 0,   // SIDE_OUR
		Enemy    = 1,   // SIDE_ENEMY
		Anybody  = 2,   // SIDE_ANYBODY
	};

	// A passive skill definition: the immutable data that determines what
	// the skill contributes when learned at a given level.
	struct SkillDefinition
	{
		SkillId id;
		std::string name;
		uint8_t maxLevel = kMaxSkillLevel;   // dwMAXLEVEL
		uint32_t grade = 0;                   // dwGRADE

		// Weapon requirements for the passive to be active.
		// Matches SSKILLBASIC.emUSE_LITEM / emUSE_RITEM.
		SkillWeaponType leftWeapon  = SkillWeaponType::NoCare;
		SkillWeaponType rightWeapon = SkillWeaponType::NoCare;

		// VERTICAL-011: whether the skill is castable or learned-only.
		//
		// Legacy CHECHSKILL refuses anything that is not EMROLE_NORMAL with
		// EMSKILL_UNKNOWN (GLogixExPC.cpp:4092-4093), so this gates execution
		// rather than being documentation.
		SkillRole role = SkillRole::Normal;

		// VERTICAL-011: the damage channel, the entity the skill resolves
		// against, and the side it lands on. See the enums above.
		SkillApply      apply      = SkillApply::PhysicalMelee;
		SkillTargetKind targetKind = SkillTargetKind::Spec;
		SkillImpactSide impactSide = SkillImpactSide::Enemy;

		// VERTICAL-013: pSkill->m_sAPPLY.emELEMENT. Only read when `apply` is
		// Magic, because legacy only consults it inside the skill path.
		SkillElement element = SkillElement::Spirit;

		// VERTICAL-014: the state blow this skill inflicts. Mirrors
		// `pSkill->m_sAPPLY.emSTATE_BLOW`, read at GLChar.cpp:3357 and guarded
		// on at :3364.
		//
		// It is per-skill rather than per-level in legacy; the per-level numbers
		// are `SkillLevelData::blowRate` / `blowVar1` / `blowVar2` / `life`.
		StatusEffect::StatusEffectType stateBlow = StatusEffect::StatusEffectType::None;

		// The basic apply type and its per-level values.
		PassiveApplyType applyType = PassiveApplyType::Hp;
		std::array<SkillLevelData, kMaxSkillLevel + 1> levelData{};

		// Additional impacts (addons). Matches SIMPACTS.
		std::array<SkillImpact, kMaxSkillImpacts> impacts{};

		// Special specs. Matches SSPECS.
		std::array<SkillSpec, kMaxSkillSpecs> specs{};

		// Validation: a definition is valid if it has a valid id, non-empty name,
		// valid maxLevel, and at least one level has non-zero basicVar or impacts/specs.
		bool IsValid() const noexcept
		{
			if (!id.IsValid() || name.empty() || maxLevel == 0 || maxLevel > kMaxSkillLevel)
			{
				return false;
			}
			// At least one level must contribute something.
			for (uint8_t lvl = 1; lvl <= maxLevel; ++lvl)
			{
				if (levelData[lvl].basicVar != 0.0f)
				{
					return true;
				}
			}
			for (const auto& imp : impacts)
			{
				if (imp.IsValid())
				{
					for (uint8_t lvl = 1; lvl <= maxLevel; ++lvl)
					{
						if (imp.values[lvl] != 0.0f)
						{
							return true;
						}
					}
				}
			}
			return false;
		}

		// Whether this skill requires a specific weapon in the given slot.
		bool RequiresWeapon(SkillWeaponSlot slot) const noexcept
		{
			const SkillWeaponType req = (slot == SkillWeaponSlot::LeftHand) ? leftWeapon : rightWeapon;
			return req != SkillWeaponType::NoCare;
		}

		// Get the required weapon type for a slot.
		SkillWeaponType GetRequiredWeapon(SkillWeaponSlot slot) const noexcept
		{
			return (slot == SkillWeaponSlot::LeftHand) ? leftWeapon : rightWeapon;
		}

		// VERTICAL-011: the per-level record for a cast level.
		//
		// Legacy indexes `m_sAPPLY.sDATA_LVL[wLevel]` raw, with no bounds check
		// (GLogixExPC.cpp:4088, :4292), because the array is a fixed 9 entries
		// and the level was already validated. This returns the level-0 record
		// for a level outside 1..kMaxSkillLevel so a caller that skipped
		// validation reads a zeroed record instead of another skill's level.
		const SkillLevelData& GetLevelData(uint8_t level) const noexcept
		{
			return levelData[(level >= 1 && level <= kMaxSkillLevel) ? level : 0];
		}
	};

	// Convert legacy weapon type to modern enum.
	SkillWeaponType LegacyWeaponTypeToModern(int legacyType);

	// Convert modern weapon type to legacy (for compatibility layer).
	int ModernWeaponTypeToLegacy(SkillWeaponType type);

	// Convert legacy basic type to modern enum.
	PassiveApplyType LegacyBasicTypeToModern(int legacyType);

	// Convert legacy impact type to modern enum.
	PassiveImpactType LegacyImpactTypeToModern(int legacyType);

	// Convert legacy spec type to modern enum.
	PassiveSpecType LegacySpecTypeToModern(int legacyType);

	// Human-readable names for debugging/logging.
	const char* ToString(SkillWeaponType type) noexcept;
	const char* ToString(PassiveApplyType type) noexcept;
	const char* ToString(PassiveImpactType type) noexcept;
	const char* ToString(PassiveSpecType type) noexcept;

	// VERTICAL-011: names for the active-skill enums.
	const char* ToString(SkillRole type) noexcept;
	const char* ToString(SkillApply type) noexcept;
	const char* ToString(SkillTargetKind type) noexcept;
	const char* ToString(SkillImpactSide type) noexcept;
	const char* ToString(SkillElement type) noexcept;
}