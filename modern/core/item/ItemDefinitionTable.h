#pragma once

// The item-definition table, loaded from the tracked legacy CSV export.
//
// ---------------------------------------------------------------------------
// WHERE THE VALUES COME FROM
// ---------------------------------------------------------------------------
//
// `reference/data-formats/Item.csv` is a tracked export of RAN's item database,
// produced by the legacy `GLItemMan::SaveCsvFile` (GLItemMan.cpp:713-741), which
// writes `SITEM::SaveCsvHead` (GLItem.cpp:644) followed by one
// `SITEM::SaveCsv` (GLItem.cpp:664) per item. This loader reads that same file
// back, using the same column order.
//
// The header is 399 columns. `SITEMBASIC` occupies 0..90, and `SSUIT` - the
// block every combat-relevant field lives in - starts at column 91:
//
//     18  emItemType        91  emSuit            92  dwHAND
//     93  emHand            94  emAttack          95  wAttRange
//     96  wReqSP            97  nHitRate          98  nAvoidRate
//     99  gdDamage wLow    100  gdDamage wHigh   101  nDefense
//    102..106  sResist nFire/nIce/nElectric/nPoison/nSpirit
//
// Every one of those positions is read from the HEADER at load time rather
// than hard-coded, so a re-export that reorders the columns is refused rather
// than silently mis-parsed. `GLItemMan::LoadCsvFile` (GLItemMan.cpp:747-804)
// parses the same file the same way, through `SITEM::LoadCsv` (GLItem.cpp:684).
//
// FIELD-COUNT POLICY
//
// The export writes a final comma, so both its header and every data row carry
// one more empty field than they have NAMED columns: 399 fields for 398 names.
// The trailing empty field is therefore a real field, and is counted on BOTH
// sides - the header and the rows are split by the same pass, so their counts
// are comparable and the per-row width check means what it says. Getting this
// wrong is not theoretical: splitting the header with `std::getline` (which
// drops a final empty field) while splitting rows by `find` (which keeps it)
// made the header one field short and had the loader reject all 18,447 rows as
// malformed.
//
// WHY NOT THE BINARY `.isf`
//
// `SITEM::LoadFile` (GLItem.cpp:120) reads a version-tagged chunked binary with
// about twenty migration paths per record. The CSV is legacy's OWN alternative
// for exactly this reason - it is what the editor's "Save CSV" produces - and it
// carries every field this milestone needs. Adding an ISF decryption dependency
// to the runtime would duplicate a loader for no gain, so it is deliberately
// absent. The `.isf` remains the format to consult when a field turns out to be
// CSV-unreachable.
//
// WHAT IS CLAIMED
//
// The loaded values are the DEPLOYED ones, from a tracked file. What is NOT
// claimed is that the loader understands all 399 columns: `SSUIT::sADDON`,
// `sVARIATE`, `sVOLUME` and `sBLOW` are read only far enough to count them, and
// random options (which live on the item INSTANCE, not the definition) are absent
// by design - the aggregator's contract has always been definition-base only.

#include "item/ItemDefinition.h"
#include "item/ItemIdentity.h"
#include "equipment/ItemDefinitionProvider.h"
#include "types/Result.h"

#include <cstddef>
#include <string>

namespace Modern::Item
{
	// ---------------------------------------------------------------------------
	// WHAT A LOAD PRODUCED
	// ---------------------------------------------------------------------------
	struct ItemTableLoadResult
	{
		// Rows that produced a definition.
		std::size_t loaded = 0;

		// Rows that could not be used, by reason. Counted separately rather than
		// lumped together, because "the file has 90 pet rows this loader does not
		// want" and "the file is truncated" need different responses.
		std::size_t rejectedFieldCount = 0;
		std::size_t rejectedIdentity  = 0;
		std::size_t rejectedName      = 0;
		std::size_t rejectedRange     = 0;

		std::size_t Rejected() const noexcept
		{
			return rejectedFieldCount + rejectedIdentity + rejectedName +
			       rejectedRange;
		}
	};

	// Reads the tracked legacy CSV into an `InMemoryItemDefinitions`.
	//
	// The provider is FILLED, not taken over: existing definitions are kept and
	// same-id rows replace them, so a caller can layer a private set under a
	// deployed one.
	//
	// A row that cannot be used is SKIPPED and counted. It never becomes a
	// zero-stat definition, because an item that contributes nothing because
	// nothing knows what it is would hide a data problem behind a plausible
	// number.
	//
	// Returns InvalidArgument when the file cannot be read at all, or when its
	// header does not name the columns this loader reads.
	Result<ItemTableLoadResult> LoadItemCsv(const std::string& path,
	                                        InMemoryItemDefinitions& provider);

	// The number of columns the loaded file's header declares. 0 when no file
	// has been loaded. Exposed so a test can reconcile the header against the
	// export rather than hard-coding 399.
	std::size_t LastHeaderColumns() noexcept;

} // namespace Modern::Item
