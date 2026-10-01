#pragma once

// VERTICAL-011: active skill execution.
//
// This is the *active* half of the skill system and it is deliberately a
// different thing from the passive half. VERTICAL-003 reads a definition to
// produce a `Stats::PassiveContribution` when a skill is learned; nothing there
// is ever cast, costs anything, hits anything, or has a cooldown. Pushing
// active execution through `PassiveContributionAggregator` would mean a learned
// passive and a castable skill sharing a pipeline, which is the confusion this
// milestone exists to avoid.
//
// The split follows legacy's own. RAN keeps them apart too: `SUM_PASSIVE`
// (GLogixExPC.cpp:918-1002) is the learn-time aggregation, while casting runs
// `CHECHSKILL` (GLogixExPC.cpp:4056) -> `ACCOUNTSKILL` (GLogixExPC.cpp:4270) ->
// `PreStrikeProc` (GLChar.cpp:2355) -> `SkillProc` (GLChar.cpp:2960). No shared
// function.
//
// What lives here: the rules for resolving one cast of one skill into one
// result. What does not: where the definition came from, whether the skill was
// learned, or which character is on the other end. Those are server decisions,
// and the server owns the authoritative state. This header takes the definition,
// the resolved level, and a description of the situation, and returns a verdict.
//
// Nothing here reads a clock, a socket, a renderer, or a legacy header.

#include "combat/CombatTypes.h"
#include "skills/SkillDefinition.h"
#include "stats/DerivedStats.h"

#include <cstdint>

namespace Modern::Skills
{
	// Why a cast did not happen, and what would be needed for it to.
	//
	// Named after the legacy `EMSKILLCHECK` codes the path actually returns
	// (GLDefine.h:643) where one exists, because a modern caller should be able
	// to read the reason without a translation table. Codes with no legacy
	// counterpart are marked.
	enum class ActiveSkillFailure : uint8_t
	{
		None = 0,

		// GLogixExPC.cpp:4085 - no definition for the id.
		UnknownSkill,

		// GLogixExPC.cpp:4074 - not in the learned set.
		NotLearned,

		// The learned level is 0, or beyond the definition's maxLevel. RAN
		// checks this when learning rather than casting, so there is no
		// EMSKILLCHECK code for it; a corrupt or hand-edited learned set
		// reaches it.
		InvalidLevel,

		// GLogixExPC.cpp:4092 - emROLE is not EMROLE_NORMAL, so the skill is
		// learned-only and must never be cast.
		NotCastable,

		// The apply channel is not physical melee: ranged is VERTICAL-012,
		// magic and elemental are VERTICAL-013. A refusal, not a zero.
		UnsupportedApply,

		// The target mode needs a world the modern server does not have:
		// TAR_ZONE, TAR_SELF_TOSPEC, TAR_SPECIFIC.
		UnsupportedTarget,

		// emIMPACT_SIDE is SIDE_OUR or SIDE_ANYBODY, i.e. the skill helps.
		// Buff and heal application is its own slice.
		UnsupportedSide,

		// fBASIC_VAR >= 0 on an EMFOR_HP skill, which legacy reads as a heal
		// (GLChar.cpp:3087-3090). Not in this slice.
		UnsupportedEffect,

		// fBASIC_VAR < 0 but abs() rounds to zero, so the skill has no
		// magnitude to apply. Legacy would deal 0 and still charge.
		NoDamageMagnitude,

		// GLogixExPC.cpp:4082 - the skill is still on cooldown.
		InCooldown,

		// GLogixExPC.cpp:4240 - `m_sHP.dwNow <= wUSE_HP * wStrikeNum`.
		InsufficientHp,

		// GLogixExPC.cpp:4241 - `m_sMP.dwNow < wUSE_MP * wStrikeNum`.
		InsufficientMp,

		// The damage is non-finite in the definition. Programming error, not a
		// game condition; refused rather than propagated.
		NonFiniteData,
	};

