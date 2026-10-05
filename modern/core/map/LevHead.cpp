#include "map/LevHead.h"

#include <cstring>

namespace Modern::Map
{
	namespace
	{
		// ---- primitive widths, from `CSerialFile::operator>>` -------------------
		constexpr std::size_t kDwordBytes = 4;

		// `FILETYPESIZE` + the FileID DWORD. `m_DefaultOffSet` is `ftell` right after
		// `ReadFileType`, i.e. exactly this (SerialFile.cpp:58).
		constexpr std::size_t kHeadVersionOffset = ByteCrypt::kBodyStart;
		constexpr std::size_t kHeadSizeOffset    = ByteCrypt::kBodyStart + kDwordBytes;
		constexpr std::size_t kHeadPayloadOffset = ByteCrypt::kBodyStart + 2 * kDwordBytes;

		// A bounds-checked little-endian cursor over the payload only. Byte-wise for
		// the reason given in 002d's `WldNavigationReader.cpp:43-53`: a
		// `reinterpret_cast` is both an alignment hazard and a silent little-endian
		// assumption.
		class Cursor
		{
		public:
			Cursor(const std::uint8_t* data, std::size_t size) noexcept
			    : m_data(data), m_size(data == nullptr ? 0 : size)
			{
			}

			std::size_t Position() const noexcept { return m_position; }
			std::size_t Remaining() const noexcept
			{
				return m_position >= m_size ? 0 : m_size - m_position;
			}
			bool Has(std::size_t bytes) const noexcept { return Remaining() >= bytes; }

			bool ReadU32(std::uint32_t& out) noexcept
			{
				if (!Has(kDwordBytes))
				{
					return false;
				}
				const std::uint8_t* p = m_data + m_position;
				out = static_cast<std::uint32_t>(p[0]) |
				      (static_cast<std::uint32_t>(p[1]) << 8) |
				      (static_cast<std::uint32_t>(p[2]) << 16) |
				      (static_cast<std::uint32_t>(p[3]) << 24);
				m_position += kDwordBytes;
				return true;
			}

			bool Skip(std::size_t bytes) noexcept
			{
				if (!Has(bytes))
				{
					return false;
				}
				m_position += bytes;
				return true;
			}

			const std::uint8_t* At(std::size_t offset) const noexcept
			{
				return m_data + offset;
			}

		private:
			const std::uint8_t* m_data     = nullptr;
			std::size_t         m_size     = 0;
			std::size_t         m_position = 0;
		};

		bool Fail(LevHeadError& error, LevHeadStatus status, const char* stage,
		          const std::string& detail, std::size_t offset, bool hasOffset = true)
		{
			error.status    = status;
			error.stage     = stage;
			error.detail    = detail;
			error.offset    = offset;
			error.hasOffset = hasOffset;
			return false;
		}

		// A RAN string: `DWORD(byteCount)` then `byteCount` bytes including the NUL
		// (SerialFile.cpp:446-448 writes `length + 1`; :590-605 reads it back and
		// assigns through a `char*`, so the NUL is what terminates it).
		//
		// The three checks legacy omits are the same three `DecodeMapsList` makes,
		// for the same reasons - see MapsList.cpp:194-247.
		bool DecodeString(Cursor& cursor, std::string& out, const char* stage,
		                  LevHeadError& error)
		{
			const std::size_t at = cursor.Position();

			std::uint32_t byteCount = 0;
			if (!cursor.ReadU32(byteCount))
			{
				return Fail(error, LevHeadStatus::Truncated, stage,
				            std::string("file ended before a string length in ") + stage, at);
			}

			if (byteCount == 0)
			{
				return Fail(error, LevHeadStatus::Inconsistent, stage,
				            std::string("a string length of 0 in ") + stage +
				                "; RAN always stores at least the NUL",
				            at);
			}

			if (!cursor.Has(byteCount))
			{
				return Fail(error, LevHeadStatus::Truncated, stage,
				            std::string("a string in ") + stage +
				                " claims more bytes than the payload holds",
				            at);
			}

			if (cursor.Remaining() - byteCount < 1)
			{
				return Fail(error, LevHeadStatus::Truncated, stage,
				            std::string("the payload ends immediately after a string in ") +
				                stage,
				            cursor.Position());
			}

			const std::uint8_t* bytes = cursor.At(cursor.Position());
			if (bytes[byteCount - 1] != 0)
			{
				return Fail(error, LevHeadStatus::Inconsistent, stage,
				            std::string("a string in ") + stage +
				                " is not NUL-terminated; the length and the content disagree",
				            at);
			}

			out.assign(reinterpret_cast<const char*>(bytes), static_cast<std::size_t>(byteCount - 1));
			cursor.Skip(byteCount);
			return true;
		}

