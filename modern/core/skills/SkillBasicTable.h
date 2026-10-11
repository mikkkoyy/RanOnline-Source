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
//     322 cols  ->  SSKILLBASIC + SLEARN          <- SKILL-001
//     719 cols  ->  SAPPLY / CDATA_LVL            <- SKILL-002
//
// This loader reads both and joins them into one `SkillDefinition` per block.
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
// SKILL-001 recovered, because their meaning is established by the legacy
// structs and they are what an authoritative server needs to decide whether a
// character may learn a skill:
//
//     SSKILLBASIC  sNATIVEID, szNAME, dwMAXLEVEL, dwGRADE, emROLE, emAPPLY,
//                  emIMPACT_TAR, emIMPACT_SIDE, wTARRANGE, emUSE_LITEM/RITEM
//     SLEARN       dwCLASS, emBRIGHT, sSKILL, sLVL_STEP[9]
//
// SKILL-002 added the per-level effect half:
//
//     SAPPLY       emBASIC_TYPE, emELEMENT, emSTATE_BLOW
//     CDATA_LVL    fDELAYTIME, fLIFE, fBASIC_VAR, wUSE_HP, wUSE_MP, wUSE_SP
//     SSTATE_BLOW  fRATE, fVAR1, fVAR2  (per level)
//
// SKILL-003 added the impact block:
//
//     SIMPACTS    emADDON[5], fADDON_VAR[5][9]
//
// Deliberately NOT carried, with no substitute invented:
//
//   * CDATA_LVL's `wAPPLYRANGE`, `wAPPLYNUM`, `wAPPLYANGLE`, `wPIERCENUM`,
//     `wTARNUM`, the arrow/charm/bullet and EXP/CP costs, `dwDATA` and the
//     `wUSE_*_PTY` fields. Each needs a world, an inventory or an
//     item-requirement table that does not exist yet.
//   * `SSPECS` entirely - `emSPEC[5]` and `sSPEC[5][9]`. It is present and
//     clean in the export (EMSPEC_ADDON runs to 59, EMSPECA_NSIZE = 60), but
//     `PassiveSpecType` has only `None`, and legacy `SSPEC` carries fVAR1..4,
//     `dwFLAG` and two `SNATIVEID` per level where `SkillSpec` has a single
//     float array. There is no lossless home for it and `IsValid()` does not
//     consult specs, so recording part of it would be inventing the rest.
//   * `SIMPACTS::fADDON_VAR2`. Present in the export, but every runtime
//     consumer - GLChar.cpp:6543, :8571, :8799, GLCharacter.cpp:6494,
//     GLAnySummon.cpp:1527 - reads only `emADDON` and `fADDON_VAR`. The only
//     other references are the authoring editor and the CSV writer.
//   * `dwCUREFLAG`, `dwUnknownData`, `fRunningEffTime`.
//   * `SEXT_DATA` and `SSPECIAL_SKILL`, which share the 322-column line.
//   * `emIMPACT_REALM`, `emACTION`, `dwFlags`, `bLearnView`,
//     `bNonEffectRemove`, `bMobEffectRate`, `bOnlyOneStats`, `sHiddenWeapon`
//     and `bHiddenWeapon` - all read and verified present in the export, but
//     none is consulted by anything the server does today, so carrying them
//     would add fields with no consumer. They are named in the column map so a
//     later milestone can add them without re-deriving the layout.
//
// An unread field is NOT a zero field: it stays absent from the model, so a
// caller cannot mistake "not recovered" for "recovered as zero".

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

		// SKILL-002: SAPPLY rows whose per-level data was parsed and attached
		// to a canonical definition.
		std::size_t sapplyParsed = 0;

		// Definitions that received their SAPPLY half, and those that did not.
		std::size_t paired = 0;
		std::size_t unpairedCanonical = 0;

		// SAPPLY rows that could not be parsed, by reason.
		std::size_t rejectedSapplyFieldCount = 0;
		std::size_t rejectedSapplyRange = 0;

		// SAPPLY rows that arrived with no canonical row to attach to, or as a
		// second SAPPLY row inside one block.
		std::size_t rejectedUnpairedSapply = 0;

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
			       rejectedRange + rejectedUnknownSchema +
			       rejectedSapplyFieldCount + rejectedSapplyRange +
			       rejectedUnpairedSapply;
		}

	// SKILL-003: impacts recovered from `SIMPACTS`.
		std::size_t impactsRecovered = 0;

		// `emADDON` values outside the modern `PassiveImpactType` vocabulary.
		// Legacy 18..23 (CHANGESTATS, the *_RECOVERY_VAR and CP values) have no
		// modern name, so the impact is counted here and NOT recorded with a
		// guessed type.
		std::size_t rejectedUnmappableImpactType = 0;

		// SKILL-002: a definition is COMPLETE when both halves were recovered.
	// An incomplete one is still registered - it carries real learn
	// requirements - but a caller that needs a castable skill must be able
	// to tell the difference rather than discover it as zeroed effects.
	bool complete(std::size_t index) const noexcept
	{
		return index < pairedIds.size() && pairedIds[index];
	}

	// pairedIds[i] is whether definition i got its SAPPLY half.
	std::vector<bool> pairedIds;
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