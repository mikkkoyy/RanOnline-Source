#include "navigation/WldNavigationReader.h"

#include "navigation/WldCrypt.h"

#include <cstring>
#include <limits>

namespace Modern::Navigation
{
	namespace
	{
		// ---- the fixed header -------------------------------------------------
		//
		// Every one of these is a measured or source-derived offset; see the header
		// note. They are named rather than inlined so a test can assert them.

		constexpr std::size_t kFileTypeOffset = 0;
		constexpr std::size_t kFileTypeBytes  = 128; // FILETYPESIZE
		constexpr std::size_t kFileIdOffset   = 128;
		constexpr std::size_t kMapIdOffset    = 132;
		constexpr std::size_t kMapNameOffset  = 136;
		constexpr std::size_t kMapNameBytes   = 128; // MAXLANDNAME, DxLandDef.h:16
		constexpr std::size_t kFileMarkOffset = 264;

		// `SLAND_FILEMARK` is four DWORDs whatever the version - only their ORDER
		// differs between 0x0101 and 0x0100.
		constexpr std::size_t kFileMarkPayloadBytes = 16;
		constexpr std::size_t kFileMarkHeaderBytes  = 8; // version + payload size

		constexpr std::uint32_t kFileMarkVersionCurrent = 0x0101; // SLAND_FILEMARK::VERSION
		constexpr std::uint32_t kFileMarkVersion100     = 0x0100; // SLAND_FILEMARK_100

		constexpr std::size_t kBytesExist  = 4;
		constexpr std::size_t kBytesCount  = 4;
		constexpr std::size_t kBytesVertex = 12; // sizeof(D3DXVECTOR3)

		// One link entry: a `BOOL` presence flag and, when set, a `DWORD` id
		// (NavigationSaveLoad.cpp:33-46). The BOOL is 4 bytes because that is the
		// type at the declaration site (NavigationSaveLoad.cpp:76).
		constexpr std::size_t kLinkFlagBytes = 4;
		constexpr std::size_t kLinkIdBytes   = 4;

		// ---- a bounds-checked cursor ------------------------------------------
		//
		// The whole reason this type exists. Every field read in this file goes
		// through it, so there is exactly one place where "is this read inside the
		// buffer" is answered, and the answer cannot be forgotten.
		//
		// It reads and writes little-endian values BYTE BY BYTE rather than
		// casting the buffer. A `reinterpret_cast<const uint32_t*>(buffer + n)`
		// would be both an alignment fault on any buffer that is not 4-byte
		// aligned and an implicit endianness assumption; the explicit version is
		// three lines and is correct on any host.
		class ByteCursor
		{
		public:
			ByteCursor(const std::uint8_t* data, std::size_t size) noexcept
			    : m_data(data), m_size(data == nullptr ? 0 : size), m_position(0)
			{
			}

			std::size_t Position() const noexcept { return m_position; }
			std::size_t Size() const noexcept { return m_size; }
			std::size_t Remaining() const noexcept { return m_position >= m_size ? 0 : m_size - m_position; }

			bool Has(std::size_t bytes) const noexcept
			{
				return Remaining() >= bytes;
			}

			bool ReadU32(std::uint32_t& out) noexcept
			{
				if (!Has(kBytesCount))
				{
					return false;
				}
				out = ReadU32At(m_position);
				m_position += kBytesCount;
				return true;
			}

			bool ReadI32(std::int32_t& out) noexcept
			{
				std::uint32_t raw = 0;
				if (!ReadU32(raw))
				{
					return false;
				}
				// The file stores a signed int (`int m_nNaviVertex`); the bit pattern
				// is reinterpreted rather than converted, so a negative count stays
				// negative instead of wrapping to a huge positive one.
				std::memcpy(&out, &raw, sizeof(out));
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

			// Absolute seek. An offset past the end FAILS rather than clamping,
			// because a mark pointing outside the file is a real defect and silently
			// reading at the end would turn it into a plausible-looking empty block.
			bool SeekTo(std::size_t offset) noexcept
			{
				if (offset > m_size)
				{
					return false;
				}
				m_position = offset;
				return true;
			}

		private:
			std::uint32_t ReadU32At(std::size_t offset) const noexcept
			{
				const std::uint8_t* p = m_data + offset;
				return static_cast<std::uint32_t>(p[0]) |
				       (static_cast<std::uint32_t>(p[1]) << 8) |
				       (static_cast<std::uint32_t>(p[2]) << 16) |
				       (static_cast<std::uint32_t>(p[3]) << 24);
			}

			const std::uint8_t* m_data      = nullptr;
			std::size_t         m_size      = 0;
			std::size_t         m_position  = 0;
		};

