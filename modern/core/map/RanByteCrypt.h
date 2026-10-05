#pragma once

// WORLD-ENTRY-002e: RAN's `BYTECRYPT` byte-substitution cipher.
//
// Two assets in this milestone use it - `mapslist.mst` and the `.lev` files - and
// both use the SAME table. That is not a simplification; the two tables in
// legacy are byte-for-byte identical (verified by comparing
// `ARRAY_LEVEL_VAR1` at ByteCryptDefVer1.h:4-22 against `ARRAY_MAPSLIST_VAR1`
// at :367-385 - 256 identical values), so one implementation here is one
// implementation of both.
//
// ---------------------------------------------------------------------------
// WHAT THE CIPHER IS
// ---------------------------------------------------------------------------
//
// A 256-entry substitution applied per byte, not a stream cipher and not a hash:
//
//     encoded[i] = TABLE[plain[i]]
//     plain[i]   = TABLE_INVERSE[encoded[i]]
//
// `BYTECRYPT::byte_decode` (ByteCrypt.cpp:106-128) builds the inverse by
// inverting `ARRAY_USE[emCRYPT]` on the fly, then applies it. Because the table
// is a permutation (every value 0..255 appears exactly once - `InitArray` even
// asserts it, ByteCrypt.cpp:70-84, and complains through a MsgBox if it is not),
// the inverse is total: every byte value decodes.
//
// ---------------------------------------------------------------------------
// WHICH BYTES ARE CIPHERED
// ---------------------------------------------------------------------------
//
// Only the body. `CSerialFile::read` decodes what it reads (SerialFile.cpp:234),
// and `ReadFileType` reads bytes 0..131 with a RAW `fread`
// (SerialFile.cpp:55-56) - the 128-byte file-type string and the 4-byte FileID.
// `m_DefaultOffSet` is `ftell` immediately afterwards, i.e. 132
// (SerialFile.cpp:58).
//
// So the rule, exactly as for `.wld`: bytes [0, 132) are plaintext, bytes
// [132, EOF) are ciphered. `kBodyStart` encodes it.
//
// `ReadFileType` also applies the cipher SELECTION, because `GLMapList` sets the
// encode type only AFTER reading the header:
//
//     GLMapList.cpp:135-141     (FileID >= 0x0100 -> OLD, >= 0x0200 -> MAPSLIST)
//     GLLevelFileSaveLoad.cpp:183-187  (FileID >= 0x0100 -> OLD, >= 0x0200 -> LEVEL)
//
// The shipped `mapslist.mst` has FileID 0x0200 and all 144 shipped `.lev` files
// have FileID 0x0200, so both take the newest table, and both newest tables are
// the same 256 bytes. `DecodeBody` here applies that one table; an older FileID
// is reported by the callers rather than silently decoded with the wrong table.
//
// ---------------------------------------------------------------------------
// NO SHARED STATE
// ---------------------------------------------------------------------------
//
// Legacy's `ARRAY_USE` is a global filled by `InitArray` on first use. That is
// mutable global state and it is not reproduced: the table here is `constexpr`
// and the inverse is built into a caller-owned buffer. Two threads may decode
// concurrently with nothing shared. `WldCrypt` (002d) is stateless for the same
// reason and in the same way.

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Map
{
	namespace ByteCrypt
	{
		// `FILETYPESIZE` (basestream.h:17) + the FileID DWORD.
		constexpr std::size_t kFileTypeSize = 128;
		constexpr std::size_t kBodyStart   = kFileTypeSize + 4;

		// The FileID at which RAN switched to the newest table, for both asset
		// kinds: `GLMapList::VERSION_ENCODE` and `GLLevelFile::VERSION_NEW_ENCODE`
		// are both 0x0200.
		constexpr std::uint32_t kNewestEncodeFileId = 0x0200;

		// `ARRAY_LEVEL_VAR1` == `ARRAY_MAPSLIST_VAR1` (ByteCryptDefVer1.h:4 and
		// :367). Transcribed, not derived.
		extern const std::array<std::uint8_t, 256> kSubstitutionTable;

		// Fills `out` with the inverse permutation of `kSubstitutionTable`.
		//
		// Exposed rather than folded into the decode so a test can assert the
		// permutation property - that the inverse is total - instead of trusting
		// it.
		void BuildInverseTable(std::uint8_t out[256]) noexcept;

		// Applies the inverse table to `data[kBodyStart, size)`, in place.
		//
		// A no-op for a buffer shorter than `kBodyStart`: such a buffer is not a
		// RAN asset at all, and the caller reports that separately.
		void DecodeBody(std::uint8_t* data, std::size_t size) noexcept;

		inline void DecodeBody(std::vector<std::uint8_t>& data) noexcept
		{
			DecodeBody(data.data(), data.size());
		}
	}
}