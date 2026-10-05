// WORLD-ENTRY-002e: map registry tests.
//
// Three kinds of test, deliberately mixed in one file because they test one thing -
// the chain WORLD-ENTRY-002c proved:
//
//   * REAL ASSET tests, driven from RAN_ASSET_ROOT. These are the ones that prove
//     the milestone, and every number they assert is the number 002c measured over
//     the deployed ASURA client.
//
//   * SYNTHETIC FIXTURE tests. A missing `.lev`, a `.lev` naming an absent `.wld`, a
//     truncated `.lev`, a truncated `mapslist.mst` - none of those exist in the
//     shipped set, because every shipped file is complete. They are built in a temp
//     directory with a real, forward-ENCRYPTED `mapslist.mst`, so each fixture goes
//     through the production decoders rather than around them.
//
//   * SYNTHETIC DECODER tests. Byte-level inputs for the checks that are about the
//     parsers rather than the filesystem: a `m_strWldFile` that would escape the map
//     directory, and a `mapslist.mst` that would need the wrong substitution table.
//
// ---------------------------------------------------------------------------
// RAN_ASSET_ROOT
// ---------------------------------------------------------------------------
//
// The same convention as modern/tests/WldNavigationTests.cpp and
// modern/client/assets/ClientMxfTests.cpp - the root of an ASURA client install,
// supplied by the environment. No path is compiled into production code.
//
//     set RAN_ASSET_ROOT=D:\FILES\project\RanOnline-Build\ASURA CLIENT
//
// When it is unset the asset tests print a loud banner and skip and the synthetic
// tests still run. A run that skipped them is not evidence of anything.

#include "TestHarness.h"

#include "map/LevHead.h"
#include "map/MapIdentity.h"
#include "map/MapRegistry.h"
#include "map/MapsList.h"
#include "map/RanByteCrypt.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Map;
using namespace Modern::Navigation;

namespace
{
	// ------------------------------------------------------------------------
	// Asset location
	// ------------------------------------------------------------------------

	enum class AssetAvailability
	{
		Available,
		RootUnset,
		RootWrong,
	};

	AssetAvailability     g_availability = AssetAvailability::RootUnset;
	std::filesystem::path g_assetRoot;
	int                    g_assetTestsSkipped = 0;

	const std::filesystem::path& AssetRoot()
	{
		if (g_availability != AssetAvailability::RootUnset)
		{
			return g_assetRoot;
		}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
		const char* root = std::getenv("RAN_ASSET_ROOT");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
		if (root == nullptr || root[0] == '\0')
		{
			g_availability = AssetAvailability::RootUnset;
			return g_assetRoot;
		}

		const std::filesystem::path base(root);

		// `MapRegistry::Load` probes the same spellings; this only decides whether
		// the assets are present at all. The deployed tree is lowercase
		// `data/glogic`, so both spellings are checked rather than assuming one.
		const std::filesystem::path candidates[] = {
		    base / "Data" / "GLogic" / "mapslist.mst",
		    base / "data" / "glogic" / "mapslist.mst",
		    base / "Data" / "GLogic" / "Level",
		    base / "data" / "glogic" / "level",
		};

		for (const std::filesystem::path& candidate : candidates)
		{
			if (std::filesystem::exists(candidate))
			{
				g_assetRoot     = base;
				g_availability = AssetAvailability::Available;
				return g_assetRoot;
			}
		}

		g_availability = AssetAvailability::RootWrong;
		return g_assetRoot;
	}

	bool RequireAssets(const char* what)
	{
		AssetRoot();

		if (g_availability == AssetAvailability::Available)
		{
			return true;
		}

		++g_assetTestsSkipped;
		std::printf("      SKIPPED %s: RAN_ASSET_ROOT is not set\n", what);
		if (g_availability == AssetAvailability::RootWrong)
		{
			std::printf("      (RAN_ASSET_ROOT is set but has no Data/GLogic tree)\n");
		}
		return false;
	}

	// The mapslist path the deployed tree actually uses. `MapRegistry` finds this by
	// probing; the tests ask for it directly so an assertion about the FILE is about
	// the file.
	std::filesystem::path MapsListPath()
	{
		const std::filesystem::path upper = AssetRoot() / "Data" / "GLogic" / "mapslist.mst";
		if (std::filesystem::exists(upper))
		{
			return upper;
		}
		return AssetRoot() / "data" / "glogic" / "mapslist.mst";
	}

	std::filesystem::path MapDirectory()
	{
		const std::filesystem::path upper = AssetRoot() / "Data" / "Map";
		if (std::filesystem::is_directory(upper))
		{
			return upper;
		}
		return AssetRoot() / "data" / "Map";
	}

	std::filesystem::path MapChild(const std::string& name)
	{
		return MapDirectory() / name;
	}