		// Multiplication that refuses to overflow.
		//
		// §13: `cellCount * 188` and `vertexCount * 12` come straight out of the
		// file, so a four-gigabyte count would otherwise ask for a 200 GB
		// allocation before any bounds check ran.
		bool CheckedMultiply(std::size_t lhs, std::size_t rhs, std::size_t& out) noexcept
		{
			if (lhs != 0 && rhs > std::numeric_limits<std::size_t>::max() / lhs)
			{
				return false;
			}
			out = lhs * rhs;
			return true;
		}

		bool CheckedAdd(std::size_t lhs, std::size_t rhs, std::size_t& out) noexcept
		{
			if (rhs > std::numeric_limits<std::size_t>::max() - lhs)
			{
				return false;
			}
			out = lhs + rhs;
			return true;
		}

		// Three little-endian floats, byte by byte.
		//
		// The obvious `reinterpret_cast<const Vector3*>(data + base)` is wrong twice
		// over: the offset may not be 4-byte aligned, and it would silently assume
		// the host is little-endian. This is the file's only float read.
		Vector3 ReadFloat3(const std::uint8_t* data, std::size_t base) noexcept
		{
			float components[3] = {0.0f, 0.0f, 0.0f};

			for (std::size_t component = 0; component < 3; ++component)
			{
				std::uint32_t bits = 0;
				for (std::size_t i = 0; i < 4; ++i)
				{
					bits |= static_cast<std::uint32_t>(data[base + component * 4 + i]) << (8 * i);
				}
				std::memcpy(&components[component], &bits, sizeof(float));
			}

			return Vector3{components[0], components[1], components[2]};
		}

		void Fail(WldNavigationError& error, NavigationLoadStatus status, const char* stage,
		          const char* detail, std::size_t offset, bool hasOffset = true)
		{
			error.status    = status;
			error.stage     = stage;
			error.detail    = detail;
			error.offset    = offset;
			error.hasOffset = hasOffset;
		}
	}

	const char* ToString(NavigationLoadStatus status) noexcept
	{
		switch (status)
		{
		case NavigationLoadStatus::Loaded: return "Loaded";
		case NavigationLoadStatus::NoNavigation: return "NoNavigation";
		case NavigationLoadStatus::InvalidFile: return "InvalidFile";
		case NavigationLoadStatus::UnsupportedFormat: return "UnsupportedFormat";
		case NavigationLoadStatus::DecryptionFailed: return "DecryptionFailed";
		case NavigationLoadStatus::Truncated: return "Truncated";
		case NavigationLoadStatus::Inconsistent: return "Inconsistent";
		}
		return "Unknown";
	}

	std::string WldNavigationError::Format(const std::string& fileName) const
	{
		std::string text = fileName + ": " + stage + ": " + detail + " [" +
		                   ToString(status) + "]";
		if (hasOffset)
		{
			text += " at offset " + std::to_string(offset);
		}
		return text;
	}

	bool IsSupportedLandManVersion(std::uint32_t version) noexcept
	{
		// The list, verbatim, from DxLandManSaveLoad.cpp:3167-3181.
		switch (version)
		{
		case 0x0108:
		case 0x0109:
		case 0x0110:
		case 0x0111:
		case 0x0112:
		case 0x0113:
		case 0x0114:
		case 0x0115:
		case 0x0116:
		case 0x0117:
		case 0x0119:
		// `DxLandMan::VERSION_WLD` (DxLandMan.h:63).
		case 0x0200:
			return true;
		default:
			return false;
		}
	}

