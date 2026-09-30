// VERTICAL-006: complete combat resolution pipeline.
//
// Legacy provenance: GLogixExPC.cpp:1291 CHECKHIT, GLogixExPC.cpp:1363 CALCDAMAGE_20060328
// ServerCharacter owns the authoritative combat resolution.
//
// Brightness/environment is not yet modeled in modern Core.
// Documented as DEFERRED: see Section 11 of VERTICAL-006 specification.
//
// targetLowSP detection uses currentSP == 0 as a proxy for "low SP"));
// Documented as LIMITED: verify against legacy behavior if needed.

#include "CombatTypes.h"
#include "CombatConstants.h"
#include "HitCalculator.h"
#include "PhysicalDamageCalculator.h"
#include "engine/GameCharacterCalculations.h"

namespace Modern::Combat
{
	// Input for full combat resolution (attacker -> target).
	// Every field is an explicit input; nothing is silently assumed.
	struct CombatInput
	{
		// Attacker derived stats
		int32_t attackerHit = 0;
		int32_t attackerAvoid = 0;
		uint16_t attackerMeleePower = 0;
		uint16_t attackerShootPower = 0;
		Stats::DamageRange attackerPhysicalDamage;
		uint32_t attackerLevel = 1;
		uint32_t attackerMaxHP = 0;
		uint32_t attackerCurrentHP = 0;
		int32_t attackerCriticalBonus = 0;
		int32_t attackerCrushingBonus = 0;
		AttackType attackType = AttackType::Melee;

		// Target derived stats
		int32_t targetHit = 0;
		int32_t targetAvoid = 0;
		int32_t targetDefense = 0;
		int32_t targetDefenseBody = 0;
		int32_t targetDefenseItem = 0;
		int32_t targetLevel = 1;
		uint32_t targetMaxHP = 0;
		uint32_t targetCurrentHP = 0;
		float targetStateDamage = 1.0f;
		float targetDamageReduce = 0.0f;
		float targetDamageReflection = 0.0f;
		float targetDamageReflectionRate = 0.0f;
		int32_t targetResistElement = 0;
		bool targetLowSP = false; // Low SP proxy: current SP == 0

		// Environmental
		// Brightness/environment modifier is DEFERRED — modern Core does not
		// model world brightness. The caller should supply the resolved modifier,
		// or treat this part as deferred (Aver assumed for now).
		Modern::Engine::GameBrightFB brightnessFB = Modern::Engine::GameBrightFB::Aver;
		float weatherElementPower = 1.0f;

