#pragma once

// WORLD-ENTRY-002d: parse the navigation block out of a decoded RAN `.wld`.
//
// This is the byte-level half of the asset path. It takes a buffer that has
// already been decrypted (see WldCrypt.h) and produces exactly what
// `NavigationMesh::Build` needs - and nothing else. It does not open files, does
// not move characters, and does not know what a Field is.
//
// ---------------------------------------------------------------------------
// THE FILE LAYOUT, AND WHERE EACH OFFSET COMES FROM
// ---------------------------------------------------------------------------
//
// Read off legacy, in the order `GLLandMan::LoadWldFile` reads it
// (GLLandManSet.cpp:10-88) and `SLAND_FILEMARK::LoadSet` (DxLandDef.cpp:23-52):
//
//     0    128  file type string          FILETYPESIZE, basestream.h:17
//     128    4  FileID                    SerialFile.cpp:56
//   ------ body begins: m_DefaultOffSet = 132, SerialFile.cpp:58 ------
//     132    4  SNATIVEID::dwID            GLLandManSet.cpp:54
//     136  128  map name                  MAXLANDNAME = 128, DxLandDef.h:16
//     264    4  file mark version         SLAND_FILEMARK::LoadSet
//     268    4  file mark payload size
//     272   16  file mark payload
//
// and then, at `132 + dwNAVI_MARK`:
//
//         + 4  bExist                     GLLandManSet.cpp:65-67
//         + 4  vertex count               NavigationMesh::LoadFile
//         +12*vertexCount  vertices
//         + 4  cell count
//         +188*cellCount   NavigationCell records
//         +12*cellCount    links: per cell, three (BOOL present, DWORD id)
//
// ---------------------------------------------------------------------------
// THE +4 ON THE NAVIGATION MARK - the offset that is easy to lose
// ---------------------------------------------------------------------------
//
// `GLLandManSet.cpp:64-67` is:
//
//     SFile.SetOffSet ( sLandMark.dwNAVI_MARK );
//     SFile >> bExist;
//     if ( bExist ) { m_pNaviMesh->LoadFile ( SFile ); }
//
// `SetOffSet` is RELATIVE (`fseek(_OffSet + m_DefaultOffSet)`, SerialFile.cpp:167),
// so the navigation block starts at `132 + dwNAVI_MARK + 4`. Reading the
// navigation data AT the mark, with no `bExist` shift, yields an implausible cell
// count on every one of the 87 shipped files. WORLD-ENTRY-002c §6.2 measured the
// 4-byte shift; this is where it is applied.
//
// `bExist` is a `BOOL` (GLLandManSet.cpp:26), so it is 4 bytes. There is a
// `CSerialFile::operator>>(bool&)` that would read 1 byte, but the call site is
// typed `BOOL`, which selects `operator>>(int&)`.
//
// ---------------------------------------------------------------------------
// THE FILE-MARK PAYLOAD ORDER CHANGES WITH ITS VERSION
// ---------------------------------------------------------------------------
//
// `SLAND_FILEMARK` (version 0x0101) is NAVI, WEATHER, GATE, COLL.
// `SLAND_FILEMARK_100` (version 0x0100) is NAVI, GATE, COLL, WEATHER.
//
// The first field is the same in both, which is why a reader that only cares
// about navigation gets away with reading the first DWORD and never notices. The
// three later marks are read here anyway, because a reader that guessed the order
// would misreport them, and they cost nothing.
//
// ---------------------------------------------------------------------------
// WHAT IS NOT VALIDATED, AND WHY
// ---------------------------------------------------------------------------
//
// The map id at offset 132 is read and reported but NOT validated. WORLD-ENTRY-002c
// §1 measured it as **0 for all 87 files**, so it carries no identity at all - the
// map registry (`mapslist.mst`) is what names a map, and the `.wld` is addressed
// entirely through it. Rejecting `m_MapID == 0` would reject every shipped map.
//
// Likewise nothing is validated about the gate, collision or weather marks. They
// are outside this reader's responsibility and are reported as read.

