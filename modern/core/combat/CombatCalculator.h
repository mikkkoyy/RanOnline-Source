#pragma once

// VERTICAL-006: complete combat resolution pipeline.
//
// Legacy provenance: GLogixExPC.cpp:1291 CHECKHIT, GLogixExPC.cpp:1363 CALCDAMAGE_20060328
// ServerCharacter owns the authoritative combat resolution.
//
// Brightness/environment is not yet modeled in modern Core.
// Documented as DEFERRED: see Section 11 of VERTICAL-006 specification.
//
// VERTICAL-009: low SP became the legacy comparison
// `currentSP < requiredSP` (GLogixExPC.cpp:3497), replacing the `currentSP == 0`
// proxy VERTICAL-006 used. VERTICAL-009 still fed it a bare constant because the
// required-SP value had nowhere to live.
//
// VERTICAL-010: the equipment term is modelled and the value arrives in
// CombatInput::attackerRequiredSP. The comparison is evaluated here from
// `attackerCurrentSP < attackerRequiredSP` rather than arriving as a flag, and
// the pool is the attacker's own — the legacy gate is evaluated on the
// character swinging (GLCharMsg.cpp:604-612, BEGIN_ATTACK), not on the target.

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

		// VERTICAL-010: the SP cost of the attack, and the attacker's current
		// pool. The low-SP rule is evaluated in Core from these two, not
		// supplied as a flag, so the rule lives in exactly one place.
		//
		// `attackerRequiredSP` is VERTICAL-009's field and keeps its name; it
		// now receives the real value, equipment contribution included. See
		// ServerCharacter::Attack.
		//
		// The pool is the *attacker's*. Legacy decides low SP before the swing,
		// on the character doing the attacking: GLCharMsg.cpp:604-612 calls
		// BEGIN_ATTACK (GLogixExPC.cpp:3492-3497), which compares that
		// character's own m_sSP.dwNow, derives bLowSP from the result, and
		// passes it to that character's PreStrikeProc. It is a property of the
		// attacker, not of whoever is on the receiving end. VERTICAL-009 fed
		// this from the target's SP, which could not produce the documented
		// behaviour for any real required-SP value.
		uint16_t attackerRequiredSP = 0; // SP required to perform the attack
		uint32_t attackerCurrentSP = 0;  // attacker's current SP

		bool isPK = false; // PK (player-vs-player) combat

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
	//   - isPK: the modifier is applied when true, but PK state detection
	//     (safe zones, parties, PK maps) is server/world state. VERTICAL-009.
	//   - targetStateDamage: 1.0f unless the caller supplies it.
	//   - attackerCriticalBonus, attackerCrushingBonus: supplied by the server
	//     from equipment/passive contributions.
	//   - weatherElementPower: assumed 1.0f.
	//   - attackerRequiredSP excludes m_wACCEPTP, as the legacy gate does.
	//     VERTICAL-010.
	inline CombatResult ResolveCombat(const CombatInput& input, const CombatConstants& constants = CombatConstants())
	{
		CombatResult result;

		// VERTICAL-010: the low-SP rule, in the one place it is written.
		//
		// Legacy: GLogixExPC.cpp:3492-3497
		//     WORD wDisSP = GLCONST_CHAR::wBASIC_DIS_SP;
		//     if ( pRHAND )  wDisSP += pRHAND->sSuitOp.wReqSP;
		//     if ( pLHAND )  wDisSP += pLHAND->sSuitOp.wReqSP;
		//     if ( m_sSP.dwNow < (wDisSP*wStrikeNum) )  return EMBEGINA_SP;
		//
		// Strict `<`: a character holding exactly the required amount is not
		// low-SP. wStrikeNum is 1 because modern combat resolves a single
		// strike; a combo count belongs with the attack-sequence system.
		//
		// Note this gate does not include m_wACCEPTP, which is why that term
		// is not part of attackerRequiredSP either.
		const bool lowSP = input.attackerCurrentSP < static_cast<uint32_t>(input.attackerRequiredSP);

		// 1. Check hit
		HitInput hitInput;
		hitInput.attackerHit = input.attackerHit;
		hitInput.targetAvoid = input.targetAvoid;
		hitInput.brightnessFB = input.brightnessFB;
		hitInput.lowSP = lowSP;
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
		damageInput.lowSP = lowSP;
		damageInput.stateDamageMultiplier = input.targetStateDamage;
		damageInput.requiredSP = input.attackerRequiredSP;
		damageInput.isPK = input.isPK;
		damageInput.attackerLevel = input.attackerLevel;
		damageInput.attackerMaxHP = input.attackerMaxHP;
		damageInput.attackerCurrentHP = input.attackerCurrentHP;
		damageInput.attackerCriticalBonus = input.attackerCriticalBonus;
		damageInput.attackerCrushingBonus = input.attackerCrushingBonus;
		damageInput.targetLevel = input.targetLevel;
		damageInput.targetMaxHP = input.targetMaxHP;
		damageInput.resistElement = input.targetResistElement;
		damageInput.weatherElementPower = 1.0f;
		damageInput.damageRoll = input.damageRoll;
		damageInput.criticalRoll = input.criticalRoll;
		damageInput.crushingRoll = input.crushingRoll;
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