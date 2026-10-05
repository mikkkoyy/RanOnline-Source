#pragma once

// WORLD-ENTRY-002d: the RAN `.wld` body cipher.
//
// Transcribed from legacy/Lib_Engine/Common/WLDCrypt.cpp:16-52 and its
// constants in WLDCrypt.h:16-20. Nothing here is inferred: every value below is
// read out of those two files.
//
// ---------------------------------------------------------------------------
// THE CIPHER
// ---------------------------------------------------------------------------
//
//     CWLDCrypt::Decryption_WLD(BYTE* B, int dwSize, bool bTool = false)
//     {
//         for (int i = 0; i < dwSize; i++)
//         {
//             if (!bTool) { B[i] -= (K1 ^ K2); B[i] ^= K1; B[i] ^= K2; }
//             ...
//         }
//     }
//
// with K1 = 0x099701AE and K2 = 0x092617BE (WLDCrypt.h:18-19). `B[i]` is a BYTE,
// so every step truncates to the low 8 bits and the whole cipher collapses to
//
//     b = ((b - 0x10) mod 256) ^ 0x10
//
// because 0xAE ^ 0xBE == 0x10. The K1/K2 pair is kept as written in the source
// and folded at run time rather than being replaced by the folded constant, so
// the next reader can check this derivation against the original.
//
// ---------------------------------------------------------------------------
// WHY THE HEADER IS NOT DECRYPTED - the part that is easy to get wrong
// ---------------------------------------------------------------------------
//
// The obvious reading of `Decryption_WLD` is "decrypt the file", which suggests
// bytes 0..127 get special treatment (there is an `i < 128` branch that replaces
// them with a fixed string). It does not, for two independent reasons:
//
//   1. That branch is `else if`, behind `bTool`. `CSerialFile::read` calls
//      `Decryption_WLD(pbuffer, dwSize)` with TWO arguments
//      (SerialFile.cpp:232), so `bTool` takes its default of `false` and the
//      `i < 128` / `i > 131` branches are unreachable on the load path. The
//      cipher is uniform over the whole buffer.
//
//   2. `CSerialFile::ReadFileType` reads bytes 0..127 (the 128-byte file-type
//      string) and 128..131 (the FileID DWORD) with raw `fread`
//      (SerialFile.cpp:55-57), never through `read()`. The cipher does not touch
//      them, at any `bTool`.
//
// `m_DefaultOffSet` is set to `ftell` immediately after that header
// (SerialFile.cpp:58), which is 128 + 4 = 132, and every mark in the format is
// relative to it (`SetOffSet` seeks to `_OffSet + m_DefaultOffSet`,
// SerialFile.cpp:167). So the practical rule is exactly: bytes [132, EOF) are
// ciphered, bytes [0, 132) are plaintext. That is what `kBodyStart` encodes.
//
// ---------------------------------------------------------------------------
// HOW AN ENCRYPTED FILE IDENTIFIES ITSELF
// ---------------------------------------------------------------------------
//
// `Encryption_WLD` writes `EnPacketMap` over bytes 0..127, and `EnPacketMap` is
// `const char[128] = "Land.Man"` - the literal plus 119 NUL bytes
// (WLDCrypt.h:8). A plain file's first 8 bytes are `DxLandMan::FILEMARK`, which
// is `"LAND.MAN"` (DxLandManSaveLoad.cpp:32). So:
//
//     "LAND.MAN"  ->  plain
//     "Land.Man"  ->  encrypted
//
// and that comparison is the whole detection rule. `GLLandManSet.cpp:26` makes
// exactly this test, and sets `EMENCODE_WLD` when it matches.
//
// Note the FileID at offset 128 is NOT transformed by `Encryption_WLD` either -
// the function only writes `i < 128` and `i > 131` - so an encrypted file carries
// its version in the clear. Measured: all 14 encrypted `.wld` files read
// FileID 0x0114 undecrypted.
//
// ---------------------------------------------------------------------------
// NOT AN INVOLUTION
// ---------------------------------------------------------------------------
//
// `Encryption_WLD` does `b ^= K1; b ^= K2; b += (K1 ^ K2)`, i.e.
//
//     b = ((b ^ 0x10) mod 256) + 0x10
//
// which is NOT the inverse of the decrypt above. Feeding 0x0110 through encrypt
// then decrypt yields 0x0100, not 0x0110. The two functions disagree, and
// WORLD-ENTRY-002c §5 already flagged that reversibility is asserted from the
// source rather than measured. It is reproduced as found and NO round-trip test
// is written against it; the only thing this reader needs is the decrypt
// direction, which is what RAN's own load path uses.
//
// ---------------------------------------------------------------------------
// THREAD SAFETY
// ---------------------------------------------------------------------------
//
// No state at all - three constants and a byte loop over the caller's own
// buffer. `DecryptBody` mutates only what it is handed, so any number of Field
// workers may decrypt concurrently with no synchronisation. Legacy's version
// also has no state, but it runs inside `CSerialFile`, which owns a `FILE*` and
// is emphatically not shareable.

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Navigation
{
	namespace WldCrypt
	{
		// The file-type field width: `FILETYPESIZE` (basestream.h:17).
		constexpr std::size_t kFileTypeSize = 128;

		// `m_DefaultOffSet` after `ReadFileType`: the 128-byte file type plus the
		// 4-byte FileID. Everything from here on is ciphered.
		constexpr std::size_t kBodyStart = kFileTypeSize + 4;

		// `WLD_XOR_DATA` and `WLD_XOR_DATA2`, low byte (WLDCrypt.h:18-19).
		// The other three bytes of each constant never survive the BYTE store.
		constexpr std::uint8_t kXorKey1 = 0xAE;
		constexpr std::uint8_t kXorKey2 = 0xBE;

		// The folded add key: `kXorKey1 ^ kXorKey2`.
		constexpr std::uint8_t kSubtractKey = static_cast<std::uint8_t>(kXorKey1 ^ kXorKey2);
		static_assert(kSubtractKey == 0x10, "K1 ^ K2 must fold to 0x10 as a byte");

		// `DxLandMan::FILEMARK` (DxLandManSaveLoad.cpp:32).
		extern const char* const kPlainFileType;

		// `EnPacketMap` (WLDCrypt.h:8). Only its first 8 bytes are non-NUL.
		extern const char* const kEncryptedFileType;

		// True when the buffer's first 8 bytes are `EnPacketMap`.
		//
		// A short buffer is not encrypted; it is too small to be a WLD at all, and
		// the caller reports that separately.
		bool IsEncrypted(const std::uint8_t* data, std::size_t size) noexcept;

		// Applies the cipher in place to `data[kBodyStart, size)`.
		//
		// A no-op when `size <= kBodyStart`. Bytes before the body are left
		// untouched - see the header note on why that is correct rather than
		// convenient.
		void DecryptBody(std::uint8_t* data, std::size_t size) noexcept;

		inline void DecryptBody(std::vector<std::uint8_t>& data) noexcept
		{
			DecryptBody(data.data(), data.size());
		}
	}
}