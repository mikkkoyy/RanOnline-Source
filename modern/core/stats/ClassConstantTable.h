#pragma once

// WORLD-ENTRY-002L-A: the recovered class-constant table, with provenance.
//
// ---------------------------------------------------------------------------
// WHAT THIS TABLE IS, AND WHAT IT IS NOT
// ---------------------------------------------------------------------------
//
// RAN's per-class combat coefficients live in `GLCONST_CHARCLASS`
// (legacy/Lib_Client/G-Logic/GLogicData.h:58), and `Stats::ClassConstants`
// (modern/core/stats/BaseStats.h:137) already models every field the stat
// pipeline reads. What is missing is the DATA: GLCONST_CHARCLASS's rows are
// populated at runtime from `default.charclass` -> `class<N>.classconst`
// (legacy/Lib_Client/G-Logic/GLogicDataLoad.cpp:1131-1214 reads exactly these
// fields out of that file), and those data files are NOT in this repository.
//
// So this table carries structure and provenance, and no values. Every row is
// `CoefficientSource::Unavailable`, which means: no coefficient for that class
// has been verified, and the caller must keep its explicitly named prototype
// fallback. That is the honest state and it is deliberately NOT papered over
// by the constructor defaults in GLogicData.cpp:609 - see the next section.
//
// ---------------------------------------------------------------------------
// WHY THE CONSTRUCTOR DEFAULTS MUST NOT BE COPIED IN HERE
// ---------------------------------------------------------------------------
//
// `cCONSTCLASS[GLCI_NUM_8CLASS]` (GLogicData.cpp:609) is a compile-time
// initialiser, and it looks like a table of values. It is a trap:
//
//   * `GLCONST_CHARCLASS::LOADFILE` (GLogicDataLoad.cpp:1131) reads
//     fWALKVELO/fRUNVELO/fHP_STR/fHIT_DEX/fPA_POW/wBEGIN_AP/sBEGIN_STATS/
//     sLVLUP_STATS and every other field out of the per-class data file, and
//     `default.charclass` names that file per class (GLogicDataLoad.cpp:557-570).
//   * RAN therefore overwrites the constructor values at startup, and they
//     never reach a running game.
//   * WORLD-ENTRY-002c proved exactly that for the two speed fields: the
//     constructor values are 12.0/34.0 and the deployed values are
//     12.0-16.0 / 36.0-44.0 (see modern/core/movement/MovementSpeed.h:8-26,
//     which keeps the constructor values under the name `kLegacyConstructor*`
//     precisely so they cannot be mistaken for measured ones).
//
// Nothing here suggests the combat coefficients are the exception. A row whose
// values came from the constructor would therefore be WRONG, and shipping one
// as "recovered" would be worse than shipping nothing: a later reader would
// trust it. `ValidateRow` is what makes that impossible to do by accident.
//
// ---------------------------------------------------------------------------
// THE SHAPE A RECOVERED ROW MUST TAKE
// ---------------------------------------------------------------------------
//
// A row is available only when EVERY coefficient the stat pipeline reads is
// present and finite. There is deliberately no partial provenance: a row that
// had, say, wBEGIN_AP but not fPA_POW would let a caller read the missing half
// as zero and compute a confidently wrong melee power. All-or-nothing means a
// caller either has a verified row or an explicit refusal, never a silent
// half-answer. The per-field history is recorded in the row's `note`, which
// names the source file and row for each field that was recovered.

#include "stats/BaseStats.h"

#include <cstddef>
#include <cstdint>

namespace Modern::Stats
{
	// Where a row's coefficients came from.
	enum class CoefficientSource : uint8_t
	{
		// No coefficient for this class has been verified. The row's
		// `constants` are the struct defaults and MUST NOT be read; a caller
		// that wants a number must supply its own and label it.
		Unavailable,

		// Every coefficient was read from the deployed `class<N>.classconst`
		// row and passed `ValidateRow`. Only then may the values be used.
		Recovered,
	};

	const char* ToString(CoefficientSource source) noexcept;

	// One class's row of the recovered table.
	struct ClassConstantRow
	{
		// The legacy EMCHARINDEX this row describes.
		CharClassIndex index = CharClassIndex::BrawlerMale;

		// Provenance. `Unavailable` means the rest of the row is not data.
		CoefficientSource source = CoefficientSource::Unavailable;

		// The coefficients. Meaningful only when `source == Recovered`.
		ClassConstants constants;

		// Provenance in words: which file and row each field came from, or why
		// the row is unavailable. Never empty - a row that cannot explain
		// itself has not been recorded.
		const char* note = "";
	};

	// ---------------------------------------------------------------------------
	// ROW VALIDATION
	// ---------------------------------------------------------------------------
	//
	// Refuses a row that claims to be Recovered but is not fit to read:
	//
	//   * an index outside the sixteen legacy values;
	//   * a non-finite coefficient - RAN's loader accepts whatever the data
	//     file contains, and one NaN would make every derived stat NaN;
	//   * an empty note - provenance that cannot be stated cannot be audited.
	//
	// A refused row is DEMOTED to `Unavailable` with the reason written into
	// `note`, and the function returns false. It never edits a value into
	// agreement, because a table that repairs itself cannot be audited.
	bool ValidateRow(ClassConstantRow& row) noexcept;

	// The table this repository can defend today.
	//
	// All sixteen legacy class/gender rows, every one `Unavailable`. The rows
	// exist so that a caller can enumerate the classes it DOES know about and
	// be told, per class, that the coefficients are missing - which is a
	// different and more useful answer than "unknown class".
	class ClassConstantTable
	{
	public:
		// The number of legacy class/gender combinations: GLCI_NUM_8CLASS.
		static constexpr std::size_t kRowCount = static_cast<std::size_t>(kClassCount);

		// The verified table. Constant, and shared: a row is data, not state.
		static const ClassConstantTable& Verified() noexcept;

		// The row for an index, or nullptr when the index is not one of the
		// sixteen legacy values. A nullptr is "no such class", never "no data".
		const ClassConstantRow* Find(CharClassIndex index) const noexcept;

		// The row at a table position, for enumeration. `position` is checked;
		// an out-of-range position returns the first row rather than reading
		// out of bounds, because a test walking the table should not be able to
		// crash the process.
		const ClassConstantRow& RowAt(std::size_t position) const noexcept;

		// How many rows are actually usable. Zero today, and reported so a
		// caller can tell "the table is empty" from "the table is unread".
		std::size_t RecoveredCount() const noexcept;

	private:
		ClassConstantRow m_rows[kRowCount];
	};

} // namespace Modern::Stats
