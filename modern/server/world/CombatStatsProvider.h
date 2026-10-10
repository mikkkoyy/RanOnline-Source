#pragma once

// WORLD-ENTRY-002L-A: the seam between recovered class data and live combat.
//
// ---------------------------------------------------------------------------
// WHAT THIS IS FOR
// ---------------------------------------------------------------------------
//
// 002K's `DamageResolution` takes its combat statistics from the caller, and
// `FieldRoleRuntime` currently fills them with the explicitly named prototype
// constants in DamageResolution.h:65-86 (hit/avoid 30/30, damage 10-20). Those
// exist because the derived-stat pipeline had no data source. This file is the
// seam that closes that gap WITHOUT letting a stat become authoritative in the
// wrong place:
//
//   * `Stats::Calculate` (modern/core/stats/StatCalculator.h) remains the only
//     place RAN's stat arithmetic lives. Nothing here restates a formula.
//   * `Stats::ClassConstantTable` (WORLD-ENTRY-002L-A) owns the recovered
//     coefficients and their provenance.
//   * THIS class turns a `WorldCharacter` into a `Stats::DerivedStats`, or
//     says it cannot.
//
// ---------------------------------------------------------------------------
// WHY IT IS AN INTERFACE
// ---------------------------------------------------------------------------
//
// The interface exists for the same reason `IMovementSpeedProvider`
// (MovementStateService.h:89) does: the runtime must not depend on where the
// numbers come from, and a test must be able to supply a row without touching
// the shipped table. `FieldRoleRuntime` holds a pointer, defaults to null, and
// falls back to its prototype constants when there is no provider or the
// provider refuses - so installing this cannot change behaviour on its own.
//
// ---------------------------------------------------------------------------
// WHAT IT DOES NOT DO
// ---------------------------------------------------------------------------
//
// It does NOT read HP, and it does not write anything. HP stays with
// `ResourceSyncService`, which remains the only HP owner in this milestone.
// The provider reads a character's class, gender and level - all of which the
// character record already carries - and returns derived statistics. A
// character whose class row has no recovered coefficients is refused rather
// than answered with zeros, because a zero is a number and a refusal is not.

#include "stats/BaseStats.h"
#include "stats/ClassConstantTable.h"
#include "stats/Contributions.h"
#include "stats/DerivedStats.h"
#include "stats/StatCalculator.h"
#include "equipment/ItemDefinitionProvider.h"
#include "world/WorldCharacter.h"

#include <cstdint>

namespace Modern::Server::World
{
	// What the provider produced. Deliberately not "the character's stats" -
	// it is the output of one stat calculation for one character, with the
	// provenance of its inputs recorded alongside so a caller can tell a
	// verified figure from a test fixture.
	struct CombatStats
	{
		Stats::DerivedStats derived;

		// The class/gender index that was resolved, for logging and tests.
		// Meaningful only when the provider returned true.
		Stats::CharClassIndex resolvedIndex = Stats::CharClassIndex::BrawlerMale;

		// Where the coefficients came from. `Unavailable` never reaches a
		// caller: a provider that would return it returns false instead.
		Stats::CoefficientSource source = Stats::CoefficientSource::Unavailable;

		// WORLD-ENTRY-002M: what the character's equipment contributed, and
		// whether any of it was usable. Reported rather than assumed, because
		// "equipped three things and they all did nothing" and "equipped nothing"
		// are different states and only one of them is a data problem.
		//
		// An item whose definition cannot be resolved is an ERROR, not a zero: it
		// is counted here so a caller can see it, and it does not silently
		// contribute nothing.
		Stats::ItemContribution equipment;
		std::size_t              contributingSlots = 0;
		std::size_t              unresolvedItems   = 0;
		std::size_t              occupiedSlots     = 0;
	};

	// The seam.
	class ICombatStatsProvider
	{
	public:
		virtual ~ICombatStatsProvider() = default;

		// Resolves `character` into derived combat statistics.
		//
		// Returns false - and writes nothing - when the character's class/gender
		// pair is not one of the sixteen legacy values, or when that class's
		// coefficients are not recovered. Never returns zeros in place of an
		// answer.
		virtual bool TryResolve(const WorldCharacter& character,
		                        CombatStats& out) const = 0;
	};

	// The provider backed by the recovered class-constant table.
	//
	// 002L-A shipped this with no item support, so every contribution was zero
	// and the hit/avoid figures it produced were the class table's alone. 002M
	// adds equipment, which is what makes hit and avoid real: `fHIT_DEX` and
	// `fAVOID_DEX` are 0 in every recovered class row, so an unarmoured
	// character derives 0 of each and the only source of either is equipment.
	//
	// The item definitions are BORROWED, not owned: this holds a pointer to an
	// `ItemDefinitionProvider` whose lifetime the caller owns, for the same
	// reason `FieldRoleRuntime` borrows its navigation source. A null provider is
	// legal and means "no items are known", which is the state a deployment
	// without item data is in - and it is reported through the counters rather
	// than silently producing zeroes.
	//
	// Passives, codex and timed facts remain zero, and statelessly so: the live
	// world server models none of them, and RAN's term for "none of those" is
	// exactly zero.
	class ClassConstantCombatStats final : public ICombatStatsProvider
	{
	public:
		bool TryResolve(const WorldCharacter& character,
		                CombatStats& out) const override;

		// Borrows the item-definition provider. Null is legal; see above.
		void SetItemDefinitions(const ItemDefinitionProvider* provider) noexcept
		{
			m_itemDefinitions = provider;
		}

		// The class/gender index a character resolves to, using the same
		// range-checked conversion as CharacterClassMovementSpeed.cpp:15-29.
		// Exposed because "which row did this character resolve to" is a
		// question an operator asks.
		static bool TryResolveClassIndex(const WorldCharacter& character,
		                                 Stats::CharClassIndex& out) noexcept;

	private:
		// Borrowed, never owned. Null is legal and means "no items are known".
		const ItemDefinitionProvider* m_itemDefinitions = nullptr;
	};

} // namespace Modern::Server::World