	bool ParseWldNavigation(const std::uint8_t* data, std::size_t size, WldNavigationData& out,
	                        WldNavigationError& error)
	{
		out     = WldNavigationData{};
		error   = WldNavigationError{};
		out.encrypted = WldCrypt::IsEncrypted(data, size);

		// ---- file type --------------------------------------------------------
		if (data == nullptr || size < WldCrypt::kBodyStart)
		{
			Fail(error, NavigationLoadStatus::InvalidFile, "header", "file is smaller than the 132-byte header",
			     size, false);
			return false;
		}

		{
			const char* const expected = out.encrypted ? WldCrypt::kEncryptedFileType
			                                            : WldCrypt::kPlainFileType;
			if (std::memcmp(data + kFileTypeOffset, expected, 8) != 0)
			{
				Fail(error, NavigationLoadStatus::InvalidFile, "file type",
				     "first 8 bytes are neither LAND.MAN nor Land.Man", kFileTypeOffset);
				return false;
			}
		}

		// ---- FileID -----------------------------------------------------------
		//
		// Raw, never decrypted: `Encryption_WLD` writes only `i < 128` and
		// `i > 131`, so bytes 128..131 are plaintext in an encrypted file too.
		{
			ByteCursor cursor(data, size);
			if (!cursor.SeekTo(kFileIdOffset) || !cursor.ReadU32(out.fileId))
			{
				Fail(error, NavigationLoadStatus::Truncated, "FileID", "file ended before the FileID",
				     kFileIdOffset);
				return false;
			}
		}

		if (!IsSupportedLandManVersion(out.fileId))
		{
			// GLLandManSet.cpp:41-45 refuses here, and so does this. `square_rd.wld`
			// is the shipped example: FileID 0x0202.
			Fail(error, NavigationLoadStatus::InvalidFile, "FileID",
			     "FileID is not in NSLANDMAN_SUPPORT::IsLandManSupported", kFileIdOffset);
			return false;
		}

		// ---- map id -----------------------------------------------------------
		//
		// Read and REPORTED, never validated. WORLD-ENTRY-002c measured it as 0 in
		// all 87 files: the `.wld` carries no map identity.
		ByteCursor cursor(data, size);

		if (!cursor.SeekTo(kMapIdOffset) || !cursor.ReadU32(out.mapId))
		{
			Fail(error, NavigationLoadStatus::Truncated, "map id", "file ended before the map id",
			     kMapIdOffset);
			return false;
		}

		// The map name is skipped, not examined. It is read by
		// GLLandManSet.cpp:57-58 and used for diagnostics only, and for encrypted
		// files it decrypts to all zeroes - a real observation, not a defect:
		// `Encryption_WLD` maps an empty name to 0x20 filler.
		if (!cursor.Skip(kMapNameBytes))
		{
			Fail(error, NavigationLoadStatus::Truncated, "map name", "file ended inside the map name",
			     kMapNameOffset);
			return false;
		}

		// ---- file mark --------------------------------------------------------
		{
			if (!cursor.SeekTo(kFileMarkOffset))
			{
				Fail(error, NavigationLoadStatus::Truncated, "file mark", "file ended before the file mark",
				     kFileMarkOffset);
				return false;
			}

			if (!cursor.ReadU32(out.fileMarkVersion) || !cursor.ReadU32(out.fileMarkPayloadBytes))
			{
				Fail(error, NavigationLoadStatus::Truncated, "file mark",
				     "file ended inside the file mark header", kFileMarkOffset);
				return false;
			}

			// `SLAND_FILEMARK::LoadSet` (DxLandDef.cpp:29-49) reads a
			// `sizeof(SLAND_FILEMARK_...)` block and, for an unrecognised version,
			// skips `dwSize` bytes past the header and gives up. The skip is a
			// diagnostic aid, not a recovery: navigation cannot be located without
			// dwNAVI_MARK, so this reports rather than continues.
			const bool known = out.fileMarkVersion == kFileMarkVersionCurrent ||
			                   out.fileMarkVersion == kFileMarkVersion100;
			if (!known)
			{
				Fail(error, NavigationLoadStatus::UnsupportedFormat, "file mark",
				     "file mark version is neither 0x0101 nor 0x0100", kFileMarkOffset);
				return false;
			}

			// The payload is `sizeof` of the version's struct, and both are four
			// DWORDs. A `dwSize` that disagrees is reported rather than trusted:
			// the block was always read at the fixed width.
			if (out.fileMarkPayloadBytes != kFileMarkPayloadBytes)
			{
				Fail(error, NavigationLoadStatus::Inconsistent, "file mark",
				     "file mark payload size is not 16", kFileMarkOffset + 4);
				return false;
			}

			if (!cursor.Has(kFileMarkPayloadBytes))
			{
				Fail(error, NavigationLoadStatus::Truncated, "file mark",
				     "file ended inside the file mark payload", cursor.Position());
				return false;
			}

			std::uint32_t first  = 0;
			std::uint32_t second = 0;
			std::uint32_t third  = 0;
			std::uint32_t fourth = 0;
			cursor.ReadU32(first);
			cursor.ReadU32(second);
			cursor.ReadU32(third);
			cursor.ReadU32(fourth);

			// dwNAVI_MARK is first in BOTH layouts - which is why a reader that
			// only wanted navigation could read one DWORD and stop. The remaining
			// three are assigned per version because the ORDER differs.
			out.naviMark = first;
			if (out.fileMarkVersion == kFileMarkVersionCurrent)
			{
				out.weatherMark = second;
				out.gateMark    = third;
				out.collMark    = fourth;
			}
			else
			{
				out.gateMark    = second;
				out.collMark    = third;
				out.weatherMark = fourth;
			}
		}

		// ---- navigation marker -------------------------------------------------
		//
		// `132 + dwNAVI_MARK`, and then the `bExist` DWORD. See the header.
		std::size_t naviBlock = 0;
		if (!CheckedAdd(WldCrypt::kBodyStart, static_cast<std::size_t>(out.naviMark), naviBlock))
		{
			Fail(error, NavigationLoadStatus::Inconsistent, "navigation marker",
			     "dwNAVI_MARK is large enough to overflow the file offset", 0, false);
			return false;
		}

		if (naviBlock > size)
		{
			Fail(error, NavigationLoadStatus::Inconsistent, "navigation marker",
			     "dwNAVI_MARK points past the end of the file", naviBlock);
			return false;
		}

		// ---- bExist -------------------------------------------------------------
		if (!cursor.SeekTo(naviBlock) || !cursor.Has(kBytesExist))
		{
			Fail(error, NavigationLoadStatus::Truncated, "bExist",
			     "file ended before the navigation existence flag", naviBlock);
			return false;
		}

		std::uint32_t exists = 0;
		cursor.ReadU32(exists);

		if (exists == 0)
		{
			// Not a failure. 9 of the 87 shipped files are like this: the
			// character-select and login maps, which RAN never walks.
			error.status = NavigationLoadStatus::NoNavigation;
			error.stage  = "bExist";
			error.detail = "file has no navigation mesh (bExist == 0)";
			error.offset = naviBlock;
			return true;
		}

		if (exists != 1)
		{
			// The format stores a BOOL, so anything else means the bytes at the mark
			// are not a BOOL - i.e. the navigation offset is wrong or the file is
			// still ciphered. Refusing is the whole point of the marker check.
			Fail(error, NavigationLoadStatus::Inconsistent, "bExist",
			     "bExist is neither 0 nor 1, so the navigation marker is wrong", naviBlock);
			return false;
		}

		// ---- vertices ------------------------------------------------------------
		std::int32_t vertexCount = 0;
		if (!cursor.ReadI32(vertexCount))
		{
			Fail(error, NavigationLoadStatus::Truncated, "vertex count",
			     "file ended before the navigation vertex count", cursor.Position());
			return false;
		}

		if (vertexCount < 0)
		{
			Fail(error, NavigationLoadStatus::Inconsistent, "vertex count",
			     "vertex count is negative", cursor.Position() - kBytesCount);
			return false;
		}

		std::size_t vertexBytes = 0;
		if (!CheckedMultiply(static_cast<std::size_t>(vertexCount), kBytesVertex, vertexBytes) ||
		    !cursor.Has(vertexBytes))
		{
			Fail(error, NavigationLoadStatus::Truncated, "vertex block",
			     "vertex block does not fit in the remaining bytes", cursor.Position());
			return false;
		}

		out.vertices.reserve(static_cast<std::size_t>(vertexCount));
		for (std::int32_t i = 0; i < vertexCount; ++i)
		{
			// Read through the same three-byte-wise path as everything else, so
			// there is no `reinterpret_cast<Vector3*>` anywhere in this file.
			const std::size_t base = cursor.Position();
			out.vertices.push_back(ReadFloat3(data, base));
			cursor.Skip(kBytesVertex);
		}

		// ---- cell records ---------------------------------------------------------
		std::uint32_t cellCount = 0;
		if (!cursor.ReadU32(cellCount))
		{
			Fail(error, NavigationLoadStatus::Truncated, "cell count",
			     "file ended before the navigation cell count", cursor.Position());
			return false;
		}

		if (cellCount == 0)
		{
			// `bExist == 1` with no cells. `NavigationMesh::LoadFile` would build an
			// empty mesh and `MakeAABBTree` would then dereference a null root
			// (navagationtree.cpp:489-500). A mesh with no cells cannot answer a
			// single query, so this is reported rather than built.
			Fail(error, NavigationLoadStatus::Inconsistent, "cell count",
			     "bExist is 1 but the cell count is 0", cursor.Position() - kBytesCount);
			return false;
		}

		std::size_t cellBytes = 0;
		if (!CheckedMultiply(static_cast<std::size_t>(cellCount),
		                     NavigationCell::kCellRecordBytes, cellBytes) ||
		    !cursor.Has(cellBytes))
		{
			Fail(error, NavigationLoadStatus::Truncated, "cell records",
			     "cell record block does not fit in the remaining bytes", cursor.Position());
			return false;
		}

		out.cellRecords.assign(data + cursor.Position(), data + cursor.Position() + cellBytes);
		const std::size_t recordsAt = cursor.Position();
		cursor.Skip(cellBytes);

		// Every cell's baked id must equal its index. `NavigationMesh::GetCell`
		// (navigationmesh.h:141-144) and the id comparisons inside
		// `NavigationCell::QueryForPath` (navigationcell.cpp:339-354) both depend
		// on it, and WORLD-ENTRY-002c measured a wrong record stride producing a
		// mismatch on 423 of 424 cells - so this is checked, not assumed.
		for (std::size_t i = 0; i < cellCount; ++i)
		{
			std::uint32_t id = 0;
			ByteCursor     recordCursor(out.cellRecords.data() + i * NavigationCell::kCellRecordBytes,
			                            NavigationCell::kCellRecordBytes);
			recordCursor.ReadU32(id);

			if (id != i)
			{
				Fail(error, NavigationLoadStatus::Inconsistent, "cell records",
				     "a cell's baked id does not equal its index", recordsAt, false);
				return false;
			}

			for (int v = 0; v < 3; ++v)
			{
				std::uint32_t vertexIndex = 0;
				recordCursor.SeekTo(NavigationCell::kOffsetVertex +
				                    static_cast<std::size_t>(v) * 4);
				recordCursor.ReadU32(vertexIndex);

				if (vertexIndex >= static_cast<std::uint32_t>(out.vertices.size()))
				{
					Fail(error, NavigationLoadStatus::Inconsistent, "cell records",
					     "a cell references a vertex that does not exist", recordsAt, false);
					return false;
				}
			}
		}

		// ---- cell links --------------------------------------------------------------
		//
		// Three per cell, in SIDE_AB, SIDE_BC, SIDE_CA order, each a `BOOL` and -
		// when set - a `DWORD` cell id (NavigationSaveLoad.cpp:28-46).
		std::size_t linkEntries = 0;
		if (!CheckedMultiply(static_cast<std::size_t>(cellCount), 3, linkEntries))
		{
			Fail(error, NavigationLoadStatus::Inconsistent, "cell links",
			     "cell count is large enough to overflow the link table", cursor.Position(), false);
			return false;
		}

	out.linkIds.assign(linkEntries, NavigationCell::kNoLink);
	for (std::size_t i = 0; i < linkEntries; ++i)
	{
		std::uint32_t present = 0;
		if (!cursor.ReadU32(present))
		{
			Fail(error, NavigationLoadStatus::Truncated, "cell links",
			     "file ended inside the link table", cursor.Position());
		return false;
		}

		if (present == 0)
		{
			// A solid edge. Recorded as kNoLink rather than skipped: the table is
			// positional, and the next entry is still the next entry.
			continue;
		}

		if (present != 1)
		{
			// The format stores a `BOOL`. Anything else means these bytes are not a
			// BOOL - either the navigation offset is wrong or the body is still
			// ciphered, which is what a single pass on the double-encrypted fixture
			// produces.
			Fail(error, NavigationLoadStatus::Inconsistent, "cell links",
			     "a link presence flag is neither 0 nor 1", cursor.Position() - kBytesCount);
		return false;
		}

		std::uint32_t linkId = 0;
		if (!cursor.ReadU32(linkId))
		{
			Fail(error, NavigationLoadStatus::Truncated, "cell links",
			     "file ended inside a link id", cursor.Position());
		return false;
		}

		if (linkId >= cellCount)
		{
			// Legacy hands this straight to `GetCell`, which tests `size() < index`
			// and then calls `.at(index)` (navigationmesh.h:141-144) - so an id
			// equal to the count throws and a larger one reads out of bounds. 002c
			// measured zero such links across all 77 meshes; this turns a would-be
			// crash into a controlled failure rather than depending on that.
			Fail(error, NavigationLoadStatus::Inconsistent, "cell links",
			     "a link id names a cell that does not exist", cursor.Position() - kBytesCount);
		return false;
		}

		out.linkIds[i] = linkId;
	}

	error.status    = NavigationLoadStatus::Loaded;
	error.stage     = "navigation";
	error.detail    = "navigation block decoded";
	error.hasOffset = false;
	return true;
}
}