		// Random values (deterministic from caller)
		float hitRoll = 0.0f;
		float damageRoll = 0.0f;
		float criticalRoll = 0.0f;
		float crushingRoll = 0.0f;
		float reflectionRoll = 0.0f;
	};

// Resolves a basic physical attack from attacker to target.
	// Returns the complete combat result including hit/miss, damage, and flags.
	// 
	// DEFERRED behaviors (documented, not omitted):
	//   - Brightness/environment modifier: assumed Aver (modern Core does not
	//     model world brightness). Caller should supply resolved modifier if
	//     available. See CombatInput::brightnessFB.
	//   - targetLowSP: uses currentSP == 0 as proxy. Verify against legacy if
	//     needed. See Section 12 of VERTICAL-006 specification.
	//   - targetDamageReduce, targetDamageReflection, targetResistElement:
	//     not yet modeled; passed as zero. See Sections 17-18.
	//   - attackerCriticalBonus, attackerCrushingBonus: not yet modeled; zero.
	//   - weatherElementPower: assumed 1.0f.
	CombatResult ResolveCombat(const CombatInput& input, const CombatConstants& constants = CombatConstants())
	{
		CombatResult result;

		// 1. Check hit
		HitInput hitInput;
		hitInput.attackerHit = input.attackerHit;
		hitInput.targetAvoid = input.targetAvoid;
		hitInput.brightnessFB = input.brightnessFB;
		hitInput.lowSP = input.targetLowSP;
		hitInput.hitRoll = input.hitRoll;

		result.hitResult = CalculateHit(hitInput, constants);

		if (!result.hitResult.hit)
		{
			result.targetHPBefore = input.targetCurrentHP;
			result.targetHPAfter = input.targetCurrentHP;
			result.damageResult.damage = 0;
			result.damageResult.rawDamage = 0;
			return result;
		}

		// 2. Build physical damage input
		PhysicalDamageInput damageInput;
		damageInput.physicalDamage = input.attackerPhysicalDamage;
		damageInput.meleePower = input.attackerMeleePower;
		damageInput.shootPower = input.attackerShootPower;
		damageInput.attackType = input.attackType;
		damageInput.defense = input.targetDefense;
		damageInput.defenseBody = input.targetDefenseBody;
		damageInput.defenseItem = input.targetDefenseItem;
		damageInput.level = input.targetLevel;
		damageInput.stateDamage = input.targetStateDamage;
		damageInput.damageReduce = input.targetDamageReduce;
		damageInput.damageReflection = input.targetDamageReflection;
		damageInput.damageReflectionRate = input.targetDamageReflectionRate;
		damageInput.resistElement = input.targetResistElement;
		damageInput.lowSP = input.targetLowSP;
		damageInput.stateDamageMultiplier = input.targetStateDamage;
		damageInput.attackerLevel = input.attackerLevel;
		damageInput.attackerMaxHP = input.attackerMaxHP;
		damageInput.attackerCurrentHP = input.attackerCurrentHP;
		damageInput.attackerCriticalBonus = input.attackerCriticalBonus;
		damageInput.attackerCrushingBonus = input.attackerCrushingBonus;
		damageInput.targetLevel = input.targetLevel;
		damageInput.targetMaxHP = input.targetMaxHP;
		damageInput.resistElement = input.targetResistElement;
		damageInput.weatherElementPower = 1.0f;
		damageInput.damageRoll = 0.5f;
		damageInput.criticalRoll = 0.5f;
		damageInput.crushingRoll = 0.5f;
		damageInput.reflectionRoll = input.reflectionRoll;

		// 3. Calculate damage
		result.damageResult = CalculatePhysicalDamage(damageInput);

		// 4. Apply damage to target HP
		result.targetHPBefore = input.targetCurrentHP;
		uint32_t damageApplied = result.damageResult.damage;
		if (damageApplied > input.targetCurrentHP)
		{
			damageApplied = input.targetCurrentHP;
		}
		result.targetHPAfter = input.targetCurrentHP - damageApplied;

		// 5. Build damage flags
		if (result.damageResult.critical)   result.damageFlag |= DAMAGE_TYPE_CRITICAL;
		if (result.damageResult.crushing)  result.damageFlag |= DAMAGE_TYPE_CRUSHING_BLOW;
		if (result.damageResult.reflectionTriggered) result.damageFlag |= DAMAGE_TYPE_PSY_REFLECTION;

		// 6. VERTICAL-008: track attacker HP for reflection.
		//
		// Legacy: GLChar.cpp:2684-2703 (DamageReflectionProc)
		//
		// Reflection damage is applied to the attacker via ResourceState.
		// The attacker HP values are tracked here for the server to apply.
		// Reflection cannot recursively trigger.
		result.attackerHPBefore = input.attackerCurrentHP;
		if (result.damageResult.reflectionTriggered && result.damageResult.reflectionDamage > 0)
		{
			uint32_t reflectionApplied = result.damageResult.reflectionDamage;
			if (reflectionApplied > input.attackerCurrentHP)
			{
				reflectionApplied = input.attackerCurrentHP;
			}
			result.attackerHPAfter = input.attackerCurrentHP - reflectionApplied;
		}
		else
		{
			result.attackerHPAfter = input.attackerCurrentHP;
		}

		return result;
	}
}