#include "math/Vector3.h"
#include "navigation/NavigationCell.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Navigation
{
	// The outcome of a load attempt.
	//
	// `NoNavigation` is deliberately NOT an error: WORLD-ENTRY-002c measured 9 of
	// the 87 shipped files with `bExist == 0` - the character-select and login
	// maps - and a character that is never walked on them must be able to say so
	// without the load failing.
	enum class NavigationLoadStatus
	{
		// A navigation mesh was decoded and is ready to build.
		Loaded,
		// The file is valid and has no navigation mesh (`bExist == 0`).
		NoNavigation,
		// Too small, unknown file type, or a FileID legacy would refuse.
		InvalidFile,
		// A recognised file whose version this reader does not implement.
		UnsupportedFormat,
		// The file announced itself encrypted but no decryption pass produced a
		// usable file mark.
		DecryptionFailed,
		// A block ran past the end of the buffer.
		Truncated,
		// Self-inconsistent: a cell id that is not its index, a vertex index out
		// of range, a link id that names no cell.
		Inconsistent,
	};

	const char* ToString(NavigationLoadStatus status) noexcept;

	// Why a load failed, in enough detail to act on.
	//
	// Deliberately small: a stage name and a short reason, never a dump of the
	// buffer. A 34 MB `.wld` in a log line helps nobody.
	struct WldNavigationError
	{
		NavigationLoadStatus status = NavigationLoadStatus::InvalidFile;
		std::string         stage;    // "file type", "FileID", "file mark", ...
		std::string         detail;   // short reason
		std::size_t         offset = 0; // where useful, in bytes from file start
		bool                hasOffset = false;

		// `"<file>: <stage>: <detail> (at offset N)"`, or without the offset.
		std::string Format(const std::string& fileName) const;
	};

	// Everything the navigation block contained.
	//
	// The cell records are kept as raw bytes rather than being decoded here. That
	// is not laziness about format knowledge - the decoder is
	// `NavigationCell::RestoreFromRecord`, which is shared with every other
	// producer of cell records - and it means the 188 bytes survive the parse
	// unchanged, so a test can assert on the file's own numbers.
	struct WldNavigationData
	{
		std::uint32_t fileId           = 0;
		std::uint32_t fileMarkVersion  = 0;
		std::uint32_t fileMarkPayloadBytes = 0;
		std::uint32_t naviMark         = 0;
		std::uint32_t weatherMark      = 0;
		std::uint32_t gateMark         = 0;
		std::uint32_t collMark         = 0;
		std::uint32_t mapId            = 0;

		bool encrypted       = false;
		std::uint32_t decryptPasses = 0;

		std::vector<Vector3>       vertices{};
		std::vector<std::uint8_t> cellRecords{};  // cellCount * 188
		std::vector<std::uint32_t> linkIds{};      // cellCount * 3

		std::size_t CellCount() const noexcept
		{
			return cellRecords.size() / NavigationCell::kCellRecordBytes;
		}
	};

	// `NSLANDMAN_SUPPORT::IsLandManSupported` (DxLandManSaveLoad.cpp:3165-3190).
	//
	// This is the check legacy applies at GLLandManSet.cpp:41, and it is what
	// rejects `square_rd.wld`: that file carries FileID 0x0202, which is not in
	// the list, so **legacy itself would refuse to load it**. Not a modern gap.
	bool IsSupportedLandManVersion(std::uint32_t version) noexcept;

	// Parses an already-decrypted `.wld`.
	//
	// Returns true when `out` holds a usable navigation mesh OR the file is
	// validly navigation-free (`NoNavigation`); `error.status` distinguishes the
	// two. Returns false only for a controlled failure, and in every failing case
	// `error.status` and `error.stage` say which check tripped.
	bool ParseWldNavigation(const std::uint8_t* data, std::size_t size, WldNavigationData& out,
	                        WldNavigationError& error);
}