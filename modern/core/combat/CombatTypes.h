#pragma once

// VERTICAL-006: basic physical combat domain types.

#include "stats/DerivedStats.h"
#include "stats/Contributions.h"
#include "engine/GameCharacterCalculations.h"

#include <cstdint>

namespace Modern::Combat
{
	// The kind of attack being resolved.
	enum class AttackType : uint8_t
	{
		Melee,   // SKILL::EMAPPLY_PHY_SHORT
		Ranged,  // SKILL::EMAPPLY_PHY_LONG
		Magic,   // SKILL::EMAPPLY_MAGIC  (VERTICAL-013)
	};

	// VERTICAL-012/013: the attack-power contribution to a damage range.
	//
	// `GLDWDATA::VAR_PARAM` (GLDefine.h:364-371) is a saturating add applied to
	// BOTH ends of the range before the random roll, each end independently:
	//
	//   if ( (int(wLow)  + nValue) < 1 )  wLow  = 1;  else wLow  += nValue;
	//   if ( (int(wHigh) + nValue) < 1 )  wHigh = 1;  else wHigh += nValue;
	//
	// The floor at 1 (not 0) matters: it is what keeps the later
	// minimum-damage branch reachable. The comparison is done in `int` so a
	// wrapped unsigned add is still compared as a signed value.
	//
	// This lives here rather than in PhysicalDamageCalculator because magic
	// uses exactly the same operation (GLogixExPC.cpp:1477 for `m_wSUM_MA`,
	// against :1451/:1463/:1584/:1594 for PA and SA). One implementation, four
	// call sites in legacy, one here.
	inline uint32_t ApplyAttackPower(uint32_t rangeEnd, int32_t attackPower) noexcept
	{
		if (static_cast<int32_t>(rangeEnd) + attackPower < 1)
		{
			return 1u;
		}
		return static_cast<uint32_t>(static_cast<int32_t>(rangeEnd) + attackPower);
	}

	// The result of a hit/miss determination.
	struct HitResult
	{
		bool hit = false;                 // true = hit, false = miss
		uint32_t hitRate = 0;             // final hit rate (0-100) after modifiers
		uint32_t hitRoll = 0;             // the roll that was compared (0-99)
	};

	// The result of a damage calculation.
	struct DamageResult
	{
		uint32_t damage = 0;               // final damage applied
		uint32_t rawDamage = 0;            // damage before defense/mitigation
		uint32_t preDefenseDamage = 0;     // damage after resistance, before defense
		bool critical = false;             // critical hit
		bool crushing = false;             // crushing blow
		bool lowSP = false;                // low SP modifier applied

		// VERTICAL-008: reflection result.
		//
		// Legacy: GLogixExPC.cpp:1745-1763, GLogicExNPC.cpp:319-338
		//
		// Reflection is checked AFTER final damage (post-critical, post-reduction).
		// It uses the post-reduction damage value.
		// Reflection cannot recursively trigger (calls ToDamage directly).
		bool reflectionTriggered = false;  // reflection rate check passed
		uint32_t reflectionDamage = 0;     // damage reflected back to attacker
		uint32_t reflectionRoll = 0;       // the roll that was compared (0-99)
	};

	// Complete combat result.
	struct CombatResult
	{
		HitResult hitResult;
		DamageResult damageResult;
		uint32_t targetHPBefore = 0;
		uint32_t targetHPAfter = 0;
		uint32_t damageFlag = 0;           // DAMAGE_TYPE_* flags

		// VERTICAL-008: attacker state after reflection.
		//
		// Legacy: GLChar.cpp:2684-2703 (DamageReflectionProc)
		//
		// Reflection damage is applied to the attacker via ResourceState.
		// The attacker HP values are tracked here for the server to apply.
		uint32_t attackerHPBefore = 0;
		uint32_t attackerHPAfter = 0;

		constexpr bool IsHit() const noexcept { return hitResult.hit; }
		constexpr bool IsMiss() const noexcept { return !hitResult.hit; }
		constexpr bool IsCritical() const noexcept { return damageResult.critical; }
		constexpr bool IsCrushing() const noexcept { return damageResult.crushing; }
		constexpr bool IsReflection() const noexcept { return damageResult.reflectionTriggered; }
	};

// Inputs for hit calculation.
	struct HitInput
	{
		int32_t attackerHit = 0;           // attacker's hit stat
		int32_t targetAvoid = 0;           // target's avoid stat
		Modern::Engine::GameBrightFB brightnessFB = Modern::Engine::GameBrightFB::Aver;
		bool lowSP = false;                // target in low SP state
		float hitRoll = 0.0f;              // [0.0, 1.0] from RNG
	};

