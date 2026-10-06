#pragma once

// WORLD-ENTRY-002f: base movement speed, resolved from a character's class pair.
//
// This is the IMPLEMENTATION of `IMovementSpeedProvider`, the seam
// MovementStateService.h (WORLD-ENTRY-002a) declared and deliberately left
// unimplemented because `default.charclass` was not available then. WORLD-ENTRY-002c
// decrypted it, so the seam can now be filled.
//
// It contributes the BASE TERM ONLY. `MovementSpeed.h` carries the formula and the
// full list of what is deferred; this file's job is narrower: turn a `WorldCharacter`
// into a `CharClassIndex` and ask the table.
//
// ---------------------------------------------------------------------------
// WHY CLASS ALONE IS NOT ENOUGH, AND WHY THAT IS THE ONLY REASON GENDER EXISTS
// ---------------------------------------------------------------------------
//
// RAN's speed table is indexed by `EMCHARINDEX`, which has SIXTEEN entries - eight
// classes times two genders (GLCharDefine.cpp:91-118). `WorldCharacter::characterClass`
// is eight wide. `CharClassToIndex` therefore cannot be evaluated without the gender,
// which is why WORLD-ENTRY-002f added `WorldCharacter::characterGender`. The
// resolved index is not transmitted and not stored: it is recomputed from the pair
// every time a speed is needed, so there is exactly one answer to "which row".
//
// A class/gender pair with no table row - `CharacterClass::Unset`, or a raw class
// value RAN does not define - yields NO speed. It does not yield a default, because a
// default would be a number RAN never produces. The caller decides what an
// unwalkable character means; see `TryMaxSpeed`.
//
// ---------------------------------------------------------------------------
// No EQUIPMENT, NO VEHICLE, NO DISGUISE, NO PASSIVE SKILL
// ---------------------------------------------------------------------------
//
// Each is a named deferral from 002c and each would change the number. Stated as
// behaviour rather than as an apology: a RAN character wearing a speed item moves
// faster than this one, because the item term of `MoveVelocity` is not supplied.
// The formula is exact; the input is absent.

#include "character/CharacterClassTable.h"
#include "movement/MovementSpeed.h"
#include "world/MovementStateService.h"
#include "world/WorldCharacter.h"

namespace Modern::Server::World
{
	class CharacterClassMovementSpeed final : public IMovementSpeedProvider
	{
	public:
		// The seam 002a declared.
		//
		// Returns 0.0f - never a fallback, never the constructor default - when the
		// character's class/gender pair has no table row. A caller that cannot tell 0
		// from a real speed would treat "no speed" as "very slow", which is a different
		// bug; `TryMaxSpeed` exists so it does not have to.
		float MaxSpeedFor(const WorldCharacter& character) const override;

		// The same answer, but reporting whether there was one.
		bool TryMaxSpeed(const WorldCharacter& character, float& out) const noexcept;

		// The class index RAN would use, or false when the pair is not in the table.
		//
		// Exposed because "which row did this character resolve to" is a question an
		// operator asks, and a speed provider that answers it only as a float makes the
		// question unanswerable.
		bool TryResolveClassIndex(const WorldCharacter& character,
		                          Stats::CharClassIndex& out) const noexcept;

		// The base velocity for a resolved index, before any multiplier.
		static bool TryBaseVelocity(Stats::CharClassIndex index, bool running,
		                            float& out) noexcept
		{
			return Movement::TryGetBaseVelocity(index, running, out);
		}
	};
}
