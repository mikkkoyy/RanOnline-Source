#pragma once

// VERTICAL-014: pure status application resolver.
//
// This resolves ONE question: given a skill or item blow definition and a set
// of injected values, does the blow land, and with what duration? It fetches no
// character state, touches no clock, reads no world, and calls no RNG. Every
// value the legacy rule actually uses is a field on the input, which is what
// makes the whole thing deterministic and testable.
//
// The legacy call sites are:
//
//   GLChar.cpp:3368-3397   skill state blow  (SkillProc)
//   GLChar.cpp:2560-2584   weapon blow on a basic attack
//   GLCrow.cpp:709         monster blow
//
// All three share the same shape, so one resolver serves all of them.

#include "StatusEffectTypes.h"

#include "engine/GameRandom.h"

namespace Modern::StatusEffect
{
	// Values that GLCONST_CHAR supplies to the legacy rule.
	//
	// They are parameters rather than compile-time constants because they are
	// data-file values in legacy (GLogicDataLoad.cpp), and because one of them
	// has to be called out explicitly. See `resistClampCeiling` below.
	struct StatusConstants
	{
		// GLCONST_CHAR::fRESIST_G (GLogicData.cpp:261) = 0.5f.
		//
		// Used as the resistance MULTIPLIER in
		// `fLIFE - (fLIFE*nRESIST/100.0f*fRESIST_G)` (GLChar.cpp:3387).
		float resistGeneralG = 0.5f;

		// VERIFIED LEGACY BUG, reproduced deliberately.
		//
		// GLChar.cpp:3369 (and :2561, :2619, :4027, :4495) all do:
		//
		//   short nBLOWRESIST = pACTOR->GETRESIST().GetElement(...);
		//   if ( nBLOWRESIST > GLCONST_CHAR::fRESIST_G )  nBLOWRESIST = fRESIST_G;
		//
		// The ceiling should be `fMAX_RESIST` (99.0f), which is the constant
		// legacy uses for exactly this purpose a few hundred lines away
		// (GLogixExPC.cpp:1516, `if (nRESIST > fMAX_RESIST) nRESIST = fMAX_RESIST`).
		// Instead it compares against `fRESIST_G`, which is 0.5f. Assigning 0.5f
		// to a `short` truncates it to 0, so every resistance value of 1 or
		// more is clamped to 0 and resistance never reduces the threshold or
		// the duration.
		//
		// This is reproduced rather than corrected: this migration exists to
		// match RAN, and silently "fixing" a clamp would diverge from the client
		// players actually play. The value is named and parameterised so that
		// adopting the intended `fMAX_RESIST` is a one-line data change rather
		// than a hunt through the rules.
		//
		// Set this to 99.0f (kIntendedResistClamp) to get the intended clamp.
		float resistClampCeiling = 0.5f;

		inline static constexpr float kIntendedResistClamp = 99.0f;
	};

	// Why an application was refused. Every refusal is named; none of them is a
	// silent zero.
	enum class StatusRefusal : uint8_t
	{
		None = 0,

		// emBLOW == EMBLOW_NONE, so there is nothing to apply. GLChar.cpp:3364
		// guards on this and skips the whole block.
		NoBlowType,

		// The target already holds this disorder. Legacy skips the probability
		// check entirely in this case (GLChar.cpp:3376-3379):
		//
		//   if ( !(pACTOR->GETHOLDBLOW() & STATE_TO_DISORDER(emTYPE)) )
		//       bBLOW = CHECKSTATEBLOW(...);
		//
		// so the roll is not consumed and the outcome does not depend on it.
		TargetImmune,

		// The threshold was not beaten by the injected roll.
		Probability,
	};

	// Inputs. Every field is used by the rule; there is nothing here the
	// resolver does not need.
	struct StatusApplicationInput
	{
		StatusEffectType type = StatusEffectType::None;

		// SKILL::SSTATE_BLOW::fRATE, already multiplied by the weather power
		// exactly as legacy does at the call site
		// (`sBLOW.fRATE * fPOWER`, GLChar.cpp:3378). Percentage scale.
		float actRate = 0.0f;

		// SKILL::CDATA_LVL::fLIFE (GLChar.cpp:3359). In seconds.
		float lifetime = 0.0f;

		// GLOGICEX::WEATHER_BLOW_POW, applied by the caller for the same reason
		// `actRate` is pre-multiplied: legacy multiplies both at the call site
		// and there is no world in core to read weather from.
		float weatherPower = 1.0f;

		float var1 = 0.0f;   // fVAR1 -> fSTATE_VAR1
		float var2 = 0.0f;   // fVAR2 -> fSTATE_VAR2

		uint32_t attackerLevel = 1;   // wACTLEVEL / GETLEVEL()
		uint32_t targetLevel   = 1;   // wLEVEL    / pACTOR->GetLevel()

		// The target's resistance to this blow's ELEMENT, as it comes out of
		// GETRESIST().GetElement( STATE_TO_ELEMENT(emTYPE) ). The legacy clamp
		// is applied here, not by the caller, because it is part of the rule.
		int32_t targetResist = 0;

		// The target's immunity mask: GETHOLDBLOW() & STATE_TO_DISORDER(type).
		// Supplied as the already-tested mask because building it requires the
		// passive/buff/item spec aggregation (EMSPECA_NONBLOW), which is a
		// separate system.
		uint32_t targetDisorderMask = 0;

		// RANDOM_POS, injected. Never generated here.
		float randomRoll = 1.0f;
	};

	// The state a successful application produces.
	struct StatusEffectState
	{
		StatusEffectType type = StatusEffectType::None;
		float remainingLifetime = 0.0f;   // SSTATEBLOW::fAGE
		float var1 = 0.0f;                 // fSTATE_VAR1
		float var2 = 0.0f;                 // fSTATE_VAR2
	};