		// A `.lev` names its `.wld` with a bare file name. `GLLandMan` concatenates it
		// onto the map directory (`SUBPATH::MAP_FILE_ROOT`, SUBPATH.cpp), so a name
		// carrying a separator, a drive, or `..` would leave that directory - and a
		// loader that honoured it would read outside the configured asset root.
		//
		// The deployed names are all bare: `innerzone_01.wld`, `Ground_Lost.wld`,
		// `1city_sportzone.wld`. Refusing a path-shaped name is therefore a real
		// check, not a hypothetical one.
		bool IsBareFileName(const std::string& name) noexcept
		{
			if (name.empty() || name == "." || name == "..")
			{
				return false;
			}
			if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos)
			{
				return false;
			}
			if (name.find(':') != std::string::npos)
			{
				return false;
			}
			return true;
		}
	}

	const char* const kLevFileType = "glmap";

	const char* ToString(LevHeadStatus status) noexcept
	{
		switch (status)
		{
		case LevHeadStatus::Loaded:                return "Loaded";
		case LevHeadStatus::InvalidFile:           return "InvalidFile";
		case LevHeadStatus::UnsupportedFormat:     return "UnsupportedFormat";
		case LevHeadStatus::UnsupportedHeadVersion: return "UnsupportedHeadVersion";
		case LevHeadStatus::Truncated:             return "Truncated";
		case LevHeadStatus::Inconsistent:          return "Inconsistent";
		}
		return "Unknown";
	}

	std::string LevHeadError::Format(const std::string& fileName) const
	{
		std::string text = fileName + ": " + stage + ": " + detail + " [" +
		                   ToString(status) + "]";
		if (hasOffset)
		{
			text += " at offset " + std::to_string(offset);
		}
		return text;
	}

	bool DecodeLevHead(const std::uint8_t* data, std::size_t size, LevHead& out,
	                   LevHeadError& error)
	{
		out   = LevHead{};
		error = LevHeadError{};

		if (data == nullptr || size < kHeadPayloadOffset)
		{
			return Fail(error, LevHeadStatus::InvalidFile, "header",
			            "buffer is smaller than the 132-byte header", size, false);
		}

		// `SetFileType` uses `StringCchCopy` (SerialFile.cpp:40) over a field filled by
		// a raw `fread` of FILETYPESIZE bytes (:55), so the tag is NUL-padded.
		{
			const std::size_t tagLength = std::strlen(kLevFileType);
			if (std::memcmp(data, kLevFileType, tagLength) != 0)
			{
				return Fail(error, LevHeadStatus::InvalidFile, "file type",
				            "header does not start with glmap", 0);
			}
		}

		// Read raw, before the cipher is chosen - the choice depends on it
		// (GLLevelFileSaveLoad.cpp:183-187).
		{
			Cursor header(data, size);
			std::uint32_t fileId = 0;
			if (!header.Skip(ByteCrypt::kFileTypeSize) || !header.ReadU32(fileId))
			{
				return Fail(error, LevHeadStatus::Truncated, "FileID",
				            "file ended before the FileID", ByteCrypt::kFileTypeSize);
			}
			out.fileId = fileId;
		}

		if (out.fileId < ByteCrypt::kNewestEncodeFileId)
		{
			// Below 0x0200 RAN selects `EMBYTECRYPT_OLD` or `EMBYTECRYPT_NONE`
			// (GLLevelFileSaveLoad.cpp:183-187), which are different tables. Decoding
			// with the newest table would produce a plausible-looking filename from
			// garbage, so this reports instead.
			return Fail(error, LevHeadStatus::UnsupportedFormat, "FileID",
			            "FileID selects a byte-substitution table this reader does not carry",
			            ByteCrypt::kFileTypeSize);
		}

		// The whole file, as an owned copy. The caller's bytes are never modified:
		// decoding is a destructive in-place transform.
		//
		// The WHOLE file, not a body-only slice, because `ByteCrypt::DecodeBody`
		// transforms `data[kBodyStart, size)` - it encodes "bytes [0, 132) are
		// plaintext" in its own indexing, exactly as `CSerialFile::ReadFileType` does
		// (SerialFile.cpp:55-58). A buffer that already started at 132 would leave the
		// first 132 body bytes - including the head version - undecoded, and the head
		// would then be read as garbage. The slice happens after decoding.
		std::vector<std::uint8_t> file(data, data + size);
		ByteCrypt::DecodeBody(file);

		const std::size_t bodySize = file.size() - ByteCrypt::kBodyStart;
		if (bodySize < 2 * kDwordBytes)
		{
			return Fail(error, LevHeadStatus::Truncated, "head",
			            "the body holds less than the head version and payload size", 0);
		}

		// `SLEVEL_HEAD::LOAD` reads `dwVer` then `dwSize` (GLLevelHead.cpp:10-12),
		// and `LOAD_0102` measures `dwRead` from immediately AFTER `dwSize`
		// (GLLevelHead.cpp:50), so `dwSize` covers the payload alone.
		std::uint32_t headVersion = 0;
		std::uint32_t declared    = 0;
		{
			Cursor bodyCursor(file.data() + ByteCrypt::kBodyStart, bodySize);
			if (!bodyCursor.ReadU32(headVersion) || !bodyCursor.ReadU32(declared))
			{
				return Fail(error, LevHeadStatus::Truncated, "head",
				            "the body ended before the head version and payload size", 0);
			}
		}
		out.headVersion          = headVersion;
		out.declaredPayloadBytes = declared;

		if (headVersion != 0x0102 && headVersion != 0x0101)
		{
			// `SLEVEL_HEAD::LOAD` ends with `ErrorVersion` and then skips `dwSize`
			// bytes (GLLevelHead.cpp:23-25). Skipping is not useful here: the only
			// thing anyone wants from a `.lev` is the WLD name, and a head version
			// whose field order is unknown cannot yield one safely.
			return Fail(error, LevHeadStatus::UnsupportedHeadVersion, "head version",
			            "head version has no SLEVEL_HEAD::LOAD branch",
			            kHeadVersionOffset - ByteCrypt::kBodyStart);
		}

		const std::size_t available = bodySize - 2 * kDwordBytes;

		// The declared size must be inside the buffer BEFORE anything is parsed from
		// it, so no read below can be fed by an attacker-chosen length.
		if (declared > available)
		{
			return Fail(error, LevHeadStatus::Truncated, "head size",
			            "the declared head payload size is larger than the bytes present",
			            kHeadSizeOffset - ByteCrypt::kBodyStart);
		}

		// Parsed over the declared span only. If the file carries trailing blocks -
		// and a real `.lev` does: `SLEVEL_REQUIRE`, the mob spawners, the gates, the
		// weather - they are outside `dwSize` and are simply not looked at.
		Cursor   payload(file.data() + ByteCrypt::kBodyStart + 2 * kDwordBytes,
		                 static_cast<std::size_t>(declared));
		LevHead& head = out;

		// `LOAD_0102`: strMapName, strWldFile, division, bright (GLLevelHead.cpp:50-55).
		// `LOAD_0101`: strWldFile, strMapName, bright, division (:33-40).
		//
		// The order is the only difference. Reading 0x0102 with the 0x0101 order yields
		// a plausible-looking filename naming the wrong file, which is the worst
		// possible failure for this field.
		if (headVersion == 0x0102)
		{
			if (!DecodeString(payload, head.mapName, "strMapName", error) ||
			    !DecodeString(payload, head.wldFileName, "strWldFile", error))
			{
				return false;
			}
			if (!payload.ReadU32(head.division) || !payload.ReadU32(head.brightness))
			{
				return Fail(error, LevHeadStatus::Truncated, "division/brightness",
				            "the head payload ended before the division and brightness",
				            payload.Position());
			}
		}
		else
		{
			if (!DecodeString(payload, head.wldFileName, "strWldFile", error) ||
			    !DecodeString(payload, head.mapName, "strMapName", error))
			{
				return false;
			}
			if (!payload.ReadU32(head.brightness) || !payload.ReadU32(head.division))
			{
				return Fail(error, LevHeadStatus::Truncated, "division/brightness",
				            "the head payload ended before the division and brightness",
				            payload.Position());
			}
		}

		out.consumedPayloadBytes = payload.Position();

		// `LOAD_0102` compares `dwRead` with `dwSIZE` and only MsgBoxes on a mismatch
		// (GLLevelHead.cpp:61-62) - a diagnostic, not a validation. Here it is a
		// failure: a disagreement means the field order or the size is wrong, and
		// reporting it beats silently trusting either number.
		if (out.consumedPayloadBytes != out.declaredPayloadBytes)
		{
			return Fail(error, LevHeadStatus::Inconsistent, "head size",
			            "the declared head payload size disagrees with the bytes read",
			            out.consumedPayloadBytes);
		}

		if (!IsBareFileName(head.wldFileName))
		{
			return Fail(error, LevHeadStatus::Inconsistent, "strWldFile",
			            "m_strWldFile is empty or is not a bare file name; it would leave "
			            "the map directory",
			            0);
		}

		error.status    = LevHeadStatus::Loaded;
		error.stage     = "lev head";
		error.detail    = "SLEVEL_HEAD decoded and the declared payload size matched";
		error.hasOffset = false;
		return true;
	}
}