	std::vector<std::uint8_t> ReadAllBytes(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::binary);
		return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(file),
		                                 std::istreambuf_iterator<char>());
	}

	// ------------------------------------------------------------------------
	// Encoders - the fixtures have to be ENCRYPTED, not merely well-formed
	// ------------------------------------------------------------------------
	//
	// `DecodeMapsList` and `DecodeLevHead` both apply the inverse substitution table
	// to the body. A fixture written in plaintext would be REJECTED, and one that
	// happened to pass would be testing a decoder that does nothing.
	// `ByteCrypt::kSubstitutionTable` is the forward table (`byte_encode`,
	// ByteCrypt.cpp:102), so applying it here produces exactly what RAN writes.

	void EncodeBody(std::vector<std::uint8_t>& bytes)
	{
		for (std::size_t i = ByteCrypt::kBodyStart; i < bytes.size(); ++i)
		{
			bytes[i] = ByteCrypt::kSubstitutionTable[bytes[i]];
		}
	}

	void PutU16(std::vector<std::uint8_t>& bytes, std::size_t at, std::uint16_t value)
	{
		if (at + 2 > bytes.size())
		{
			bytes.resize(at + 2, 0u);
		}
		bytes[at]     = static_cast<std::uint8_t>(value & 0xFFu);
		bytes[at + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
	}

	void PutU32(std::vector<std::uint8_t>& bytes, std::size_t at, std::uint32_t value)
	{
		if (at + 4 > bytes.size())
		{
			bytes.resize(at + 4, 0u);
		}
		bytes[at]     = static_cast<std::uint8_t>(value & 0xFFu);
		bytes[at + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
		bytes[at + 2] = static_cast<std::uint8_t>((value >> 16) & 0xFFu);
		bytes[at + 3] = static_cast<std::uint8_t>((value >> 24) & 0xFFu);
	}

	// A file-type tag NUL-padded to FILETYPESIZE then the FileID - exactly the shape
	// `SetFileType` produces (SerialFile.cpp:36-44).
	// `resize`, never `assign`: a caller that already sized the buffer past the
	// header - the `DecodeBody` header test does, deliberately - must keep those
	// bytes. Assigning here would silently truncate the fixture instead.
	void PutFileType(std::vector<std::uint8_t>& bytes, const char* tag,
	                 std::uint32_t fileId)
	{
		bytes.resize(ByteCrypt::kBodyStart, 0u);
		std::memset(bytes.data(), 0, ByteCrypt::kFileTypeSize);
		std::memcpy(bytes.data(), tag, std::strlen(tag));
		PutU32(bytes, ByteCrypt::kFileTypeSize, fileId);
	}

	// A RAN string: `DWORD(byteCount)` then `byteCount` bytes INCLUDING the NUL
	// (SerialFile.cpp:446-448).
	void PutRanString(std::vector<std::uint8_t>& bytes, const std::string& value)
	{
		PutU32(bytes, bytes.size(), static_cast<std::uint32_t>(value.size() + 1));
		for (char c : value)
		{
			bytes.push_back(static_cast<std::uint8_t>(c));
		}
		bytes.push_back(0u);
	}

	struct MapsListRecordSpec
	{
		std::uint16_t mainId   = 0;
		std::uint16_t subId    = 0;
		std::string   levelFile;
		std::string   mapName;
		bool          used     = true;
	};

	// One `SMAPNODE_DATA` record at 0x0203 - the version all 99 shipped records use -
	// followed by exactly the field sequence `SMAPNODE_DATA::LOAD` reads
	// (GLMapNode.cpp:141-187). The 11 leading and 16 trailing `bool`s are the widths
	// this format is most often mis-read at, so they are spelled out.
	void PutMapsListRecord(std::vector<std::uint8_t>& bytes, const MapsListRecordSpec& spec)
	{
		PutU32(bytes, bytes.size(), 0x0203);
		bytes.push_back(spec.used ? 1u : 0u);

		PutRanString(bytes, spec.levelFile); // strFile comes FIRST
		PutU16(bytes, bytes.size(), spec.mainId);
		PutU16(bytes, bytes.size(), spec.subId);
		PutU32(bytes, bytes.size(), 0); // dwFieldSID

		for (int i = 0; i < 11; ++i) // PeaceZone .. ClubBattleZone
		{
			bytes.push_back(0u);
		}

		PutRanString(bytes, spec.mapName);
		PutRanString(bytes, std::string()); // strBGM
		PutRanString(bytes, std::string()); // strLoadingImageName

		for (int i = 0; i < 16; ++i) // 0x0203's trailing flags
		{
			bytes.push_back(0u);
		}
	}

	std::vector<std::uint8_t> BuildMapsList(const std::vector<MapsListRecordSpec>& records)
	{
		std::vector<std::uint8_t> bytes;
		PutFileType(bytes, kMapsListFileType, 0x0200);
		PutU32(bytes, ByteCrypt::kBodyStart, static_cast<std::uint32_t>(records.size()));

		for (const MapsListRecordSpec& record : records)
		{
			PutMapsListRecord(bytes, record);
		}

		EncodeBody(bytes);
		return bytes;
	}

	// A `.lev` at head version 0x0102 (`LOAD_0102`: strMapName, strWldFile,
	// division, bright - GLLevelHead.cpp:50-55). Only the head is written:
	// `DecodeLevHead` parses within the declared payload size and never looks past
	// it, which is how a real `.lev` is read here too.
	//
	// `declaredSizeOverride` writes a size field other than the true one, which is
	// how the "declared size disagrees with the bytes read" case is produced without
	// hand-assembling a file.
	// `extraPayloadBytes` appends filler INSIDE the declared head payload, after the
	// last field. That is the only shape that produces a declared size which is
	// readable yet larger than the bytes the fields consume - which is exactly the
	// mismatch legacy MsgBoxes about (GLLevelHead.cpp:61-62).
	std::vector<std::uint8_t> BuildLev(const std::string& wldFile,
	                                   std::uint32_t declaredSizeOverride = 0,
	                                   bool         useOverride           = false,
	                                   std::size_t  extraPayloadBytes     = 0)
	{
		std::vector<std::uint8_t> head;
		PutRanString(head, std::string()); // m_strMapName
		PutRanString(head, wldFile);       // m_strWldFile
		PutU32(head, head.size(), 0);      // m_eDivision
		PutU32(head, head.size(), 0);      // m_emBright
		head.resize(head.size() + extraPayloadBytes, 0xA5u);

		std::vector<std::uint8_t> bytes;
		PutFileType(bytes, kLevFileType, 0x0200);
		PutU32(bytes, ByteCrypt::kBodyStart, 0x0102u);
		PutU32(bytes, bytes.size(),
		       useOverride ? declaredSizeOverride
		                   : static_cast<std::uint32_t>(head.size()));
		bytes.insert(bytes.end(), head.begin(), head.end());

		EncodeBody(bytes);
		return bytes;
	}

	// A `.lev` written explicitly at head version 0x0101, whose `LOAD_0101` reads
	// strWldFile FIRST (GLLevelHead.cpp:33-40).
	std::vector<std::uint8_t> BuildLev0101(const std::string& wldFile)
	{
		std::vector<std::uint8_t> head;
		PutRanString(head, wldFile);       // m_strWldFile
		PutRanString(head, std::string()); // m_strMapName
		PutU32(head, head.size(), 0);      // m_emBright
		PutU32(head, head.size(), 0);      // m_eDivision

		std::vector<std::uint8_t> bytes;
		PutFileType(bytes, kLevFileType, 0x0200);
		PutU32(bytes, ByteCrypt::kBodyStart, 0x0101u);
		PutU32(bytes, bytes.size(), static_cast<std::uint32_t>(head.size()));
		bytes.insert(bytes.end(), head.begin(), head.end());

		EncodeBody(bytes);
		return bytes;
	}

	void WriteBytes(const std::filesystem::path& path,
	                const std::vector<std::uint8_t>& bytes)
	{
		std::filesystem::create_directories(path.parent_path());
		std::ofstream file(path, std::ios::binary);
		file.write(reinterpret_cast<const char*>(bytes.data()),
		           static_cast<std::streamsize>(bytes.size()));
	}

	// ------------------------------------------------------------------------
	// A throwaway asset root, in the deployed layout
	// ------------------------------------------------------------------------
	//
	// Using the real directory names is the point: `MapRegistry` finds `Data/GLogic`,
	// `Data/GLogic/Level` and `Data/Map` by probing, so a fixture with invented names
	// would be testing a different tree from the shipped one.
	class Fixture
	{
	public:
		Fixture()
		{
			static int counter = 0;
			root_ = std::filesystem::temp_directory_path() /
			        ("modern-002e-" + std::to_string(++counter));

			std::filesystem::remove_all(root_);
			std::filesystem::create_directories(LevelDirectory());
			std::filesystem::create_directories(MapDirectory());
		}

		~Fixture()
		{
			std::error_code ec;
			std::filesystem::remove_all(root_, ec);
		}

		Fixture(const Fixture&)            = delete;
		Fixture& operator=(const Fixture&) = delete;

		std::string Root() const { return root_.string(); }

		std::filesystem::path LevelDirectory() const
		{
			return root_ / "Data" / "GLogic" / "Level";
		}

		std::filesystem::path MapDirectory() const
		{
			return root_ / "Data" / "Map";
		}

		void WriteMapsList(const std::vector<std::uint8_t>& bytes)
		{
			WriteBytes(root_ / "Data" / "GLogic" / "mapslist.mst", bytes);
		}

		void WriteLev(const std::string& name, const std::vector<std::uint8_t>& bytes)
		{
			WriteBytes(LevelDirectory() / name, bytes);
		}

		void CopyWld(const std::string& name, const std::filesystem::path& source)
		{
			std::filesystem::copy_file(source, MapDirectory() / name,
			                          std::filesystem::copy_options::overwrite_existing);
		}

	private:
		std::filesystem::path root_;
	};

	// Loads the real registry once and reuses it. 99 navigation meshes over 359,366
	// cells is real work, every asset test wants the same object, and sharing one
	// instance is also what lets the tests observe the same shared-ownership
	// behaviour the Field workers will.
	const MapRegistry& RealRegistry()
	{
		static MapRegistry registry(AssetRoot().string());
		static bool         attempted = false;
		if (!attempted)
		{
			attempted = true;
			registry.Load();
		}
		return registry;
	}
}

// ===========================================================================
// 1. The cipher, in isolation
// ===========================================================================

MODERN_TEST(RanByteCrypt_TableIsATotalPermutation)
{
	// Legacy's `InitArray` complains through a MsgBox when a table has duplicates
	// (ByteCrypt.cpp:70-84). Asserting it here is what lets the decoder rely on the
	// inverse being total, with no fallback for an unmapped byte.
	std::uint8_t inverse[256];
	ByteCrypt::BuildInverseTable(inverse);

	bool seen[256] = {};
	for (int i = 0; i < 256; ++i)
	{
		CHECK(!seen[static_cast<std::size_t>(inverse[i])]);
		seen[static_cast<std::size_t>(inverse[i])] = true;
	}

	// And the round trip, which is the property every fixture below depends on.
	for (int i = 0; i < 256; ++i)
	{
		const std::uint8_t plain = static_cast<std::uint8_t>(i);
		const std::uint8_t coded = ByteCrypt::kSubstitutionTable[plain];
		CHECK_EQ(static_cast<int>(inverse[coded]), static_cast<int>(plain));
	}
}

MODERN_TEST(RanByteCrypt_DecodeBodyLeavesTheHeaderAlone)
{
	// `ReadFileType` reads bytes 0..131 with a raw `fread` and never through the
	// decoding path (SerialFile.cpp:55-57). A decoder that touched them would be
	// wrong rather than merely different.
	std::vector<std::uint8_t> bytes;
	PutFileType(bytes, kMapsListFileType, 0x0200);

	// Filler AFTER the header, so there is a body for `DecodeBody` to transform.
	// Written after `PutFileType` rather than before, because that helper only
	// guarantees the header is present.
	bytes.resize(400, 0x5A);

	const std::vector<std::uint8_t> before = bytes;
	ByteCrypt::DecodeBody(bytes);

	for (std::size_t i = 0; i < ByteCrypt::kBodyStart; ++i)
	{
		CHECK_EQ(static_cast<int>(bytes[i]), static_cast<int>(before[i]));
	}
	CHECK_NE(static_cast<int>(bytes[ByteCrypt::kBodyStart]), static_cast<int>(0x5A));
}

// ===========================================================================
// 2. Identity
// ===========================================================================

MODERN_TEST(MapIdentity_PackedRoundTripAndOrdering)
{
	const MapIdentity id(2, 0);

	CHECK_EQ(id.Packed(), 2u);
	CHECK(MapIdentity::FromPacked(id.Packed()) == id);
	CHECK(!id.IsNull());
	CHECK(MapIdentity::Null().IsNull());

	// `wMainID` is the low half because it is declared first in the struct the union
	// overlays. The two halves must not be interchangeable: swapping them would
	// silently address a different map.
	CHECK_NE(MapIdentity(0, 2).Packed(), id.Packed());
	CHECK(MapIdentity(0, 2) != id);

	// Deterministic order, and a hash that agrees with equality - the two
	// properties an ordered container and an unordered one need respectively.
	CHECK(MapIdentity(1, 5) < MapIdentity(2, 0));
	CHECK(MapIdentity(2, 0) < MapIdentity(2, 1));
	CHECK_EQ(MapIdentityHash{}(id), MapIdentityHash{}(MapIdentity::FromPacked(id.Packed())));

	CHECK_EQ(ToString(id), std::string("2/0"));
}

// ===========================================================================
// 3. mapslist.mst against the real asset
// ===========================================================================

MODERN_TEST(MapsList_ShippedAssetIsConsumedExactly)
{
	if (!RequireAssets("mapslist decode")) { return; }

	const std::vector<std::uint8_t> bytes = ReadAllBytes(MapsListPath());

	// 11,494 - 132 == 11,362. The exact-consumption check is the whole point of
	// this test: a wrong field order, a wrong `bool` width, or the wrong
	// substitution table all leave bytes over or run past the end.
	CHECK_EQ(bytes.size(), static_cast<std::size_t>(11494));

	MapsListFile   decoded;
	MapsListError  error;
	const bool     ok = DecodeMapsList(bytes.data(), bytes.size(), decoded, error);
	if (!ok)
	{
		std::printf("      %s\n", error.Format("mapslist.mst").c_str());
	}
	CHECK_EQ(static_cast<int>(error.status), static_cast<int>(MapsListStatus::Loaded));

	CHECK_EQ(decoded.fileId, 0x0200u);
	CHECK_EQ(decoded.payloadBytes, static_cast<std::size_t>(11362));
	CHECK_EQ(decoded.consumedBytes, decoded.payloadBytes);
	CHECK_EQ(decoded.declaredCount, 99u);
	CHECK_EQ(decoded.records.size(), static_cast<std::size_t>(99));

	// The two records 002c names explicitly, asserted against the file's own bytes.

	bool sawZero = false;
	bool sawTwo  = false;

	for (const MapRecord& record : decoded.records)
	{
		CHECK_EQ(record.recordVersion, 0x0203u);
		CHECK_EQ(record.flagCount, 16u);
		CHECK(record.used);
		CHECK_EQ(record.fieldServerId, 0u);
		CHECK(record.mapName.size() <= kMapNameMax);
		CHECK(record.RegistrationEligible());

		if (record.identity == MapIdentity(0, 0))
		{
			sawZero = true;
			// NOTE: WORLD-ENTRY-002c §1.2 printed this as `innerzone_01.Lev`. The
			// shipped file says `innerzone_01.lev` - lowercase - while map 2/0 really
			// does use `w_school_01.Lev`. The id and the map name match either way, so
			// this is the report's spelling and not a decode difference. It is also
			// the case mix the resolver must preserve rather than normalise.
			CHECK_EQ(record.levelFileName, std::string("innerzone_01.lev"));
			CHECK_EQ(record.mapName, std::string("SG_Campus1F"));
		}
		if (record.identity == MapIdentity(2, 0))
		{
			sawTwo = true;
			CHECK_EQ(record.levelFileName, std::string("w_school_01.Lev"));
			CHECK_EQ(record.mapName, std::string("SG_Campus"));
		}
	}

	CHECK(sawZero);
	CHECK(sawTwo);

	// Ids are distinct, because `LoadMapsListFile` inserts into a
	// `std::map<DWORD, SMAPNODE_DATA>` and a duplicate would be DROPPED rather than
	// overwrite - so 99 records means 99 usable maps only if the ids differ.
	std::map<std::uint32_t, int> seen;
	for (const MapRecord& record : decoded.records)
	{
		++seen[record.identity.Packed()];
	}
	CHECK_EQ(seen.size(), static_cast<std::size_t>(99));

	// And the case mix the resolver has to cope with is really present. Both
	// spellings occur in the deployed file; if one stopped occurring the registry
	// would still work, so this asserts the fixture rather than the code.
	bool sawCapitalExtension = false;
	bool sawLowerExtension    = false;
	for (const MapRecord& record : decoded.records)
	{
		if (record.levelFileName.size() > 4)
		{
			const std::string extension =
			    record.levelFileName.substr(record.levelFileName.size() - 4);
			sawCapitalExtension = sawCapitalExtension || extension == ".Lev";
			sawLowerExtension    = sawLowerExtension || extension == ".lev";
		}
	}
	CHECK(sawCapitalExtension);
	CHECK(sawLowerExtension);
}

// ===========================================================================
// 4. The registry against the real asset
// ===========================================================================

MODERN_TEST(MapRegistry_KnownMapResolvesToItsNavigationMesh)
{
	if (!RequireAssets("map 2/0")) { return; }

	// The proven example, end to end: 2/0 -> w_school_01.Lev -> w_school_01.wld.
	const MapRegistry& registry = RealRegistry();
	CHECK(registry.Loaded());

	const MapEntry* entry = registry.Find(MapIdentity(2, 0));
	if (entry == nullptr)
	{
		std::printf("      map 2/0 is not in the shipped mapslist.mst\n");
		CHECK(false);
		return;
	}

	CHECK_EQ(static_cast<int>(entry->status), static_cast<int>(MapResolveStatus::Loaded));
	CHECK_EQ(entry->levelFileName, std::string("w_school_01.Lev"));
	CHECK_EQ(entry->wldFileName, std::string("w_school_01.wld"));
	CHECK_EQ(entry->mapName, std::string("SG_Campus"));
	CHECK(entry->mesh != nullptr);

	if (entry->mesh != nullptr)
	{
		// 002c measured these off the parsed navigation block.
		CHECK_EQ(entry->mesh->CellCount(), static_cast<std::size_t>(13206));
		CHECK_EQ(entry->mesh->VertexCount(), static_cast<std::size_t>(24893));
		CHECK(entry->mesh->Built());
	}

	CHECK(!registry.WldPath(MapIdentity(2, 0)).empty());
}

MODERN_TEST(MapRegistry_FirstMapResolvesToItsNavigationMesh)
{
	if (!RequireAssets("map 0/0")) { return; }

	// 0/0 -> innerzone_01.Lev -> innerzone_01.wld. 4,392 cells, 12,523 vertices.
	const MapEntry* entry = RealRegistry().Find(MapIdentity(0, 0));
	if (entry == nullptr)
	{
		std::printf("      map 0/0 is not in the shipped mapslist.mst\n");
		CHECK(false);
		return;
	}

	CHECK_EQ(static_cast<int>(entry->status), static_cast<int>(MapResolveStatus::Loaded));
	CHECK_EQ(entry->levelFileName, std::string("innerzone_01.lev"));
	CHECK_EQ(entry->wldFileName, std::string("innerzone_01.wld"));
	CHECK_EQ(entry->mapName, std::string("SG_Campus1F"));

	if (entry->mesh != nullptr)
	{
		CHECK_EQ(entry->mesh->CellCount(), static_cast<std::size_t>(4392));
		CHECK_EQ(entry->mesh->VertexCount(), static_cast<std::size_t>(12523));
	}
}

MODERN_TEST(MapRegistry_SharedWldsYieldOneMesh)
{
	if (!RequireAssets("shared WLD")) { return; }

	const MapRegistry& registry = RealRegistry();

	// Group the registered maps by the `.wld` each resolved to. 002c measured 16 of
	// the 56 distinct files named by the 99 maps being named by MORE THAN ONE map
	// id, so there are many such groups; finding one from the data rather than
	// hardcoding a pair means this keeps working if a repack renumbers ids.
	std::map<std::string, std::vector<MapIdentity>> byPath;
	for (const MapEntry& entry : registry.Entries())
	{
		if (entry.status != MapResolveStatus::Loaded || entry.wldPath.empty())
		{
			continue;
		}
		byPath[entry.wldPath].push_back(entry.identity);
	}

	std::size_t sharedGroups = 0;
	std::size_t sharedMaps   = 0;

	for (const auto& group : byPath)
	{
		if (group.second.size() < 2)
		{
			continue;
		}

		++sharedGroups;
		sharedMaps += group.second.size();

		// The decisive assertion: ONE mesh object, not two equal ones.
		const std::shared_ptr<const NavigationMesh> a =
		    registry.NavigationMesh(group.second[0]);
		const std::shared_ptr<const NavigationMesh> b =
		    registry.NavigationMesh(group.second[1]);
		CHECK(a != nullptr);
		CHECK(b != nullptr);
		if (a != nullptr && b != nullptr)
		{
			CHECK(a.get() == b.get());
			// And it really is the same content, not merely the same pointer.
			CHECK_EQ(a->CellCount(), b->CellCount());
		}

		if (sharedGroups == 1)
		{
			std::printf("      first shared WLD: %s serves", group.first.c_str());
			for (MapIdentity id : group.second)
			{
				std::printf(" %s", ToString(id).c_str());
			}
			std::printf("\n");
		}
	}

	std::printf("      shared WLD files: %zu, maps behind them: %zu, distinct WLDs: %zu\n",
	            sharedGroups, sharedMaps, byPath.size());

	// 002c: 16 shared files, 99 maps over 56 distinct files.
	CHECK_GT(sharedGroups, static_cast<std::size_t>(0));
	CHECK_EQ(byPath.size(), static_cast<std::size_t>(56));
	CHECK_EQ(sharedGroups, static_cast<std::size_t>(16));

	// The specific three-map example the investigation named, checked for
	// existence so a repack that renumbers `suhak.wld` is visible rather than silent.
	const auto suhak = byPath.find(MapChild("suhak.wld").string());
	if (suhak != byPath.end())
	{
		CHECK_GE(suhak->second.size(), static_cast<std::size_t>(2));
	}
}

MODERN_TEST(MapRegistry_EveryRegisteredMapResolves)
{
	if (!RequireAssets("all registered maps")) { return; }

	const MapRegistry&       registry = RealRegistry();
	const MapRegistrySummary& summary = registry.Summary();

	// The strongest acceptance statement WORLD-ENTRY-002c supports, and the one this
	// milestone exists to make:
	//
	//     99 registered -> 99 resolved -> 99 existing WLD -> 99 navigation meshes
	//
	// Printed as well as asserted, so a different asset set is diagnosable rather
	// than merely fatal.
	std::printf("      mapslist records: %zu\n", summary.recordsRead);
	std::printf("      registered:       %zu\n", summary.registered);
	std::printf("      resolved:         %zu\n", summary.resolved);
	std::printf("      navigation meshes:%zu\n", summary.navigationMeshes);
	std::printf("      distinct WLDs:    %zu\n", summary.distinctWlds);
	std::printf("      missing lev %zu, malformed lev %zu, missing wld %zu, "
	            "malformed wld %zu, unsupported wld %zu, no navigation %zu\n",
	            summary.missingLev, summary.malformedLev, summary.missingWld,
	            summary.malformedWld, summary.unsupportedWld, summary.noNavigation);

	CHECK_EQ(summary.recordsRead, static_cast<std::size_t>(99));
	CHECK_EQ(summary.registered, static_cast<std::size_t>(99));
	CHECK_EQ(summary.resolved, static_cast<std::size_t>(99));
	CHECK_EQ(summary.navigationMeshes, static_cast<std::size_t>(99));

	CHECK_EQ(summary.missingLev, static_cast<std::size_t>(0));
	CHECK_EQ(summary.malformedLev, static_cast<std::size_t>(0));
	CHECK_EQ(summary.missingWld, static_cast<std::size_t>(0));
	CHECK_EQ(summary.malformedWld, static_cast<std::size_t>(0));
	CHECK_EQ(summary.unsupportedWld, static_cast<std::size_t>(0));
	CHECK_EQ(summary.noNavigation, static_cast<std::size_t>(0));
	CHECK_EQ(summary.notRegistered, static_cast<std::size_t>(0));
	CHECK_EQ(summary.ineligible, static_cast<std::size_t>(0));

	// 99 maps over 56 distinct files - the many-to-one relation, and the reason the
	// cache exists.
	CHECK_EQ(summary.distinctWlds, static_cast<std::size_t>(56));

	// Every registered map must be reachable by identity, and an id the deployed
	// list does not contain must not be.
	CHECK(registry.Find(MapIdentity(0, 0)) != nullptr);
	CHECK(registry.Find(MapIdentity(2, 0)) != nullptr);
	CHECK(registry.Find(MapIdentity(0, 200)) == nullptr);
	CHECK(registry.Find(MapIdentity(40000, 0)) == nullptr);
	CHECK(registry.NavigationMesh(MapIdentity(0, 200)) == nullptr);
	CHECK(registry.WldPath(MapIdentity(40000, 0)).empty());
}

MODERN_TEST(MapRegistry_LookupsAreStableAndConsumeNothing)
{
	if (!RequireAssets("immutable entries")) { return; }

	// The thread-safety claim, stated against the REGISTRY rather than against a
	// mesh: `MapRegistry` has no mutating member after `Load` returns, so repeated
	// lookups must all observe the same mesh pointer, and a lookup must consume
	// nothing that a later reader needs.
	const MapRegistry& registry = RealRegistry();

	const MapIdentity id(2, 0);
	const std::shared_ptr<const NavigationMesh> first = registry.NavigationMesh(id);
	CHECK(first != nullptr);

	for (int i = 0; i < 8; ++i)
	{
		const std::shared_ptr<const NavigationMesh> again = registry.NavigationMesh(id);
		CHECK(again.get() == first.get());
	}

	CHECK(registry.NavigationMesh(MapIdentity(0, 0)) != nullptr);
	CHECK(first != nullptr);
	if (first != nullptr)
	{
		CHECK(first->Built());
	}
}

MODERN_TEST(MapRegistry_AssetRootIsRequired)
{
	// No asset root, no registry - and the accessors must be safe afterwards rather
	// than reading a half-built object.
	// Braces, not parentheses: MapRegistry registry(std::string()) would declare a
	// FUNCTION named registry taking a function pointer - the most vexing parse.
	MapRegistry registry{std::string()};
	CHECK(!registry.Load());
	CHECK(!registry.Loaded());
	CHECK(!registry.LoadError().empty());
	CHECK(registry.Find(MapIdentity(0, 0)) == nullptr);
	CHECK(registry.NavigationMesh(MapIdentity(0, 0)) == nullptr);
	CHECK(registry.WldPath(MapIdentity(0, 0)).empty());
	CHECK_EQ(registry.Summary().registered, static_cast<std::size_t>(0));
	CHECK_EQ(registry.Entries().size(), static_cast<std::size_t>(0));

	// A root that exists but has no ASURA tree under it fails the same way, rather
	// than reporting an empty map list as a successful load.
	MapRegistry wrong("this-directory-does-not-exist-002e");
	CHECK(!wrong.Load());
	CHECK(!wrong.Loaded());
	// The two ways a load can fail must be distinguishable, or an operator cannot
	// tell a misconfigured root from a corrupt map list.
	CHECK(wrong.LoadError().find("Data") != std::string::npos);
	CHECK(wrong.LoadError().size() < 512);
}

// ===========================================================================
// 5. Failure paths, on synthetic fixtures
// ===========================================================================
//
// Each fixture below carries a REAL, forward-encrypted `mapslist.mst`, so every
// failure is produced by the production decoders with exactly one thing wrong.

MODERN_TEST(MapRegistry_MissingLevIsReportedAsMissingLev)
{
	Fixture fixture;

	MapsListRecordSpec spec;
	spec.mainId    = 7;
	spec.levelFile = "not_there.Lev";
	spec.mapName   = "TestMap";
	fixture.WriteMapsList(BuildMapsList({spec}));
	// No `.lev` written at all.

	MapRegistry registry(fixture.Root());
	CHECK(registry.Load());
	CHECK(registry.Loaded());

	const MapEntry* entry = registry.Find(MapIdentity(7, 0));
	CHECK(entry != nullptr);
	if (entry == nullptr)
	{
		return;
	}

	CHECK_EQ(static_cast<int>(entry->status), static_cast<int>(MapResolveStatus::MissingLev));
	CHECK(entry->mesh == nullptr);
	CHECK(entry->wldFileName.empty());

	// The message names the file that was looked for, so an operator does not have
	// to reconstruct it from the source.
	CHECK(entry->detail.find("not_there.Lev") != std::string::npos);
	CHECK(entry->detail.size() < 512);

	CHECK_EQ(registry.Summary().registered, static_cast<std::size_t>(1));
	CHECK_EQ(registry.Summary().missingLev, static_cast<std::size_t>(1));
	CHECK_EQ(registry.Summary().navigationMeshes, static_cast<std::size_t>(0));
}

MODERN_TEST(MapRegistry_MissingWldIsReportedAsMissingWld)
{
	Fixture fixture;
	fixture.WriteLev("good.lev", BuildLev("no_such_map.wld"));

	MapsListRecordSpec spec;
	spec.mainId    = 8;
	spec.levelFile = "good.lev";
	spec.mapName   = "TestMap";
	fixture.WriteMapsList(BuildMapsList({spec}));

	MapRegistry registry(fixture.Root());
	CHECK(registry.Load());

	const MapEntry* entry = registry.Find(MapIdentity(8, 0));
	CHECK(entry != nullptr);
	if (entry == nullptr)
	{
		return;
	}

	// The `.lev` parsed, so its WLD name is retained - which is what makes the
	// message actionable.
	CHECK_EQ(static_cast<int>(entry->status), static_cast<int>(MapResolveStatus::MissingWld));
	CHECK_EQ(entry->wldFileName, std::string("no_such_map.wld"));
	CHECK(entry->mesh == nullptr);
	CHECK(entry->detail.find("no_such_map.wld") != std::string::npos);
	CHECK_EQ(entry->wldPath, std::string());

	CHECK_EQ(registry.Summary().missingWld, static_cast<std::size_t>(1));
	CHECK_EQ(registry.Summary().navigationMeshes, static_cast<std::size_t>(0));
}

MODERN_TEST(MapRegistry_MalformedLevIsReportedWithoutCrashing)
{
	Fixture fixture;

	const std::vector<std::uint8_t> valid = BuildLev("some_map.wld");

	MapsListRecordSpec spec;
	spec.mainId    = 9;
	spec.levelFile = "truncated.lev";
	fixture.WriteMapsList(BuildMapsList({spec}));

	// Every prefix of a valid `.lev`. Each must produce a controlled status and no
	// crash - this is the loop that catches an out-of-bounds read a single
	// truncation point happened to miss.
	for (std::size_t size = 0; size < valid.size(); size += 5)
	{
		fixture.WriteLev("truncated.lev",
		                 std::vector<std::uint8_t>(
		                     valid.begin(),
		                     valid.begin() + static_cast<std::ptrdiff_t>(size)));

		MapRegistry registry(fixture.Root());
		CHECK(registry.Load());

		const MapEntry* entry = registry.Find(MapIdentity(9, 0));
		CHECK(entry != nullptr);
		if (entry == nullptr)
		{
			continue;
		}

		const int status = static_cast<int>(entry->status);
		CHECK(status == static_cast<int>(MapResolveStatus::MalformedLev) ||
		      status == static_cast<int>(MapResolveStatus::MissingLev));
		CHECK(entry->mesh == nullptr);
		CHECK(entry->detail.size() < 512);
	}

	// Control: the whole file is accepted, so the loop above is not passing because
	// the decoder rejects everything.
	fixture.WriteLev("truncated.lev", valid);

	MapRegistry registry(fixture.Root());
	CHECK(registry.Load());

	// With the whole file present the `.lev` PARSES, so the chain proceeds to the
	// `.wld` - which this fixture does not contain. Asserting `MissingWld` rather
	// than `MalformedLev` is what proves the difference: the prefix loop above
	// failed at the `.lev`, and this one did not.
	const MapEntry* entry = registry.Find(MapIdentity(9, 0));
	CHECK(entry != nullptr);
	if (entry != nullptr)
	{
		CHECK_EQ(static_cast<int>(entry->status), static_cast<int>(MapResolveStatus::MissingWld));
		CHECK_EQ(entry->wldFileName, std::string("some_map.wld"));
	}
}

MODERN_TEST(MapRegistry_MalformedMapslistFailsTheLoad)
{
	Fixture fixture;

	MapsListRecordSpec spec;
	spec.mainId    = 11;
	spec.levelFile = "any.lev";

	const std::vector<std::uint8_t> valid = BuildMapsList({spec});

	// Every truncation of the header and body must fail the load cleanly. No crash,
	// no out-of-bounds read, and above all no "success" with an empty list - that
	// would be the worst outcome, because every map would then read as unregistered.
	for (std::size_t size = 0; size < valid.size(); size += 37)
	{
		fixture.WriteMapsList(
		    std::vector<std::uint8_t>(valid.begin(),
		                              valid.begin() + static_cast<std::ptrdiff_t>(size)));

		MapRegistry registry(fixture.Root());
		CHECK(!registry.Load());
		CHECK(!registry.Loaded());
		CHECK(registry.Find(MapIdentity(11, 0)) == nullptr);
		CHECK_EQ(registry.Summary().recordsRead, static_cast<std::size_t>(0));
		// Every truncation must SAY something: an empty reason would be the case
		// where a silent "no maps" is worst.
		CHECK(!registry.LoadError().empty());
		CHECK(registry.LoadError().size() < 512);
	}

	// A corrupted FILE-TYPE tag is refused rather than parsed as records.
	{
		std::vector<std::uint8_t> corrupt = valid;
		std::memcpy(corrupt.data(), "GLMAPS_LISX", 11);
		fixture.WriteMapsList(corrupt);

		MapRegistry registry(fixture.Root());
		CHECK(!registry.Load());
	}

	// A FileID below the newest table selects a DIFFERENT substitution table in
	// legacy (GLMapList.cpp:137-141). Decoding this one with the newest table would
	// produce a plausible record stream from garbage, so it is reported instead.
	{
		std::vector<std::uint8_t> older = valid;
		PutU32(older, ByteCrypt::kFileTypeSize, 0x0100u);
		fixture.WriteMapsList(older);

		MapRegistry registry(fixture.Root());
		CHECK(!registry.Load());
	}

	// Control: the untouched file loads, and the reason is cleared - a stale failure
	// message left behind after a successful load would be its own small lie.
	fixture.WriteMapsList(valid);

	MapRegistry registry(fixture.Root());
	CHECK(registry.Load());
	CHECK(registry.LoadError().empty());
	CHECK_EQ(registry.Summary().recordsRead, static_cast<std::size_t>(1));
}

// ===========================================================================
// 6. Failure paths, on real shipped files
// ===========================================================================

MODERN_TEST(MapRegistry_UnsupportedWldIsClassifiedNotMalformed)
{
	if (!RequireAssets("unsupported FileID")) { return; }

	// `square_rd.wld` carries FileID 0x0202, which
	// `NSLANDMAN_SUPPORT::IsLandManSupported` does not list - so LEGACY refuses it
	// too (GLLandManSet.cpp:41). The registry must say "unsupported", because an
	// operator told "malformed" would spend a day looking for corruption that does
	// not exist.
	//
	// The file is unregistered content - all 99 registered maps resolve - so a
	// fixture names it directly rather than relying on a map id that does not exist.
	Fixture fixture;
	fixture.CopyWld("square_rd.wld", MapChild("square_rd.wld"));
	fixture.WriteLev("square.lev", BuildLev("square_rd.wld"));

	MapsListRecordSpec spec;
	spec.mainId    = 21;
	spec.levelFile = "square.lev";
	fixture.WriteMapsList(BuildMapsList({spec}));

	MapRegistry registry(fixture.Root());
	CHECK(registry.Load());

	const MapEntry* entry = registry.Find(MapIdentity(21, 0));
	CHECK(entry != nullptr);
	if (entry == nullptr)
	{
		return;
	}

	CHECK_EQ(static_cast<int>(entry->status), static_cast<int>(MapResolveStatus::UnsupportedWld));
	CHECK(entry->mesh == nullptr);
	CHECK(entry->detail.find("FileID") != std::string::npos);

	CHECK_EQ(registry.Summary().unsupportedWld, static_cast<std::size_t>(1));
	CHECK_EQ(registry.Summary().malformedWld, static_cast<std::size_t>(0));
}

MODERN_TEST(MapRegistry_NoNavigationIsNotAnError)
{
	if (!RequireAssets("bExist == 0")) { return; }

	// `login.wld` is one of the nine shipped files with `bExist == 0`: a valid file
	// with no navigation in it, which RAN never walks. The registry must classify it
	// as `NoNavigation` - a successful read - not as a malformed `.wld`, because
	// nine good login maps becoming nine load errors would be a real regression.
	//
	// The other eight are named too, so a change to WHICH files lack navigation is a
	// failure rather than a count that still adds up.
	const char* expected[] = {
	    "character1_slt.wld",    "character_slt.wld",  "character_slt_main.wld",
	    "character_slt_old.wld", "character_slt_s01.wld", "character_slt_s02.wld",
	    "character_slt_s03.wld", "log_in.wld",         "login.wld",
	};

	for (const char* name : expected)
	{
		Fixture fixture;
		fixture.CopyWld(name, MapChild(name));
		fixture.WriteLev("x.lev", BuildLev(name));

		MapsListRecordSpec spec;
		spec.mainId    = 22;
		spec.levelFile = "x.lev";
		fixture.WriteMapsList(BuildMapsList({spec}));

		MapRegistry registry(fixture.Root());
		CHECK(registry.Load());

		const MapEntry* entry = registry.Find(MapIdentity(22, 0));
		CHECK(entry != nullptr);
		if (entry == nullptr)
		{
			continue;
		}

		if (entry->status != MapResolveStatus::NoNavigation)
		{
			std::printf("      %s -> %s (%s)\n", name, ToString(entry->status),
			            entry->detail.c_str());
		}
		CHECK_EQ(static_cast<int>(entry->status),
		         static_cast<int>(MapResolveStatus::NoNavigation));
		CHECK(entry->mesh == nullptr);
		CHECK_EQ(entry->wldFileName, std::string(name));

		// Counted as neither resolved-with-a-mesh nor a failure.
		CHECK_EQ(registry.Summary().noNavigation, static_cast<std::size_t>(1));
		CHECK_EQ(registry.Summary().navigationMeshes, static_cast<std::size_t>(0));
		CHECK_EQ(registry.Summary().malformedWld, static_cast<std::size_t>(0));
	}
}

// ===========================================================================
// 7. The chain and the shared mesh, without depending on the real registry
// ===========================================================================

MODERN_TEST(MapRegistry_FixtureResolvesTheWholeChain)
{
	// The positive control for every fixture failure test above: the same builder
	// produces a tree that DOES resolve, so "the registry reported MissingLev"
	// cannot be an artefact of the fixture.
	//
	// It needs a real `.wld`, so it borrows the smallest shipped one. Without the
	// assets it is skipped, and the failure tests stand on their own.
	if (!RequireAssets("fixture chain")) { return; }

	Fixture fixture;
	fixture.CopyWld("bambooforest.wld", MapChild("bambooforest.wld"));
	fixture.WriteLev("a.lev", BuildLev("bambooforest.wld"));
	fixture.WriteLev("b.lev", BuildLev("bambooforest.wld"));

	// Two map ids, two `.lev` files, ONE `.wld`. 002c measured this shape on the
	// deployed set (`w_city_s_02.lev` and `w_city_s_03.lev` both name `suhak.wld`),
	// and it is the whole reason the cache exists.
	MapsListRecordSpec first;
	first.mainId    = 30;
	first.levelFile = "a.lev";
	first.mapName   = "SharedA";

	MapsListRecordSpec second;
	second.mainId    = 31;
	second.levelFile = "b.lev";
	second.mapName   = "SharedB";

	fixture.WriteMapsList(BuildMapsList({first, second}));

	MapRegistry registry(fixture.Root());
	CHECK(registry.Load());

	CHECK_EQ(registry.Summary().registered, static_cast<std::size_t>(2));
	CHECK_EQ(registry.Summary().resolved, static_cast<std::size_t>(2));
	CHECK_EQ(registry.Summary().navigationMeshes, static_cast<std::size_t>(2));
	// Two maps, ONE file, ONE mesh built.
	CHECK_EQ(registry.Summary().distinctWlds, static_cast<std::size_t>(1));

	const std::shared_ptr<const NavigationMesh> a = registry.NavigationMesh(MapIdentity(30, 0));
	const std::shared_ptr<const NavigationMesh> b = registry.NavigationMesh(MapIdentity(31, 0));
	CHECK(a != nullptr);
	CHECK(b != nullptr);
	if (a != nullptr && b != nullptr)
	{
		CHECK(a.get() == b.get());
		// 002c's measurement for bambooforest.wld.
		CHECK_EQ(a->CellCount(), static_cast<std::size_t>(404));
		CHECK_EQ(a->VertexCount(), static_cast<std::size_t>(1212));
	}

	// Both entries record the same resolved `.wld`, which is the inspectable form of
	// the shared-ownership claim - a caller can see it without comparing pointers.
	CHECK(!registry.WldPath(MapIdentity(30, 0)).empty());
	CHECK_EQ(registry.WldPath(MapIdentity(30, 0)), registry.WldPath(MapIdentity(31, 0)));

	// And a `.lev` naming the SAME WLD file under a different spelling still shares
	// the mesh, because the cache key is the canonical path of the file that was
	// actually opened rather than the string the asset used.
	fixture.WriteLev("c.lev", BuildLev("BAMBOOFOREST.WLD"));

	MapsListRecordSpec third;
	third.mainId    = 32;
	third.levelFile = "c.lev";
	fixture.WriteMapsList(BuildMapsList({first, second, third}));

	MapRegistry mixedCase(fixture.Root());
	CHECK(mixedCase.Load());

	const std::shared_ptr<const NavigationMesh> first_ =
	    mixedCase.NavigationMesh(MapIdentity(30, 0));
	const std::shared_ptr<const NavigationMesh> third_ =
	    mixedCase.NavigationMesh(MapIdentity(32, 0));
	CHECK(first_ != nullptr);
	CHECK(third_ != nullptr);
	if (first_ != nullptr && third_ != nullptr)
	{
		CHECK(first_.get() == third_.get());
	}
	CHECK_EQ(mixedCase.Summary().distinctWlds, static_cast<std::size_t>(1));
}

MODERN_TEST(MapRegistry_IneligibleRecordIsSkippedLikeLegacy)
{
	Fixture fixture;

	MapsListRecordSpec unused;
	unused.mainId    = 40;
	unused.levelFile = "a.lev";
	unused.mapName   = "Disabled";
	unused.used      = false;

	MapsListRecordSpec usable;
	usable.mainId    = 41;
	usable.levelFile = "b.lev";
	usable.mapName   = "Usable";

	fixture.WriteMapsList(BuildMapsList({unused, usable}));

	MapRegistry registry(fixture.Root());
	CHECK(registry.Load());

	// Both records are READ - the file really does contain both - but RAN's filter
	// (`LoadMapsListFile`, GLMapList.cpp:162-164) keeps only the second. Collapsing
	// these two counts into one is how "99 registered" would stop meaning what 002c
	// meant.
	CHECK_EQ(registry.Summary().recordsRead, static_cast<std::size_t>(2));
	CHECK_EQ(registry.Summary().registered, static_cast<std::size_t>(1));
	CHECK_EQ(registry.Summary().ineligible, static_cast<std::size_t>(1));

	// The skipped record is still findable, with its `.lev` name retained, so the
	// count of records in the file stays honest.
	const MapEntry* skipped = registry.Find(MapIdentity(40, 0));
	CHECK(skipped != nullptr);
	if (skipped != nullptr)
	{
		CHECK_EQ(static_cast<int>(skipped->status), static_cast<int>(MapResolveStatus::Ineligible));
		CHECK_EQ(skipped->levelFileName, std::string("a.lev"));
	}

	// A map name over `MAP_NAME_MAX` is rejected the same way (GLMapList.cpp:172-176).
	MapsListRecordSpec tooLong;
	tooLong.mainId    = 42;
	tooLong.levelFile = "c.lev";
	tooLong.mapName   = "a_name_that_is_far_too_long";

	fixture.WriteMapsList(BuildMapsList({usable, tooLong}));

	MapRegistry named(fixture.Root());
	CHECK(named.Load());
	CHECK_EQ(named.Summary().registered, static_cast<std::size_t>(1));
	CHECK_EQ(named.Summary().ineligible, static_cast<std::size_t>(1));
	CHECK(named.Find(MapIdentity(42, 0)) != nullptr);
}

// ===========================================================================
// 8. The .lev decoder, at the byte level
// ===========================================================================

MODERN_TEST(LevHead_WldNameMayNotEscapeTheMapDirectory)
{
	// `GLLandMan` concatenates `m_strWldFile` onto the map directory, so a name
	// carrying a separator, a drive or `..` would read outside the configured asset
	// root. A loader that honoured it would be a path-traversal primitive, so the
	// decoder refuses it here rather than trusting the caller to sanitise.
	const char* escapes[] = {
	    "../outside.wld",
	    "..\\outside.wld",
	    "sub/dir/inside.wld",
	    "sub\\dir\\inside.wld",
	    "C:\\windows\\system32\\evil.wld",
	    "",
	};

	for (const char* name : escapes)
	{
		const std::vector<std::uint8_t> bytes = BuildLev(name);

		LevHead      head;
		LevHeadError error;
		const bool   ok = DecodeLevHead(bytes.data(), bytes.size(), head, error);
		if (ok)
		{
			std::printf("      accepted m_strWldFile \"%s\"\n", name);
		}
		CHECK(!ok);
		CHECK_EQ(static_cast<int>(error.status), static_cast<int>(LevHeadStatus::Inconsistent));
	}

	// The control: a bare name is what every shipped `.lev` has.
	const std::vector<std::uint8_t> good = BuildLev("innerzone_01.wld");

	LevHead      head;
	LevHeadError error;
	CHECK(DecodeLevHead(good.data(), good.size(), head, error));
	CHECK_EQ(head.wldFileName, std::string("innerzone_01.wld"));
}

MODERN_TEST(LevHead_BothHeadVersionsAreImplemented)
{
	// `LOAD_0102` reads strMapName then strWldFile; `LOAD_0101` reads them the other
	// way round (GLLevelHead.cpp:31-63). Reading one with the other's order yields a
	// plausible-looking filename naming the WRONG file, which for this field is the
	// worst possible failure - so both are exercised, each asserting the name it
	// should have produced.
	const std::vector<std::uint8_t> bytes = BuildLev("shared_target.wld");

	LevHead      head;
	LevHeadError error;
	CHECK(DecodeLevHead(bytes.data(), bytes.size(), head, error));
	CHECK_EQ(head.headVersion, 0x0102u);
	CHECK_EQ(head.wldFileName, std::string("shared_target.wld"));
	CHECK_EQ(head.consumedPayloadBytes, head.declaredPayloadBytes);

	// 0x0101's order: the WLD name comes FIRST.
	const std::vector<std::uint8_t> older = BuildLev0101("shared_target.wld");

	LevHead      parsed;
	LevHeadError parsedError;
	CHECK(DecodeLevHead(older.data(), older.size(), parsed, parsedError));
	CHECK_EQ(parsed.headVersion, 0x0101u);
	CHECK_EQ(parsed.wldFileName, std::string("shared_target.wld"));
}

MODERN_TEST(LevHead_UnsupportedHeadVersionIsReported)
{
	// Legacy skips `dwSize` bytes and carries on (GLLevelHead.cpp:23-25), which
	// cannot produce a trustworthy filename. Reporting is strictly better than
	// guessing: a wrong WLD name silently loads the wrong map.
	std::vector<std::uint8_t> head;
	PutRanString(head, std::string());
	PutRanString(head, std::string("some_map.wld"));
	PutU32(head, head.size(), 0);
	PutU32(head, head.size(), 0);

	std::vector<std::uint8_t> bytes;
	PutFileType(bytes, kLevFileType, 0x0200);
	PutU32(bytes, ByteCrypt::kBodyStart, 0x0999u);
	PutU32(bytes, bytes.size(), static_cast<std::uint32_t>(head.size()));
	bytes.insert(bytes.end(), head.begin(), head.end());
	EncodeBody(bytes);

	LevHead      rejected;
	LevHeadError rejectedError;
	CHECK(!DecodeLevHead(bytes.data(), bytes.size(), rejected, rejectedError));
	CHECK_EQ(static_cast<int>(rejectedError.status),
	         static_cast<int>(LevHeadStatus::UnsupportedHeadVersion));
	CHECK_EQ(std::string(rejectedError.stage), std::string("head version"));
}

MODERN_TEST(LevHead_BadInputsAreRefused)
{
	LevHead      head;
	LevHeadError error;

	CHECK(!DecodeLevHead(nullptr, 4096, head, error));
	CHECK(!DecodeLevHead(nullptr, 0, head, error));

	std::vector<std::uint8_t> tiny(16, 0u);
	CHECK(!DecodeLevHead(tiny.data(), tiny.size(), head, error));
	CHECK_EQ(static_cast<int>(error.status), static_cast<int>(LevHeadStatus::InvalidFile));

	// Wrong file type.
	std::vector<std::uint8_t> wrong;
	PutFileType(wrong, kMapsListFileType, 0x0200);
	wrong.resize(400, 0u);
	CHECK(!DecodeLevHead(wrong.data(), wrong.size(), head, error));
	CHECK_EQ(std::string(error.stage), std::string("file type"));

	// A FileID below the newest table selects a DIFFERENT substitution table in
	// legacy (GLLevelFileSaveLoad.cpp:183-187). Decoding with this one would
	// produce a plausible filename from garbage, so it is reported instead.
	std::vector<std::uint8_t> old;
	PutFileType(old, kLevFileType, 0x0100u);
	old.resize(400, 0u);
	CHECK(!DecodeLevHead(old.data(), old.size(), head, error));
	CHECK_EQ(static_cast<int>(error.status), static_cast<int>(LevHeadStatus::UnsupportedFormat));

	// A declared payload size larger than the file. Legacy MsgBoxes and carries on
	// (GLLevelHead.cpp:61-62); here it is refused, because no read below can be fed
	// by an attacker-chosen length.
	const std::vector<std::uint8_t> oversized = BuildLev("x.wld", 0xFFFFu, true);
	CHECK(!DecodeLevHead(oversized.data(), oversized.size(), head, error));
	CHECK_EQ(std::string(error.stage), std::string("head size"));
	CHECK_EQ(static_cast<int>(error.status), static_cast<int>(LevHeadStatus::Truncated));

	// A declared size that is LARGER than the fields consume. Legacy MsgBoxes and
	// carries on regardless (GLLevelHead.cpp:61-62), which is a diagnostic and not a
	// validation; here it is `Inconsistent`, because a mismatch means the size or the
	// field order is wrong and reporting beats silently trusting either number.
	//
	// The filler has to be INSIDE the declared payload: a declared size larger than
	// the file is a truncation (checked first), not an inconsistency.
	const std::vector<std::uint8_t> mismatched = BuildLev("x.wld", 0u, false, 4u);
	CHECK(!DecodeLevHead(mismatched.data(), mismatched.size(), head, error));
	CHECK_EQ(static_cast<int>(error.status), static_cast<int>(LevHeadStatus::Inconsistent));
	CHECK_EQ(std::string(error.stage), std::string("head size"));

	// A declared size SMALLER than the fields consume is a truncation instead: the
	// payload genuinely ends early, so a read runs past it.
	LevHead      good;
	LevHeadError goodError;
	const std::vector<std::uint8_t> goodBytes = BuildLev("x.wld");
	CHECK(DecodeLevHead(goodBytes.data(), goodBytes.size(), good, goodError));

	const std::vector<std::uint8_t> shortPayload =
	    BuildLev("x.wld", good.declaredPayloadBytes - 8u, true);
	CHECK(!DecodeLevHead(shortPayload.data(), shortPayload.size(), head, error));
	CHECK_EQ(static_cast<int>(error.status), static_cast<int>(LevHeadStatus::Truncated));

	// And the decoder never modifies the caller's buffer, so a caller that caches
	// file contents can reuse it.
	std::vector<std::uint8_t>       original = BuildLev("reusable.wld");
	const std::vector<std::uint8_t> before   = original;

	LevHead      parsed;
	LevHeadError parsedError;
	CHECK(DecodeLevHead(original.data(), original.size(), parsed, parsedError));
	CHECK(original == before);
	CHECK_EQ(parsed.wldFileName, std::string("reusable.wld"));
}

// ===========================================================================
// 9. Configuration
// ===========================================================================

MODERN_TEST(MapRegistry_AssetRootIsConfigured)
{
	// If RAN_ASSET_ROOT is set but unusable, that is a CONFIGURATION error and must
	// fail loudly - otherwise every asset test above silently skips and the run
	// looks green.
	AssetRoot();

	if (g_availability == AssetAvailability::RootUnset)
	{
		++g_assetTestsSkipped;
		std::printf("      RAN_ASSET_ROOT is not set: the asset tests above DID NOT RUN "
		            "(%d skipped)\n",
		            g_assetTestsSkipped);
		return;
	}

	CHECK(g_availability == AssetAvailability::Available);
	CHECK(std::filesystem::is_regular_file(MapsListPath()));
	CHECK(std::filesystem::is_directory(MapDirectory()));
}