	struct StatusApplicationResult
	{
		StatusRefusal refusal = StatusRefusal::NoBlowType;
		StatusEffectState state{};

		// The intermediate values, exposed so a test can assert the formula
		// rather than only its outcome.
		int32_t levelIndex = 0;
		float   levelModifier = 0.0f;
		float   threshold = 0.0f;
		float   effectiveRate = 0.0f;   // actRate with the weather power applied
		float   duration = 0.0f;

		constexpr bool Applied() const noexcept { return refusal == StatusRefusal::None; }
	};

	// Reproduces GLOGICEX::CHECKSTATEBLOW (GameCharacterCalculations.cpp:259-279):
	//
	//   int nDXLEVEL = int(wLEVEL - wACTLEVEL);
	//   int nINDEX   = nDXLEVEL + nStateBlowLevelBase;
	//   if (nINDEX < 0)                  nINDEX = 0;
	//   if (nINDEX >= nStateBlowLevelSize) nINDEX = nStateBlowLevelSize - 1;
	//   float fThreshold = fACTRATE - fACTRATE * 0.01f * wRESIST * 0.6f
	//                      + nStateBlowLevel[nINDEX];
	//   return (RANDOM_POS * 100.0f) < fThreshold;
	//
	// The comparison direction matters and is NOT "improved": the roll is on
	// the left and the strict `<` is what legacy uses.
	inline float ComputeStatusThreshold(float actRate, int32_t resist,
	                                    int32_t attackerLevel, int32_t targetLevel) noexcept
	{
		int32_t index = (targetLevel - attackerLevel) + kStateBlowLevelBase;
		if (index < 0)
		{
			index = 0;
		}
		if (index >= kStateBlowLevelSize)
		{
			index = kStateBlowLevelSize - 1;
		}

		const float threshold =
			actRate - actRate * 0.01f * static_cast<float>(resist) * 0.6f
			        + static_cast<float>(kStateBlowLevel[index]);
		return threshold;
	}

	// The level-table index a pair of levels resolves to. Exposed because the
	// clamp at both ends is itself part of the rule and worth testing.
	inline int32_t ComputeStatusLevelIndex(int32_t attackerLevel, int32_t targetLevel) noexcept
	{
		int32_t index = (targetLevel - attackerLevel) + kStateBlowLevelBase;
		if (index < 0)
		{
			index = 0;
		}
		if (index >= kStateBlowLevelSize)
		{
			index = kStateBlowLevelSize - 1;
		}
		return index;
	}

	// Reproduces the duration calculation at GLChar.cpp:3386-3387:
	//
	//   float fLIFE = sBLOW.fLIFE * fPOWER;
	//   fLIFE = ( fLIFE - (fLIFE*nBLOWRESIST/100.0f*GLCONST_CHAR::fRESIST_G) );
	//   sSTATEBLOW.fAGE = fLIFE;
	inline float ComputeStatusDuration(float lifetime, float weatherPower,
	                                   int32_t resist, const StatusConstants& constants) noexcept
	{
		float life = lifetime * weatherPower;
		life = life - (life * static_cast<float>(resist) / 100.0f * constants.resistGeneralG);
		return life;
	}

	// Applies the legacy resistance ceiling. See `resistClampCeiling`.
	inline int32_t ClampStatusResist(int32_t resist, const StatusConstants& constants) noexcept
	{
		if (static_cast<float>(resist) > constants.resistClampCeiling)
		{
			return static_cast<int32_t>(constants.resistClampCeiling);
		}
		return resist;
	}

	// Resolves one status application. Pure: no clock, no RNG, no character.
	inline StatusApplicationResult ResolveStatusApplication(
		const StatusApplicationInput& input,
		const StatusConstants& constants = StatusConstants())
	{
		StatusApplicationResult result;

		// GLChar.cpp:3364 - `if ( sBLOW.emTYPE != EMBLOW_NONE )`
		if (input.type == StatusEffectType::None)
		{
			result.refusal = StatusRefusal::NoBlowType;
			return result;
		}

		// GLChar.cpp:3376 - the immunity test gates the probability check, so a
		// refused-by-immunity result never looks at the roll.
		const uint32_t disorder = static_cast<uint32_t>(DisorderFor(input.type));
		if (disorder != 0u && (input.targetDisorderMask & disorder) != 0u)
		{
			result.refusal = StatusRefusal::TargetImmune;
			return result;
		}

		const int32_t resist = ClampStatusResist(input.targetResist, constants);

		// `sBLOW.fRATE * fPOWER` at the call site, GLChar.cpp:3378.
		result.effectiveRate = input.actRate * input.weatherPower;

		result.levelIndex = ComputeStatusLevelIndex(
			static_cast<int32_t>(input.attackerLevel),
			static_cast<int32_t>(input.targetLevel));
		result.levelModifier = static_cast<float>(kStateBlowLevel[result.levelIndex]);

		result.threshold = result.effectiveRate
		                 - result.effectiveRate * 0.01f * static_cast<float>(resist) * 0.6f
		                 + result.levelModifier;

		if (!Modern::Engine::CheckProbability(result.threshold, input.randomRoll))
		{
			result.refusal = StatusRefusal::Probability;
			return result;
		}

		result.duration = ComputeStatusDuration(
			input.lifetime, input.weatherPower, resist, constants);

		result.state.type              = input.type;
		result.state.remainingLifetime = result.duration;
		result.state.var1              = input.var1;
		result.state.var2              = input.var2;

		result.refusal = StatusRefusal::None;
		return result;
	}
}