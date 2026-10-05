#pragma once

// WORLD-ENTRY-002e: decode `mapslist.mst` into map records.
//
// This is the AUTHORITATIVE source of map identity. Not the `.wld` - 002c measured
// `m_MapID == 0` in all 87 shipped files - and not a filename, since 16 of the 56
// files named by the 99 registered maps are shared by more than one map id.
//
// ---------------------------------------------------------------------------
// THE FILE
// ---------------------------------------------------------------------------
//
// Transcribed from `GLMapList::LoadMapsListFile` (GLMapList.cpp:108-218),
// `SMAPNODE_DATA::LOAD` (GLMapNode.cpp:133-438) and `CSerialFile::operator>>`
// (SerialFile.cpp:475-605).
//
//     0    128  "GLMAPS_LIST"               GLMapList::_FILEHEAD, GLMapList.cpp:14
//   128      4  FileID = 0x0200              GLMapList::VERSION, GLMapList.h:21
//   ------ body begins: m_DefaultOffSet = 132, SerialFile.cpp:58 ------
//   132      4  record count
//          ...  SMAPNODE_DATA records
//
// The body is ciphered with the newest `BYTECRYPT` table; see RanByteCrypt.h.
//
// ---------------------------------------------------------------------------
// THE PRIMITIVES, AND WHY THE WIDTHS ARE WHAT THEY ARE
// ---------------------------------------------------------------------------
//
// `CSerialFile` has an overload per type and each `read`s `sizeof` of it:
//
//     DWORD 4    WORD  2    BYTE/bool/char 1    string: DWORD length THEN length bytes
//
// The `bool` width is worth stating because it is the one that surprises: MSVC's
// `sizeof(bool)` is 1, and `SMAPNODE_DATA` declares its 11 + 16 flags as `bool`,
// so each is ONE byte. Reading them as DWORDs is the first thing that goes wrong
// in this format.
//
// A `std::string` is stored as `DWORD(length + 1)` followed by `length + 1` bytes
// including the NUL (SerialFile.cpp:446-448 on write, :590-605 on read). So the
// stored length is the byte count, and the character count is one less.
//
// ---------------------------------------------------------------------------
// THE RECORD LAYOUT IS VERSION-DEPENDENT, AND THE TAIL IS WHAT CHANGES
// ---------------------------------------------------------------------------
//
// Every version from 0x0100 to 0x0203 shares one prefix and differs only in how
// many `bool` flags follow the three strings. `SMAPNODE_DATA::LOAD` is eight
// near-identical branches, and collapsing them is what this table is:
//
//     version  flags after the three strings
//     0x0203        16
//     0x0202        15
//     0x0201        14
//     0x0200        10
//     0x0103         5
//     0x0102         3
//     0x0101         3
//     0x0100         2
//
// The shipped file is 0x0203 in all 99 records. The other seven are implemented
// because they are eight lines each and a parser that silently accepted only the
// newest would report a version mismatch as a truncation.
//
// ---------------------------------------------------------------------------
// WHAT IS KEPT AND WHAT IS SKIPPED
// ---------------------------------------------------------------------------
//
// Kept: identity, the `.lev` filename, `dwFieldSID`, `bUsed`, the map name, and
// the flag count actually present.
//
// Skipped, by reading and discarding: `strBGM`, `strLoadingImageName`, and the
// individual flag VALUES. That is a deliberate limit, not an oversight. The
// fields that decide which maps exist are the four the registration filter reads;
// the rest are presentation and zone policy, and this milestone is an asset/data
// layer, not a gameplay rule set. `strBGM` and `strLoadingImageName` ARE fully
// decoded and their lengths validated - only the strings themselves are not
// retained - so a corrupt one still fails the parse instead of being skipped.
//
// What that means for a caller: this is not yet enough to answer "is this a PK
// zone", and it is not meant to be. See MapRegistry.h for what IS promised.

