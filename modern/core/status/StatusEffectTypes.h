#pragma once

// VERTICAL-014: status effect ("state blow") domain types.
//
// Modern translations of the legacy status vocabulary. Core must not include
// GLCharDefine.h, so `EMSTATE_BLOW`, `EMDISORDER` and the `EMBLOW_MULTI` slot
// count are reproduced here as modern types. The mapping is documented on every
// declaration because these names are the contract between this file and the
// legacy tree.

#include <cstdint>

namespace Modern::StatusEffect
{
	// Mirrors SKILL::EMSTATE_BLOW (GLCharDefine.h:907-923).
	//
	// The numeric values are the legacy values, because they are load-bearing:
	// the slot mapping subtracts from `Frozen`, so renumbering would silently
	// change which state lands in which slot.
	enum class StatusEffectType : uint8_t
	{
		None   = 0,   // EMBLOW_NONE
		Numb   = 1,   // EMBLOW_NUMB      - ����
		Stun   = 2,   // EMBLOW_STUN      - ����
		Stone  = 3,   // EMBLOW_STONE     - ��ȭ
		Burn   = 4,   // EMBLOW_BURN      - ȭ��
		Frozen = 5,   // EMBLOW_FROZEN    - �õ�
		Mad    = 6,   // EMBLOW_MAD       - ����
		Poison = 7,   // EMBLOW_POISON    - �ߵ�
		Curse  = 8,   // EMBLOW_CURSE     - ����
	};

	// Mirrors EMDISORDER (GLCharDefine.h:894-903). A bitmask, not an enum of
	// states: `STATE_TO_DISORDER` maps one state to one bit, and both the
	// immunity test and the cure test use it as a mask.
	enum StatusDisorder : uint32_t
	{
		DisorderNone   = 0x00,
		DisorderNumb   = 0x01,
		DisorderStun   = 0x02,
		DisorderStone  = 0x04,
		DisorderBurn   = 0x08,
		DisorderFrozen = 0x10,
		DisorderMad    = 0x20,
		DisorderPoison = 0x40,
		DisorderCurse  = 0x80,
		DisorderAll    = 0xFF,
	};

	// EMBLOW_MULTI = 4 (GLCharDefine.h:922): the number of state slots a
	// character has. `SSTATEBLOW m_sSTATEBLOWS[EMBLOW_MULTI]`
	// (GLCharClient.h:106).
	inline constexpr uint8_t kStatusSlotCount = 4;

	// EMBLOW_SINGLE = 5 (GLCharDefine.h:915) - an ALIAS of EMBLOW_FROZEN, not a
	// separate state. It is the pivot of the slot mapping:
	//
	//   GLChar.cpp:6204-6205
	//   if ( sStateBlow.emBLOW <= EMBLOW_SINGLE )  nIndex = 0;
	//   else                                       nIndex = emBLOW - EMBLOW_SINGLE;
	//
	// So Numb, Stun, Stone, Burn and Frozen ALL share slot 0, and applying any
	// of them overwrites whichever of the others was there. Mad, Poison and
	// Curse get slots 1, 2 and 3.
	inline constexpr uint8_t kSingleSlotPivot = 5;

	// The slot a state occupies. Reproduces GLChar.cpp:6204-6205 exactly.
	constexpr uint8_t SlotFor(StatusEffectType type) noexcept
	{
		const uint8_t raw = static_cast<uint8_t>(type);
		if (raw == static_cast<uint8_t>(StatusEffectType::None))
		{
			return kStatusSlotCount;   // never occupies a slot
		}
		if (raw <= kSingleSlotPivot)
		{
			return 0;
		}
		return static_cast<uint8_t>(raw - kSingleSlotPivot);
	}

	// Mirrors STATE_TO_DISORDER (GLCharDefine.h:925-940).
	constexpr StatusDisorder DisorderFor(StatusEffectType type) noexcept
	{
		switch (type)
		{
			case StatusEffectType::Numb:   return DisorderNumb;
			case StatusEffectType::Stun:   return DisorderStun;
			case StatusEffectType::Stone:  return DisorderStone;
			case StatusEffectType::Burn:   return DisorderBurn;
			case StatusEffectType::Frozen: return DisorderFrozen;
			case StatusEffectType::Mad:    return DisorderMad;
			case StatusEffectType::Poison: return DisorderPoison;
			case StatusEffectType::Curse:  return DisorderCurse;
			case StatusEffectType::None:   break;
		}
		return DisorderNone;
	}

	// Mirrors STATE_TO_ELEMENT (GLCharDefine.h:942-957). Needed because the
	// resistance looked up for a blow is the resistance of the blow's ELEMENT,
	// not of the blow itself: GLChar.cpp:3368 does
	// `GETRESIST().GetElement( STATE_TO_ELEMENT(sBLOW.emTYPE) )`.
	//
	// These are the legacy `EMELEMENT` values, translated. Core does not
	// include EMELEMENT.
	enum class BlowElement : uint8_t
	{
		Spirit   = 0,   // EMELEMENT_SPIRIT  (also the default return)
		Fire     = 1,   // EMELEMENT_FIRE
		Ice      = 2,   // EMELEMENT_ICE
		Electric = 3,   // EMELEMENT_ELECTRIC
		Stone    = 4,   // EMELEMENT_STONE
		Stun     = 5,   // EMELEMENT_STUN
		Mad      = 6,   // EMELEMENT_MAD
		Poison   = 7,   // EMELEMENT_POISON
		Curse    = 8,   // EMELEMENT_CURSE
	};

	// Mirrors STATE_TO_ELEMENT (GLCharDefine.h:942-957).
	constexpr BlowElement ElementFor(StatusEffectType type) noexcept
	{
		switch (type)
		{
			case StatusEffectType::Numb:   return BlowElement::Electric;
			case StatusEffectType::Stun:   return BlowElement::Stun;
			case StatusEffectType::Stone:  return BlowElement::Stone;
			case StatusEffectType::Burn:   return BlowElement::Fire;
			case StatusEffectType::Frozen: return BlowElement::Ice;
			case StatusEffectType::Mad:    return BlowElement::Mad;
			case StatusEffectType::Poison: return BlowElement::Poison;
			case StatusEffectType::Curse:  return BlowElement::Curse;
			case StatusEffectType::None:   break;
		}
		return BlowElement::Spirit;
	}

	// GLCONST_CHAR::nSTATEBLOW_LEVEL (GLogicData.cpp:309).
	//
	// Data, not code: it is loaded from the data file at GLogicDataLoad.cpp:236,
	// so the default here is the shipped default and not a hardcoded rule. The
	// values are added to the application threshold as raw percentage points.
	inline constexpr int kStateBlowLevel[10] =
	{
		+10, +8, +6, +3, 0, -2, -4, -6, -8, -10
	};

	// GLogicData.h:508. BASE is added to the level difference before indexing;
	// SIZE bounds the table.
	inline constexpr int kStateBlowLevelBase = 1;
	inline constexpr int kStateBlowLevelSize = 10;

	constexpr bool IsValidStatusType(StatusEffectType type) noexcept
	{
		return static_cast<uint8_t>(type) <= static_cast<uint8_t>(StatusEffectType::Curse);
	}
}