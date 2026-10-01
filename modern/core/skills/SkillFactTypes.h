#pragma once

// VERTICAL-015: skill FACT ("buff") domain types.
//
// FACT is the OTHER persistent-effect mechanism in RAN and is not the same thing
// as a status effect. VERTICAL-014 models `EMSTATE_BLOW` - short-lived ailments
// in four shared slots. FACT models `SSKILLFACT` - timed skill enhancements in
// fourteen slots, keyed by skill, holding impacts and specs.
//
// The two stay in separate domains on purpose. They have different storage,
// different lifetimes, different slot rules and different consumers, and merging
// them would obscure exactly the differences that matter.
//
// Legacy references below; core includes none of them.

#include "skills/SkillDefinition.h"   // PassiveApplyType (== SKILL::EMTYPES)
#include "types/Ids.h"

#include <array>
#include <cstdint>

namespace Modern::Skills
{
	// SKILLREALFACT_SIZE and SKILLFACT_SIZE are both 14 (GLCharData.h:200-201).
	// The pool size is a shipped constant and is not "corrected" here.
	inline constexpr uint8_t kSkillFactSlotCount = 14;

	// SKILL::MAX_IMPACT and SKILL::MAX_SPEC are both 5 (GLSkillDefine.h:17-18).
	inline constexpr uint8_t kSkillFactMaxImpacts = 5;
	inline constexpr uint8_t kSkillFactMaxSpecs   = 5;

	// The impact and spec type enums, and the per-level entry structs, live in\n	// SkillDefinition.h: a SkillDefinition has to carry them, and this header\n	// depends on that one, so declaring them here would be a circular include.\n\n	// SSKILLFACT_IMPACTS (GLFactData.h:20-25).
	struct SkillFactImpact
	{
		SkillFactImpactType type = SkillFactImpactType::None;
		float value = 0.0f;      // fADDON_VAR
	};

	// SSKILLFACT_SPECS (GLFactData.h:27-36).
	struct SkillFactSpec
	{
		SkillFactSpecType type = SkillFactSpecType::None;
		float    var1 = 0.0f;    // fSPECVAR1
		float    var2 = 0.0f;    // fSPECVAR2
		uint32_t specFlag = 0;   // dwSPECFLAG
		uint32_t nativeId = 0;   // dwNativeID
	};

	// SSKILLFACT (GLFactData.h:50-74).
	//
	// `remainingLifetime` is legacy's `fAGE`, which despite the name holds the
	// REMAINING lifetime: it is assigned `sSKILL_DATA.fLIFE` at creation
	// (GLChar.cpp:6609) and decremented every tick (GLogixExPC.cpp:2292).
	struct SkillFact
	{
		// Defaults to the null id, matching legacy's `sNATIVEID(NATIVEID(false))`
		// constructor (GLFactData.h:68). This is what makes an untouched slot
		// report `Occupied() == false`.
		SkillId       skillId = SkillId::Invalid();
		uint16_t      level = 0;                    // wLEVEL
		float         remainingLifetime = 0.0f;      // fAGE
		PassiveApplyType basicType = PassiveApplyType::Hp;  // emTYPE
		float         basicValue = 0.0f;             // fMVAR
		uint32_t      specialSkill = 0;              // dwSpecialSkill
		std::array<SkillFactImpact, kSkillFactMaxImpacts> impacts{};
		std::array<SkillFactSpec,   kSkillFactMaxSpecs>   specs{};

		uint16_t casterCrow = 0;   // _wCasterCrow
		uint32_t casterId   = 0;   // _dwCasterID

		// A slot is occupied when it holds a skill. Legacy's emptiness test is
		// `sNATIVEID == SNATIVEID(false)` (GLChar.cpp:6395), so the skill id is
		// what marks occupancy - not the lifetime.
		bool Occupied() const noexcept
		{
			return skillId.IsValid();
		}

		bool HasSpec(SkillFactSpecType type) const noexcept
		{
			for (const SkillFactSpec& spec : specs)
			{
				if (spec.type == type)
				{
					return true;
				}
			}
			return false;
		}

		const SkillFactSpec* FindSpec(SkillFactSpecType type) const noexcept
		{
			for (const SkillFactSpec& spec : specs)
			{
				if (spec.type == type)
				{
					return &spec;
				}
			}
			return nullptr;
		}

		bool HasImpact(SkillFactImpactType type) const noexcept
		{
			for (const SkillFactImpact& impact : impacts)
			{
				if (impact.type == type)
				{
					return true;
				}
			}
			return false;
		}

		const SkillFactImpact* FindImpact(SkillFactImpactType type) const noexcept
		{
			for (const SkillFactImpact& impact : impacts)
			{
				if (impact.type == type)
				{
					return &impact;
				}
			}
			return nullptr;
		}
	};

	// The EMFOR_* types legacy actually copies into a FACT's emTYPE/fMVAR.
	// GLChar.cpp:6519-6539 switches over exactly this set; anything else leaves
	// the pair at its constructor default and sets no `bHOLD`.
	constexpr bool IsFactBasicType(PassiveApplyType type) noexcept
	{
		switch (type)
		{
			case PassiveApplyType::VarHp:
			case PassiveApplyType::VarMp:
			case PassiveApplyType::VarSp:
			case PassiveApplyType::VarAp:
			case PassiveApplyType::Defense:
			case PassiveApplyType::HitRate:
			case PassiveApplyType::AvoidRate:
			case PassiveApplyType::VarDamage:
			case PassiveApplyType::VarDefense:
			case PassiveApplyType::Pa:
			case PassiveApplyType::Sa:
			case PassiveApplyType::Ma:
			case PassiveApplyType::Resist:
			case PassiveApplyType::SummonTime:
				return true;
			case PassiveApplyType::Hp:
			case PassiveApplyType::Mp:
			case PassiveApplyType::Sp:
			case PassiveApplyType::HpRate:
			case PassiveApplyType::MpRate:
			case PassiveApplyType::SpRate:
				break;
		}
		return false;
	}

	// Does this FACT do anything? Legacy's `bHOLD` gate (GLChar.cpp:6605):
	// without a whitelisted basic type, an impact or a spec there is nothing to
	// store, and `RECEIVE_SKILLFACT` refuses rather than storing an inert record.
	constexpr bool FactHoldsAnything(const SkillFact& fact) noexcept
	{
		if (IsFactBasicType(fact.basicType))
		{
			return true;
		}
		for (const SkillFactImpact& impact : fact.impacts)
		{
			if (impact.type != SkillFactImpactType::None)
			{
				return true;
			}
		}
		for (const SkillFactSpec& spec : fact.specs)
		{
			if (spec.type != SkillFactSpecType::None)
			{
				return true;
			}
		}
		return false;
	}
}