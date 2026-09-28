#pragma once

// VERTICAL-001: the boundary between a character's facts and the stat system.
//
// CORE-001 modelled a character as eight classes and deliberately dropped
// gender, because nothing in the core needed it. CORE-002 kept RAN's real
// class table, which is indexed by the sixteen `EMCHARINDEX` values - class
// *and* gender together. This is where those two meet.
//
// The gap is real and cannot be papered over: `default.charclass` has one row
// per EMCHARINDEX, so a character cannot be looked up without a gender. The
// gender is therefore a character fact after all, and it is introduced here
// rather than inside the stat system, because the stat table needs it and the
// rest of the core does not.
//
// Legacy origin: `legacy/Lib_Client/G-Logic/GLCharDefine.h:235` EMCHARINDEX,
// which pairs every class with a gender across all sixteen values, and
// `GLogicData.h:58` GLCONST_CHARCLASS, whose per-class rows are indexed by it.
// RAN stores the pair as `m_wSex` beside `m_emClass` in SCHARDATA
// (`GLCharData.h:596-597`).

#include "character/Character.h"
#include "stats/BaseStats.h"

#include <cstdint>

namespace Modern
{
	// Which of RAN's paired class entries a character occupies.
	//
	// RAN has no separate notion of gender: `m_wSex` exists only to pick the
	// half of the class table. Modelling it explicitly is the smallest change
	// that keeps the table lookup honest.
	enum class CharacterGender : uint8_t
	{
		Male,
		Female
	};

	const char* ToString(CharacterGender gender) noexcept;

	// The RAN class-table index for a class and gender pair.
	//
	// Returns false for `CharacterClass::Unset`, which has no table row, and
	// the caller is expected to refuse the character rather than guess a row.
	bool TryToCharClassIndex(CharacterClass characterClass, CharacterGender gender,
	                         Stats::CharClassIndex& out) noexcept;

	// The class a RAN class-table index belongs to, for the direction a server
	// needs when it reads a stored index. Returns `CharacterClass::Unset` for an
	// index that names no class.
	CharacterClass ClassOfCharClassIndex(Stats::CharClassIndex index) noexcept;

	CharacterGender GenderOfCharClassIndex(Stats::CharClassIndex index) noexcept;
}
