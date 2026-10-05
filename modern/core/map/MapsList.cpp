#include "map/MapsList.h"

#include <cstring>
#include <limits>
#include <unordered_set>

namespace Modern::Map
{
	namespace
	{
		// ---- primitive widths, from `CSerialFile::operator>>` -------------------
		//
		// Each overload reads `sizeof` of its parameter type (SerialFile.cpp:475,
		// :486, :557, :579), so these are the wire widths, not choices.
		constexpr std::size_t kBoolBytes   = 1; // sizeof(bool) == 1 on MSVC
		constexpr std::size_t kWordBytes   = 2;
		constexpr std::size_t kDwordBytes  = 4;

		// `GLMapList::MAPLIST_MAX` (GLMapList.h:24). RAN never enforces it on load;
		// it is here as the ceiling that turns a corrupt record count into a
		// reported failure rather than a multi-gigabyte reserve.
		constexpr std::uint32_t kMaxRecords = 1000;

		// The shortest a record can be: version DWORD + bUsed + two length-prefixed
		// strings at their minimum + the two WORDs + the field-server DWORD +
		// 11 + 16 flag bytes. Only used to reject an absurd declared count before
		// looping; exact per-record bounds are enforced by the cursor.
		constexpr std::size_t kMinimumRecordBytes = 4 + 1 + 8 + 4 + 11 + 16;

		// ---- the version -> trailing flag count table ---------------------------
		//
		// `SMAPNODE_DATA::LOAD` (GLMapNode.cpp:133-438) is eight branches that share
		// a prefix and differ only in how many `bool`s follow the three strings.
		// Counting them out of the source:
		//
		//   0x0203  InstantMap,QBox,LunchBox,CPReset,PKMap,UIMapSelect,UIMapInfo,
		//           ClubPKRecord,OpenPrivateMarket,PartySparring,BlockTaxi,
		//           BlockFriendCard,BlockRecall,BlockTeleport,DisableSystemBuffs,
		//           BlockHPPotion                                     = 16
		//   0x0202  ... through DisableSystemBuffs                     = 15
		//   0x0201  ... through BlockTeleport                          = 14
		//   0x0200  ... through PartySparring                           = 10
		//   0x0103  InstantMap,QBox,LunchBox,CPReset,PKMap               = 5
		//   0x0102  InstantMap,QBox,LunchBox                            = 3
		//   0x0101  InstantMap,QBox,LunchBox                            = 3
		//   0x0100  InstantMap,QBox                                     = 2
		//
		// The shipped file is 0x0203 throughout. The rest are here because they are
		// one line each, and a parser that handled only the newest would report a
		// genuine version difference as a truncation.
		struct VersionLayout
		{
			std::uint32_t version;
			std::uint32_t trailingFlags;
		};

		constexpr VersionLayout kLayouts[] = {
		    {0x0203, 16}, {0x0202, 15}, {0x0201, 14}, {0x0200, 10},
		    {0x0103, 5},  {0x0102, 3},  {0x0101, 3},  {0x0100, 2},
		};

		const VersionLayout* FindLayout(std::uint32_t version) noexcept
		{
			for (const VersionLayout& layout : kLayouts)
			{
				if (layout.version == version)
				{
					return &layout;
				}
			}
			return nullptr;
		}

		// The flags between `bUsed` and the three strings, in `SMAPNODE_DATA`'s
		// declaration order (GLMapNode.cpp:74-88): PeaceZone, Commission, PKZone,
		// FreePK, ItemDrop, Move, Restart, PetActivity, DECEXP, VehicleActivity,
		// ClubBattleZone. Eleven.
		constexpr std::size_t kLeadingFlags = 11;

		// A bounds-checked little-endian cursor.
		//
		// Byte-wise on purpose, for the reason given in 002d's
		// `WldNavigationReader.cpp:43-53`: a `reinterpret_cast<const uint32_t*>` is
		// both an alignment hazard and a silent little-endian assumption. One
		// cursor for the whole decoder means "is this read inside the buffer" is
		// answered in exactly one place.
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

			bool ReadU16(std::uint16_t& out) noexcept
			{
				if (!Has(kWordBytes))
				{
					return false;
				}
				const std::uint8_t* p = m_data + m_position;
				out = static_cast<std::uint16_t>(static_cast<std::uint16_t>(p[0]) |
				                                 (static_cast<std::uint16_t>(p[1]) << 8));
				m_position += kWordBytes;
				return true;
			}

