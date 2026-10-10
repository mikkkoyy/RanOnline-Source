#pragma once

// WORLD-ENTRY-002L-B: the recovered class-constant table, with provenance.
//
// ---------------------------------------------------------------------------
// WHAT THIS TABLE IS
// ---------------------------------------------------------------------------
//
// RAN's per-class combat coefficients live in `GLCONST_CHARCLASS`
// (legacy/Lib_Client/G-Logic/GLogicData.h:58), and `Stats::ClassConstants`
// (modern/core/stats/BaseStats.h:137) already models every field the stat
// pipeline reads. 002L-A shipped the structure with every row unavailable,
// because the deployed data was not in the repository.
//
// 002L-B recovered it. The deployed files live at
// `D:\FILES\project\RanOnline-Build\ASURA CLIENT\data\glogic\`:
//
//   default.charclass      names each row's file through a
//                         `<CLASS>_<GENDER>.SETFILE` key
//   class0..classF.classconst   one row per EMCHARINDEX
//
// The files are Rijndael v8 (the 4-byte version prefix is 8), decryptable with
// `CRijndael::sm_Version[7]` (Rijndael.cpp:943) and the version>=5 key
// transform (Rijndael.cpp:979-986) - the same path `CStringFile::Open`
// (StringFile.cpp:84-99) takes, which is how the plaintext was read and how
// `GLCONST_CHARCLASS::LOADFILE` (GLogicDataLoad.cpp:1131-1216) parses it.
//
// Every row in this table is `CoefficientSource::Recovered`, and every row's
// `note` names its file, the SETFILE key, the key version, the loader and the
// getflag keys.
//
// ---------------------------------------------------------------------------
// WHY THE CONSTRUCTOR DEFAULTS ARE NOT IN HERE
// ---------------------------------------------------------------------------
//
// `cCONSTCLASS[GLCI_NUM_8CLASS]` (GLogicData.cpp:609) is a compile-time
// initialiser, and it looks like a table of values. It is a trap: RAN
// overwrites those fields at startup from the deployed data, and they never
// reach a running game.
//
// That is not a theory. The deployed values are in this table, and they differ
// from the constructor in every field - for row 0, fHP_STR is 2.0 against the
// constructor's 10.0, fDEFENSE_DEX is 0.032 against 0.4, and fHIT_DEX is 0
// against 0.08. 002c proved the same for the two speed fields, where the
// constructor says 12.0/34.0 and the deployed table says 12.0-16.0 / 36.0-44.0
// (see modern/core/movement/MovementSpeed.h:8-26, which keeps the constructor
// values under the name `kLegacyConstructor*` so nobody mistakes them for
// measurements).
//
// `ValidateRow` is what makes it impossible to do by accident: a row that
// claims to be `Recovered` but has a bad index, a non-finite coefficient, or
// no stated source is DEMOTED, never repaired.
//
// ---------------------------------------------------------------------------
// TWO THINGS A READER MUST KNOW ABOUT THE VALUES
// ---------------------------------------------------------------------------
//
// 1. `fHIT_DEX` and `fAVOID_DEX` are 0 in EVERY deployed row. That is the
//    data, not an omission: in this build hit and avoid have no dexterity
//    term, so they come entirely from equipment and passives. The structural
//    consequence is real - a character with no equipment has derived hit and
//    avoid of 0, so the hit rate is `100 + 0 - 0` clamped to 99.
//
// 2. `sBEGIN_STATS` and `sLVLUP_STATS` are six-space-separated tokens behind
//    `[...|...]` bracket groups. The brackets and pipes are EDITOR cosmetics;
//    `CSEPARATOR::DoSeparate` (StringUtils.cpp:340-373) drops empty tokens and
//    the loader reads all six FLAT, in pow/str/spi/dex/int/sta order
//    (GLogicDataLoad.cpp:1204-1209, :1211-1216). The deployed consequence is
//    that every class carries intel == 0.
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
