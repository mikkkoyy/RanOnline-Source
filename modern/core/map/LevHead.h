#pragma once

// WORLD-ENTRY-002e: read `m_strWldFile` out of a `.lev` file.
//
// This is the middle link in the chain WORLD-ENTRY-002c proved and nothing more:
//
//     MapIdentity -> mapslist.mst -> strFile -> <this> -> m_strWldFile -> .wld
//
// ---------------------------------------------------------------------------
// IT IS NOT THE LEVEL ENGINE, AND THE HEADER SAYS WHY THAT MATTERS
// ---------------------------------------------------------------------------
//
// A `.lev` is a complete RAN level: terrain, gates, collision, mob spawns,
// weather, and the `SLEVEL_REQUIRE` / `SLEVEL_ETC_FUNC` policy blocks. All of it
// is deliberately absent here. What is present is `SLEVEL_HEAD`
// (GLLevelHead.h:28-50), whose `m_strWldFile` is the only field this milestone
// needs.
//
// The line drawn is: read the header, stop. `GLLevelFile::LoadFile`
// (GLLevelFileSaveLoad.cpp:152) would continue into mob spawners and terrain,
// which need D3D resources. Nothing here touches those.
//
// ---------------------------------------------------------------------------
// THE FILE
// ---------------------------------------------------------------------------
//
//     0    128  "glmap" + NUL padding        GLLevelFile::FILE_EXT, GLLevelFile.cpp:11
//   128      4  FileID = 0x0200              GLLevelFile::VERSION, GLLevelFile.h:44
//   ------ body begins: m_DefaultOffSet = 132, SerialFile.cpp:58 ------
//   132      4  head version = 0x0102        SLEVEL_HEAD::VERSION, GLLevelHead.h:30
//   136      4  head payload size
//   140    ...  the head payload
//
// Measured over all 144 shipped `.lev` files: FileID 0x0200 in every one, head
// version 0x0102 in every one, and the declared payload size equal to the bytes
// consumed in every one.
//
// ---------------------------------------------------------------------------
// THE TWO HEAD VERSIONS DIFFER IN THE ORDER OF TWO STRINGS
// ---------------------------------------------------------------------------
//
//     LOAD_0102 (GLLevelHead.cpp:48-63)   strMapName, strWldFile, division, bright
//     LOAD_0101 (GLLevelHead.cpp:31-46)   strWldFile, strMapName, bright, division
//
// The `SLEVEL_HEAD_100` path (`GLLevelFile::LOAD_000`,
// GLLevelFileSaveLoad.cpp:260-262) reads a fixed 264-byte POD with no length
// prefixes at all, and is NOT implemented - a version-0 FileID is reported rather
// than guessed at, and no shipped file uses one.
//
// Both orders are implemented. Reading 0x0102 with the 0x0101 order yields a
// plausible-looking filename that names the wrong file, which is the worst
// possible failure mode for this field, so it is worth both branches.
//
// ---------------------------------------------------------------------------
// THE PAYLOAD SIZE IS CHECKED, NOT TRUSTED
// ---------------------------------------------------------------------------
//
// Legacy reads the fields and then compares the consumed count with `dwSize`,
// reporting a mismatch through a `MsgBox` (GLLevelHead.cpp:61-62) and carrying on
// regardless. That is a diagnostic, not a validation.
//
// Here a mismatch is `Inconsistent`. The reader does not skip to `dwSize` on
// disagreement, because nothing downstream needs to: the next thing anyone wants
// from a `.lev` is a fresh one, and this file is the one being decoded. Reporting
// the disagreement is strictly more useful than silently trusting either number.

#include "map/RanByteCrypt.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace Modern::Map
{
	enum class LevHeadStatus
	{
		// The header parsed and the declared payload size matched the bytes read.
		Loaded,
		// Shorter than the 132-byte header, or the file type is not "glmap".
		InvalidFile,
		// A FileID selecting a `BYTECRYPT` table this milestone does not carry.
		UnsupportedFormat,
		// A head version with no branch in `SLEVEL_HEAD::LOAD`.
		UnsupportedHeadVersion,
		// A read ran past the end of the buffer.
		Truncated,
		// A length field that cannot be true, a string with no NUL, or a declared
		// payload size that disagrees with what was read.
		Inconsistent,
	};

	const char* ToString(LevHeadStatus status) noexcept;

	struct LevHeadError
	{
		LevHeadStatus status = LevHeadStatus::InvalidFile;
		std::string   stage;
		std::string   detail;
		std::size_t   offset    = 0;
		bool          hasOffset = false;

		std::string Format(const std::string& fileName) const;
	};

	// What `SLEVEL_HEAD` yielded. Only the fields this milestone needs plus the
	// two enums, which come free in the same reads and are useful for a log line.
	struct LevHead
	{
		// `m_strWldFile`: the `.wld` file name, relative to the map directory.
		// Stored exactly as the asset spells it.
		std::string wldFileName;

		// `m_strMapName`. Empty in the shipped files - measured across all 144 -
		// which is why it is not used for anything. Read anyway, because the two
		// head versions differ in which string comes FIRST and skipping it would
		// desynchronise every subsequent read.
		std::string mapName;

		// `m_eDivision`, `m_emBright`. Raw values; the enum meanings are level
		// policy and out of scope here.
		std::uint32_t division = 0;
		std::uint32_t brightness = 0;

		std::uint32_t fileId      = 0;
		std::uint32_t headVersion = 0;
		std::uint32_t declaredPayloadBytes = 0;
		std::size_t   consumedPayloadBytes  = 0;
	};

	// Decodes the `SLEVEL_HEAD` of a `.lev` from a buffer already in memory.
	//
	// Takes bytes rather than a path, for the same reason `DecodeMapsList` does:
	// the parser stays independent of the filesystem, and a truncation test can
	// drive a malformed input without building a file. The caller's buffer is NOT
	// modified.
	bool DecodeLevHead(const std::uint8_t* data, std::size_t size, LevHead& out,
	                   LevHeadError& error);

	// The file-type string RAN writes, `GLLevelFile::FILE_EXT`
	// (GLLevelFile.cpp:11). Seven characters, NUL-padded to FILETYPESIZE.
	extern const char* const kLevFileType;
}