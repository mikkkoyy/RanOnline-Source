#include "navigation/WldCrypt.h"

#include <cstring>

namespace Modern::Navigation
{
	namespace WldCrypt
	{
		// "LAND.MAN" - DxLandMan::FILEMARK (DxLandManSaveLoad.cpp:32). Only the
		// 8 significant bytes are compared, because `CSerialFile::GetFileType`
		// copies the whole 128-byte field into a buffer and then `strcmp`s it
		// (GLLandManSet.cpp:26) - and the field is NUL-padded after byte 8, so the
		// NULs are the terminator either way.
		const char* const kPlainFileType = "LAND.MAN";

		// "Land.Man" - EnPacketMap (WLDCrypt.h:8).
		const char* const kEncryptedFileType = "Land.Man";

		namespace
		{
			// The two markers are both 8 bytes. Comparing a fixed length rather
			// than using `strcmp` keeps this a bounds-checked read on a possibly
			// truncated buffer.
			constexpr std::size_t kFileTypeTagSize = 8;

			bool TagEquals(const std::uint8_t* data, const char* tag) noexcept
			{
				return std::memcmp(data, tag, kFileTypeTagSize) == 0;
			}
		}

		bool IsEncrypted(const std::uint8_t* data, std::size_t size) noexcept
		{
			if (data == nullptr || size < kFileTypeTagSize)
			{
				return false;
			}
			return TagEquals(data, kEncryptedFileType);
		}

		void DecryptBody(std::uint8_t* data, std::size_t size) noexcept
		{
			if (data == nullptr || size <= kBodyStart)
			{
				return;
			}

			// The loop body, verbatim from Decryption_WLD's `!bTool` branch, with
			// the BYTE truncation made explicit by the uint8_t type. Kept as three
			// separate steps rather than folded into
			// `((b - 0x10) ^ 0x10)` so the correspondence with the legacy source is
			// checkable line by line.
			std::uint8_t* body = data + kBodyStart;
			const std::size_t bodySize = size - kBodyStart;

			for (std::size_t i = 0; i < bodySize; ++i)
			{
				body[i] = static_cast<std::uint8_t>(body[i] - kSubtractKey);
				body[i] = static_cast<std::uint8_t>(body[i] ^ kXorKey1);
				body[i] = static_cast<std::uint8_t>(body[i] ^ kXorKey2);
			}
		}
	}
}