	// Whether the attacker's SP is short of the cost.
	//
	// Distinct from `InsufficientHp`/`InsufficientMp` on purpose. RAN does not
	// refuse a skill for low SP - it casts it in a degraded state
	// (GLCharSkillMsg.cpp:357 tolerates EMSKILL_NOTSP at the running-cast
	// re-check, GLChar.cpp:4797) - so this is a flag on a successful cast, not a
	// failure. Legacy's server-side first check *does* reject
	// EMSKILL_NOTSP (GLCharSkillMsg.cpp:358, the tolerance is commented out),
	// which is a legacy inconsistency; the modern server follows the
	// running-cast behaviour and lets the cast proceed degraded.
	enum class LowSpState : uint8_t
	{
		Normal = 0,
		Low    = 1,
	};

	// The situation a cast is resolved in.
	//
	// Every number the resolver needs is a field here. Nothing is fetched, so a
	// caller cannot accidentally let a rule depend on state the resolver never
	// saw, and a test can drive every branch without building a world.
	struct ActiveSkillInput
	{
		// The definition and the level to cast at. Both are resolved by the
		// server from its own state; a client-supplied level is a hint that the
		// server must overwrite, never a value to trust.
		const SkillDefinition* definition = nullptr;
		uint8_t level = 0;

		// The caster's committed statistics, at the moment of the cast.
		Stats::DerivedStats attacker;

		// The target's committed statistics. Only the fields a skill reads are
		// meaningful; the rest exist so a caller can hand over the whole
		// record rather than hand-picking and getting it subtly wrong.
		Stats::DerivedStats target;
		bool     hasTarget = false;
		uint32_t targetCurrentHp = 0;
		uint16_t targetLevel = 1;   // see `attackerLevel` for why this is separate

		// The caster's character level. `DerivedStats` carries no level - it is
		// a property of the character, not of a stat sum - so the combat level
		// term and the cooldown denominator both come from here.
		uint16_t attackerLevel = 1;

		// The caster's resources, current values. The resolver never mutates
		// them - it returns what should be spent, and the owner applies it
		// through `Resources::ResourceState`. `currentHp` doubles as the
		// attacker's HP for the critical-rate term, which is the caster's own
		// pool in legacy (`GETHP()` in the `CriticalBaseRate` call).
		uint32_t currentHp = 0;
		uint32_t currentMp = 0;
		uint32_t currentSp = 0;

		// The equipment required-SP contribution. This is VERTICAL-010's
		// `Stats::ItemContribution::requiredSP`, passed in rather than
		// recomputed: the sum over the two hand slots is a property of the
		// character's worn set, and recomputing it here would be a second
		// aggregation path.
		uint16_t equipmentRequiredSP = 0;

		// `GLCONST_CHAR::wBASIC_DIS_SP`, from CombatConstants. Named here so a
		// caller does not have to know that a basic skill's cost has a floor
		// unrelated to `wUSE_SP`.
		uint16_t basicAttackSP = 1;

		// Whether the skill is currently on cooldown, and how much of its delay
		// remains. The map and the tick live on the server; Core sees only the
		// question and the answer, so Core has no clock.
		bool onCooldown = false;

		// Injected randomness. Deterministic by construction: a test supplies
		// these and gets the same result every run.
		float hitRoll      = 0.0f;
		float damageRoll   = 0.0f;
		float criticalRoll = 0.0f;
		float crushingRoll = 0.0f;
		float reflectionRoll = 0.0f;

		// `GLOGICEX::WEATHER_ELEMENT_POW` for this skill's element. 1.0f when
		// no weather is active, which is also the no-weather-system default
		// (GameCharacterCalculations.cpp:118-126). The modern server has no map
		// weather, so it passes 1.0f.
		float weatherElementPower = 1.0f;

		// Whether the caster is barred from skills (RAN's `m_bProhibitSkill`).
		bool skillProhibited = false;

		// Whether the caster is stunned. RAN refuses a skill while stunned
		// (GLogixExPC.cpp:4062-4063) and calls it a prohibition.
		bool stunned = false;
	};