	// Inputs for physical damage calculation.
	struct PhysicalDamageInput
	{
		// Attacker stats
		Stats::DamageRange physicalDamage;  // low/high physical damage range
		uint16_t meleePower = 0;            // attacker's melee power
		uint16_t shootPower = 0;            // attacker's shoot power
		AttackType attackType = AttackType::Melee;

		// VERTICAL-019: the aggregated `EMIMPACTA_DAMAGE` contribution.
		//
		// Legacy: `m_gdDAMAGE_SKILL.VAR_PARAM(int(fADDON_VAR))`
		// (GLogixExPC.cpp:2329) - a saturating add to BOTH ends of the damage
		// range, floored at 1, applied BEFORE the weapon's item damage and
		// BEFORE the attack power.
		//
		// It is applied here, at the range, rather than to the rolled figure,
		// because that is what legacy does. One field serves physical, ranged and
		// magic, matching legacy's single `m_gdDAMAGE_SKILL`.
		int32_t factDamage = 0;

		// Target stats
		int32_t defense = 0;                // target's defense
		int32_t defenseBody = 0;            // target's body defense
		int32_t defenseItem = 0;            // target's item defense
		int32_t level = 1;                  // target's level
		float stateDamage = 1.0f;           // target's state damage multiplier
		float damageReduce = 0.0f;          // target's damage reduction
		float damageReflection = 0.0f;      // target's damage reflection
		float damageReflectionRate = 0.0f;  // target's damage reflection rate
		int32_t resistElement = 0;          // target's element resistance
		bool lowSP = false;                 // target in low SP state
		float stateDamageMultiplier = 1.0f; // fSTATE_DAMAGE
		uint16_t requiredSP = 0;            // SP required to perform the attack
		bool isPK = false;                  // PK (player-vs-player) combat

		// Attacker
		uint32_t attackerLevel = 1;         // attacker's level
		uint32_t attackerMaxHP = 0;         // attacker's max HP
		uint32_t attackerCurrentHP = 0;     // attacker's current HP
		int32_t attackerCriticalBonus = 0;  // critical rate bonus from items
		int32_t attackerCrushingBonus = 0;  // crushing blow bonus from items

		// Target
		uint32_t targetLevel = 1;           // target's level
		uint32_t targetMaxHP = 0;           // target's max HP

		// Environmental
		Modern::Engine::GameBrightFB brightnessFB = Modern::Engine::GameBrightFB::Aver;
		float weatherElementPower = 1.0f;   // weather element power
		int32_t targetResistElement = 0;    // target's resistance for the element
		float fDamageReduce = 0.0f;         // target's damage reduction
		float fDamageReflection = 0.0f;     // target's damage reflection
		float fDamageReflectionRate = 0.0f; // target's damage reflection rate

		// Random values
		float hitRoll = 0.0f;               // [0.0, 1.0]
		float damageRoll = 0.0f;            // [0.0, 1.0]
		float criticalRoll = 0.0f;          // [0.0, 1.0]
		float crushingRoll = 0.0f;          // [0.0, 1.0]
		float reflectionRoll = 0.0f;        // [0.0, 1.0]
		float lowSPHitRoll = 0.0f;          // for low SP hit check
		float lowSPDamageRoll = 0.0f;       // for low SP damage check
	};

	// Damage type flags matching legacy DAMAGE_TYPE_*
	enum DamageFlag : uint32_t
	{
		DAMAGE_TYPE_NONE         = 0,
		DAMAGE_TYPE_CRITICAL     = 0x0001,
		DAMAGE_TYPE_CRUSHING_BLOW = 0x0002,
		DAMAGE_TYPE_SHOCK        = 0x0004,
		DAMAGE_TYPE_PSY_REDUCE   = 0x0008,
		DAMAGE_TYPE_PSY_REFLECTION = 0x0010,
		DAMAGE_TYPE_MAGIC_REFLECTION = 0x0020,
	};
}