#include "map/MapIdentity.h"
#include "map/RanByteCrypt.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Map
{
	// What a decode produced.
	enum class MapsListStatus
	{
		// Every record parsed and the payload was consumed exactly.
		Loaded,
		// Shorter than the 132-byte header, or the file type is not
		// "GLMAPS_LIST".
		InvalidFile,
		// A FileID that selects a `BYTECRYPT` table this milestone does not
		// carry. See MapRegistry.h.
		UnsupportedFormat,
		// A read ran past the end of the buffer.
		Truncated,
		// A length field that cannot be true: a string length of zero (RAN always
		// stores at least the NUL), a string whose last byte is not NUL, a
		// negative or absurd record count, bytes left over, or a version no
		// branch of `SMAPNODE_DATA::LOAD` handles.
		Inconsistent,
	};

	const char* ToString(MapsListStatus status) noexcept;

	// Why a decode failed. A stage, a short reason, and an offset when there is
	// one - never a dump of the buffer.
	struct MapsListError
	{
		MapsListStatus status = MapsListStatus::InvalidFile;
		std::string    stage;
		std::string    detail;
		std::size_t    offset    = 0;
		bool           hasOffset = false;

		std::string Format(const std::string& fileName) const;
	};

	// `MAP_NAME_MAX` (GLMapNode.h:15). `LoadMapsListFile` refuses a longer map
	// name (GLMapList.cpp:172-176), so a record with one is present-but-rejected
	// rather than usable, and this exposes that.
	inline constexpr std::size_t kMapNameMax = 16;

	// One registered map, as `mapslist.mst` describes it.
	//
	// Immutable once the owning `MapsListFile` is built.
	struct MapRecord
	{
		MapIdentity identity{};

		// `strFile`: the `.lev` file name, relative to the level directory. Stored
		// EXACTLY as the asset spells it - `w_school_01.Lev` and
		// `w_city_s_01.lev` both occur in the shipped file, and 25 of the 99 use
		// the lowercase extension. Case is a filesystem concern, resolved at open
		// time; see MapRegistry.h.
		std::string levelFileName;

		// `strMapName`: `SG_Campus`, `SacredGateHole`, ...
		std::string mapName;

		// `dwFieldSID`: which field server owns the map. 0 for all 99 shipped
		// records - RAN's deployed list declares a single field server
		// (GLMapList.cpp:178-189).
		std::uint32_t fieldServerId = 0;

		// `bUsed`. All 99 shipped records are 1.
		bool used = false;

		// The record's own version, `SMAPNODE_DATA::VERSION` in the shipped file.
		std::uint32_t recordVersion = 0;

		// How many trailing `bool` flags the version carries, after the three
		// strings. 16 for 0x0203. Kept so a caller can tell "the version I do not
		// model fully" from "the version I mis-parsed".
		std::uint32_t flagCount = 0;

		// True when the map name is within `MAP_NAME_MAX`, i.e. RAN's registration
		// filter would keep this record. False means present in the file but not
		// usable, and it is reported rather than dropped so the count of records in
		// the file stays honest.
		bool RegistrationEligible() const noexcept
		{
			return used && identity.InRange() && mapName.size() <= kMapNameMax;
		}
	};

	// A decoded `mapslist.mst`.
	struct MapsListFile
	{
		std::vector<MapRecord> records{};

		// The FileID read from the header. 0x0200 in the shipped asset.
		std::uint32_t fileId = 0;

		// Byte accounting, so a caller - and a test - can prove the parse consumed
		// the payload rather than stopping early.
		//
		// The shipped file is 11,362 of 11,362 body bytes with zero residue. That
		// exact check is what rules out a wrong field order or a wrong substitution
		// table, because either would leave bytes over or run past the end.
		std::size_t payloadBytes    = 0;
		std::size_t consumedBytes   = 0;
		std::uint32_t declaredCount = 0;
	};

	// Decodes `mapslist.mst` from a buffer already in memory.
	//
	// Takes bytes rather than a path so the parser is independent of the
	// filesystem and a malformed-input test can drive truncation without building
	// files. The caller's buffer is NOT modified; the body is copied before being
	// decoded, because decoding is a destructive in-place transform and mutating a
	// buffer the caller still holds would be a nasty surprise.
	//
	// `data` is decoded only when `fileId >= kNewestEncodeFileId`, which is what
	// `GLMapList::LoadMapsListFile` does at GLMapList.cpp:137-141.
	bool DecodeMapsList(const std::uint8_t* data, std::size_t size, MapsListFile& out,
	                    MapsListError& error);

	// The file-type string RAN writes, `GLMapList::_FILEHEAD` (GLMapList.cpp:14).
	extern const char* const kMapsListFileType;
}