			bool ReadBool(bool& out) noexcept
			{
				if (!Has(kBoolBytes))
				{
					return false;
				}
				// `CSerialFile::operator>>(bool&)` reads `sizeof(bool)` = 1 byte and
				// assigns it. Any non-zero byte is therefore true, exactly as
				// legacy would see it - this does not normalise 0xFF to 1.
				out = m_data[m_position] != 0;
				m_position += kBoolBytes;
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

			// The byte at `offset`, valid only after `Has(offset + 1)` has been
			// established. Exposed so `DecodeString` can read a validated string
			// body without the cursor having to know what a string is.
			const std::uint8_t* At(std::size_t offset) const noexcept
			{
				return m_data + offset;
			}

		private:
			const std::uint8_t* m_data     = nullptr;
			std::size_t         m_size     = 0;
			std::size_t         m_position = 0;
		};

		bool Fail(MapsListError& error, MapsListStatus status, const char* stage,
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
		// Three things are checked, and legacy checks none of them:
		//
		//   * `byteCount == 0` is impossible - the writer always emits at least the
		//     NUL - so it means the length field was read from the wrong place.
		//   * `byteCount` must fit in the remaining bytes, BEFORE any allocation.
		//     Legacy does `new char[dwSize]` and `fread`s into it unconditionally
		//     (SerialFile.cpp:598-599), so a 0xFFFFFFFF length is a 4 GB allocation
		//     and then a short read whose result is ignored.
		//   * the final byte must be NUL. Legacy never checks this; it relies on the
		//     writer. Without the check a length/content mismatch would read past
		//     the string into the next field.
		bool DecodeString(Cursor& cursor, std::string& out, const char* stage,
		                  MapsListError& error)
		{
			const std::size_t at = cursor.Position();

			std::uint32_t byteCount = 0;
			if (!cursor.ReadU32(byteCount))
			{
				return Fail(error, MapsListStatus::Truncated, stage,
				            std::string("file ended before a string length in ") + stage, at);
			}

			if (byteCount == 0)
			{
				return Fail(error, MapsListStatus::Inconsistent, stage,
				            std::string("a string length of 0 in ") + stage +
				                "; RAN always stores at least the NUL",
				            at);
			}

			if (!cursor.Has(byteCount))
			{
				return Fail(error, MapsListStatus::Truncated, stage,
				            std::string("a string in ") + stage +
				                " claims more bytes than the buffer holds",
				            at);
			}

			if (cursor.Remaining() - byteCount < 1)
			{
				// No byte at all after the string: the buffer ends here, so the
				// record cannot be complete. Distinct from the case above, which is
				// about the string overrunning.
				return Fail(error, MapsListStatus::Truncated, stage,
				            std::string("file ends immediately after a string in ") + stage,
				            cursor.Position());
			}

			// Only NOW is the payload known to be inside the buffer, so this is the
			// first point at which reading it is safe.
			const std::uint8_t* bytes = cursor.At(cursor.Position());
			if (bytes[byteCount - 1] != 0)
			{
				return Fail(error, MapsListStatus::Inconsistent, stage,
				            std::string("a string in ") + stage +
				                " is not NUL-terminated; the length and the content disagree",
				            at);
			}

			out.assign(reinterpret_cast<const char*>(bytes), static_cast<std::size_t>(byteCount - 1));

			cursor.Skip(byteCount);
			return true;
		}
	}

	const char* const kMapsListFileType = "GLMAPS_LIST";

	const char* ToString(MapsListStatus status) noexcept
	{
		switch (status)
		{
		case MapsListStatus::Loaded:            return "Loaded";
		case MapsListStatus::InvalidFile:       return "InvalidFile";
		case MapsListStatus::UnsupportedFormat: return "UnsupportedFormat";
		case MapsListStatus::Truncated:         return "Truncated";
		case MapsListStatus::Inconsistent:      return "Inconsistent";
		}
		return "Unknown";
	}

	std::string MapsListError::Format(const std::string& fileName) const
	{
		std::string text = fileName + ": " + stage + ": " + detail + " [" +
		                   ToString(status) + "]";
		if (hasOffset)
		{
			text += " at offset " + std::to_string(offset);
		}
		return text;
	}

