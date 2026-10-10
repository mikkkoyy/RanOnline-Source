#pragma once

// SKILL-001: recovering the canonical skill definitions from the mixed-schema
// ASURA `Skill.csv` export.
//
// ---------------------------------------------------------------------------
// WHAT THIS FILE ACTUALLY IS
// ---------------------------------------------------------------------------
//
// `GLSKILL::SaveCsvHead` (GLSkill.cpp:625) writes exactly TWO header lines and
// then `GLSKILL::SaveCsv` (:637) writes exactly TWO data lines per skill:
//
//     line 1:  SSKILLBASIC + SLEARN + SEXT_DATA + SSPECIAL_SKILL   (322 cols)
//     line 2:  SAPPLY, which carries sDATA_LVL[9]                   (719 cols)
//
// That is the format, and it is why the file looks mixed when it is not. The
// 719-column rows are not an obsolete schema at all - they are the per-level
// EFFECT data, which is the other half of every skill. An earlier read of this
// export took them for a legacy schema and under-counted; revalidating the file
// showed all 1,139 blocks are perfectly regular, with byte-identical headers.
//
// So the two schemas are not "old and new". They are "definition" and
// "per-level effects", and they belong to different legacy structs:
//
//     322 cols  ->  SSKILLBASIC + SLEARN          <- recovered HERE
//     719 cols  ->  SAPPLY / CDATA_LVL            <- NOT recovered here
//
// This loader reads the first and never interprets the second. That boundary is
// the whole reason the two are counted separately.
//
// ---------------------------------------------------------------------------
// WHY NOT LINE PARITY
// ---------------------------------------------------------------------------
//
// Legacy's own loader reads the two headers ONCE (GLSkill.cpp:1410-1416) and
// then decides by `iLine % 2` (:1427). That works because the runtime export
// writes its headers once.
//
// This export does not: it re-emits BOTH headers inside every block, so a
// parity reader would drift the moment the first repeated header appeared and
// would silently pair a definition row with the wrong effects row. Header
// DETECTION is used instead, and each data row is accepted only while it is
// provably under a verified 322-column header. A row whose width does not match
// its governing header is rejected, so a mis-parse cannot pass as data.
//
// ---------------------------------------------------------------------------
// WHAT IS RECOVERED, AND WHAT IS NOT
// ---------------------------------------------------------------------------
//
// Recovered, because their meaning is established by the legacy structs and
// they are what an authoritative server needs to decide whether a character may
// learn a skill:
//
//     SSKILLBASIC  sNATIVEID, szNAME, dwMAXLEVEL, dwGRADE, emROLE, emAPPLY,
//                  emIMPACT_TAR, emIMPACT_SIDE, wTARRANGE, emUSE_LITEM/RITEM
//     SLEARN       dwCLASS, emBRIGHT, sSKILL, sLVL_STEP[9]
//
// Deliberately NOT carried, with no substitute invented:
//
//   * SAPPLY / CDATA_LVL entirely - the 719-column rows. This is the per-level
//     effect data, and it is a separate recovery.
//   * SEXT_DATA and SSPECIAL_SKILL, which share the 322-column line but are
//     animation, impact and spec payloads.
//   * `emIMPACT_REALM`, `emACTION`, `dwFlags`, `bLearnView`,
//     `bNonEffectRemove`, `bMobEffectRate`, `bOnlyOneStats`, `sHiddenWeapon`
//     and `bHiddenWeapon` - all read and verified present in the export, but
//     none is consulted by anything the server does today, so carrying them
//     would add fields with no consumer. They are named in the column map so a
//     later milestone can add them without re-deriving the layout.

#include "skills/SkillDefinitionProvider.h"
#include "types/Result.h"

#include <cstddef>
#include <string>

namespace Modern::Skill
{
	// ---------------------------------------------------------------------------
	// WHAT A LOAD PRODUCED
	// ---------------------------------------------------------------------------
	struct SkillTableLoadResult
	{
		// Definitions admitted to the provider.
		std::size_t loaded = 0;

		// Rows recognised as a verified definition header.
		std::size_t canonicalHeaders = 0;

		// Rows recognised as a verified SAPPLY header. Counted, never read.
		std::size_t legacyHeaders = 0;

		// Rows read as skill data.
		std::size_t accepted = 0;

		// SAPPLY data rows, excluded by design. They are the per-level effect
		// half and belong to a struct this milestone does not recover.
		std::size_t excludedLegacyRows = 0;

		// Rows that could not be used, by reason.
		std::size_t rejectedFieldCount = 0;
		std::size_t rejectedIdentity   = 0;
		std::size_t rejectedName       = 0;
		std::size_t rejectedRange      = 0;

		// Canonical rows whose id was already seen. Reported, never applied:
		// the second row for an id is NOT allowed to overwrite the first.
		std::size_t duplicateIds = 0;

		// Rows that matched no known header at all.
		std::size_t rejectedUnknownSchema = 0;

		std::size_t blankLines = 0;

		std::size_t Rejected() const noexcept
		{
			return rejectedFieldCount + rejectedIdentity + rejectedName +
			       rejectedRange + rejectedUnknownSchema;
		}
	};

	// Reads the verified 322-column definition rows out of `path` into
	// `provider`.
	//
	// A row that cannot be used is skipped and counted; it never becomes a
	// zeroed definition, because a skill that teaches nothing because nothing
	// knew what it was would hide a data problem behind a plausible number.
	//
	// Returns InvalidArgument when the file cannot be opened, or when it
	// contains no verified definition header at all - a file that is not this
	// export, or an export whose layout changed, is refused rather than
	// half-read.
	Result<SkillTableLoadResult> LoadSkillCsv(const std::string& path,
	                                          InMemorySkillDefinitions& provider);

	// The column count of the last recognised definition header. 0 when no
	// file has been loaded. Exposed so a test can reconcile against the export
	// instead of hard-coding 322.
	std::size_t LastCanonicalHeaderColumns() noexcept;

} // namespace Modern::Skill