	// What a cast resolved to.
	//
	// `combat` is the authoritative combat outcome when the cast reached the
	// damage stage, and is left at its default when it did not. Reading damage
	// out of a failed result is a caller error, and `Succeeded()` is the guard.
	struct ActiveSkillResult
	{
		ActiveSkillFailure failure = ActiveSkillFailure::None;
		LowSpState lowSp = LowSpState::Normal;

		// The level actually cast at, after validation.
		uint8_t level = 0;

		// `wUSE_SP` plus the equipment contribution: the cost the SP pool was
		// measured against. Same arithmetic as VERTICAL-010's basic attack,
		// with the skill's own `wUSE_SP` in place of `wBASIC_DIS_SP`.
		uint16_t requiredSP = 0;

		// What to deduct. Zero when the cast was refused, and SP is zero
		// whenever the cast was low-SP, because legacy does not charge for a
		// cast it already knows the caster cannot afford.
		uint16_t spCost = 0;
		uint16_t hpCost = 0;
		uint16_t mpCost = 0;

		// The cooldown this cast starts, in seconds.
		//
		// `GLOGICEX::SKILLDELAY(dwGRADE, wLevel, wCharLevel, fDELAYTIME)`
		// (GameCharacterCalculations.cpp:118-126), without RAN's
		// `m_fSTATE_DELAY` multiplier and without the 0.3f `NET_MSGDELAY`
		// network compensation, neither of which has a modern counterpart. The
		// server stores and ticks it.
		float cooldownSeconds = 0.0f;

		// The skill's own damage magnitude, before the combat pipeline:
		// `abs(int(fBASIC_VAR * fPOWER))` (GLogixExPC.cpp:1524).
		int32_t basicDamage = 0;

		// The resolved combat outcome, when the cast reached damage.
		Combat::CombatResult combat{};

		constexpr bool Succeeded() const noexcept { return failure == ActiveSkillFailure::None; }
		constexpr bool IsLowSp() const noexcept { return lowSp == LowSpState::Low; }
	};

	// Resolves one cast of one active skill.
	//
	// Free functions rather than a class, because the rule set is small,
	// stateless, and reads better as a sequence in the source. There is no
	// object to construct and no state to carry between calls.
	namespace ActiveSkillResolver
	{
		// The SP cost of a cast: the equipment contribution plus the skill's own
		// `wUSE_SP`.
		//
		// Legacy GLogixExPC.cpp:4254-4256:
		//   WORD wDisSP = sSKILL_DATA.wUSE_SP;
		//   if ( pRHAND )  wDisSP += pRHAND->sSuitOp.wReqSP;
		//   if ( pLHAND )  wDisSP += pLHAND->sSuitOp.wReqSP;
		//
		// VERTICAL-010 already established that the equipment term is the hand
		// sum, so it arrives as a value and is not recomputed.
		uint16_t RequiredSP(const ActiveSkillInput& input, const SkillDefinition& definition) noexcept;

		// The low-SP comparison, kept separate so a caller can reason about it
		// without running the whole resolution. Strict `<`, matching
		// GLogixExPC.cpp:4258.
		LowSpState EvaluateLowSp(uint32_t currentSp, uint16_t requiredSP) noexcept;

		// The cooldown a cast of this skill starts.
		//
		// GLOGICEX::SKILLDELAY, GameCharacterCalculations.cpp:118-126:
		//   return (float)(dwGRADE * wSKILL_LEV) / (float)(wCHAR_LEVEL) + fDelay;
		//
		// The legacy product is computed in integer arithmetic before the cast
		// to float, and that is reproduced here rather than "fixed": a caller
		// comparing against RAN's numbers needs the same overflow behaviour.
		// RAN's `m_fSTATE_DELAY` multiplier and the 0.3f `NET_MSGDELAY`
		// network compensation are not applied; neither has a modern
		// counterpart, and both are transport or debuff adjustments.
		float CooldownSeconds(const SkillDefinition& definition, uint8_t level,
		                     uint16_t attackerLevel) noexcept;

		// Resolves the whole cast.
		ActiveSkillResult Resolve(const ActiveSkillInput& input) noexcept;
	}
}