	bool DecodeMapsList(const std::uint8_t* data, std::size_t size, MapsListFile& out,
	                    MapsListError& error)
	{
		out   = MapsListFile{};
		error = MapsListError{};

		if (data == nullptr || size < ByteCrypt::kBodyStart)
		{
			return Fail(error, MapsListStatus::InvalidFile, "header",
			            "buffer is smaller than the 132-byte header", size, false);
		}

		// `GetFileType` copies `m_szFileType`, which was filled by a raw `fread` of
		// FILETYPESIZE bytes (SerialFile.cpp:55) and written by `SetFileType` via
		// `StringCchCopy` (SerialFile.cpp:40). So the tag is NUL-padded, not
		// space-padded - the shipped file is "GLMAPS_LIST" then NULs.
		{
			const std::size_t tagLength = std::strlen(kMapsListFileType);
			if (std::memcmp(data, kMapsListFileType, tagLength) != 0)
			{
				return Fail(error, MapsListStatus::InvalidFile, "file type",
				            "header does not start with GLMAPS_LIST", 0);
			}
		}

		// The FileID is read raw, before any cipher is chosen - the cipher selection
		// DEPENDS on it (GLMapList.cpp:135-141).
		{
			Cursor cursor(data, size);
			if (!cursor.Skip(ByteCrypt::kFileTypeSize) || !cursor.ReadU32(out.fileId))
			{
				return Fail(error, MapsListStatus::Truncated, "FileID",
				            "file ended before the FileID", ByteCrypt::kFileTypeSize);
			}
		}

		if (out.fileId < ByteCrypt::kNewestEncodeFileId)
		{
			// Below 0x0200 RAN selects `EMBYTECRYPT_OLD` or `EMBYTECRYPT_NONE`
			// (GLMapList.cpp:137-141), which are different tables. Decoding with the
			// newest table would produce a plausible-looking record stream from
			// garbage, so this reports instead.
			//
			// The shipped file is 0x0200. No such file exists in the deployed tree.
			return Fail(error, MapsListStatus::UnsupportedFormat, "FileID",
			            "FileID selects a byte-substitution table this reader does not carry",
			            ByteCrypt::kFileTypeSize);
		}

		// The whole file, as an owned copy: the caller's bytes are never modified,
		// because decoding is a destructive in-place transform.
		//
		// The WHOLE file, not a body-only slice, and that is not a detail.
		// `ByteCrypt::DecodeBody` transforms `data[kBodyStart, size)` - it encodes
		// "bytes [0, 132) are plaintext" in its own indexing, exactly as
		// `CSerialFile::ReadFileType` does (SerialFile.cpp:55-58). Handing it a
		// buffer that ALREADY starts at 132 leaves the first 132 body bytes - the
		// record count and the first few records - undecoded, and the file is then
		// parsed as garbage while looking like a format mistake. The slice happens
		// after decoding, never before.
		std::vector<std::uint8_t> file(data, data + size);
		ByteCrypt::DecodeBody(file);

		const std::uint8_t* body     = file.data() + ByteCrypt::kBodyStart;
		const std::size_t   bodySize = file.size() - ByteCrypt::kBodyStart;

		out.payloadBytes = bodySize;

		Cursor cursor(body, bodySize);

		if (!cursor.ReadU32(out.declaredCount))
		{
			return Fail(error, MapsListStatus::Truncated, "record count",
			            "file ended before the record count", 0);
		}

		if (out.declaredCount > kMaxRecords)
		{
			return Fail(error, MapsListStatus::Inconsistent, "record count",
			            "declared record count exceeds GLMapList::MAPLIST_MAX", 4);
		}

		// A count that could not possibly fit is refused before the loop, rather
		// than failing one record at a time and leaving a half-populated vector.
		if (static_cast<std::size_t>(out.declaredCount) * kMinimumRecordBytes > cursor.Remaining())
		{
			return Fail(error, MapsListStatus::Inconsistent, "record count",
			            "declared record count cannot fit in the remaining bytes", 4);
		}

		out.records.reserve(out.declaredCount);

		// `LoadMapsListFile` inserts into `std::map<DWORD, SMAPNODE_DATA>`, so a
		// duplicate `dwID` is dropped by `insert` rather than overwriting. Tracked
		// here so the count of records READ and the count DISTINCT can both be
		// reported. Measured on the shipped file: 99 records, 99 distinct ids, 0
		// duplicates.
		std::unordered_set<std::uint32_t> seenIds{};

		for (std::uint32_t i = 0; i < out.declaredCount; ++i)
		{
			const std::size_t recordStart = cursor.Position();

			MapRecord record;

			if (!cursor.ReadU32(record.recordVersion))
			{
				return Fail(error, MapsListStatus::Truncated, "record",
				            "file ended before a record version", recordStart);
			}

			const VersionLayout* layout = FindLayout(record.recordVersion);
			if (layout == nullptr)
			{
				// `SMAPNODE_DATA::LOAD` ends with `ErrorVersion` and returns FALSE
				// (GLMapNode.cpp:431-435); `LoadMapsListFile` then SKIPS this record
				// and continues (GLMapList.cpp:152-154). This reports rather than
				// skips, because a skip would leave the byte position wrong for every
				// record after it - there is no way to know how many bytes an
				// unrecognised version's flags occupy.
				return Fail(error, MapsListStatus::Inconsistent, "record version",
				            "record version has no SMAPNODE_DATA::LOAD branch",
				            recordStart);
			}

			record.flagCount = layout->trailingFlags;

			// ---- bUsed -------------------------------------------------------
			if (!cursor.ReadBool(record.used))
			{
				return Fail(error, MapsListStatus::Truncated, "bUsed",
				            "file ended inside a record's bUsed", cursor.Position());
			}

			// ---- strFile, sNativeID, dwFieldSID ------------------------------
			//
			// `strFile` is read FIRST, then the two WORDs of the native id, then the
			// field-server id (GLMapNode.cpp:141-145). Getting this order wrong is
			// the classic mistake in this format, and it produces a parse that
			// consumes the right number of bytes while being entirely wrong.
			if (!DecodeString(cursor, record.levelFileName, "strFile", error))
			{
				return false;
			}

			std::uint16_t mainId = 0;
			std::uint16_t subId  = 0;
			if (!cursor.ReadU16(mainId) || !cursor.ReadU16(subId))
			{
				return Fail(error, MapsListStatus::Truncated, "sNativeID",
				            "file ended inside a record's map id", cursor.Position());
			}
			record.identity = MapIdentity(mainId, subId);

			if (!cursor.ReadU32(record.fieldServerId))
			{
				return Fail(error, MapsListStatus::Truncated, "dwFieldSID",
				            "file ended inside a record's field server id", cursor.Position());
			}

			// ---- the eleven zone/flag bools -----------------------------------
			for (std::size_t f = 0; f < kLeadingFlags; ++f)
			{
				bool flag = false;
				if (!cursor.ReadBool(flag))
				{
					return Fail(error, MapsListStatus::Truncated, "map flags",
					            "file ended inside a record's flag block", cursor.Position());
				}
			}

			// ---- the three strings -------------------------------------------
			//
			// `strMapName`, `strBGM`, `strLoadingImageName` (GLMapNode.cpp:159-161).
			// All three are DECODED and length-validated; only the first is kept.
			// Skipping them by length without validating would let a corrupt BGM
			// string pass unnoticed and desynchronise every record after it.
			if (!DecodeString(cursor, record.mapName, "strMapName", error))
			{
				return false;
			}

			std::string discarded;
			if (!DecodeString(cursor, discarded, "strBGM", error))
			{
				return false;
			}
			if (!DecodeString(cursor, discarded, "strLoadingImageName", error))
			{
				return false;
			}

			// ---- the trailing flags ------------------------------------------
			for (std::uint32_t f = 0; f < record.flagCount; ++f)
			{
				bool flag = false;
				if (!cursor.ReadBool(flag))
				{
					return Fail(error, MapsListStatus::Truncated, "trailing flags",
					            "file ended inside a record's trailing flag block",
					            cursor.Position());
				}
			}

			// A duplicate id: legacy's `std::map::insert` drops it
			// (GLMapList.cpp:154). Recorded here and dropped identically, so the
			// registry's map count matches RAN's.
			if (!seenIds.insert(record.identity.Packed()).second)
			{
				continue;
			}

			out.records.push_back(std::move(record));
		}

		out.consumedBytes = cursor.Position();

		// Residue check. The shipped file consumes 11,362 of 11,362 body bytes.
		//
		// This is the assertion that earns the decoder its keep: a wrong field
		// order, a wrong bool width, or the wrong substitution table all produce
		// either bytes left over or a read past the end, and both are otherwise
		// easy to mistake for "it parsed".
		if (out.consumedBytes != out.payloadBytes)
		{
			return Fail(error, MapsListStatus::Inconsistent, "payload",
			            "the payload was not consumed exactly - field order, bool width or "
			            "substitution table is wrong",
			            out.consumedBytes);
		}

		error.status = MapsListStatus::Loaded;
		error.stage  = "mapslist";
		error.detail = "all records decoded and the payload consumed exactly";
		error.hasOffset = false;
		return true;
	}
}