// WORLD-ENTRY-002d: WLD navigation asset tests.
//
// Two kinds of test live here, and they are deliberately mixed in one file
// because they test one thing - the asset path - and splitting them would put
// the boundary in the middle of that thing:
//
//   * REAL ASSET tests, driven from RAN_ASSET_ROOT. These are the ones that
//     matter: the expected counts below are the ones WORLD-ENTRY-002c measured
//     across all 87 shipped `.wld` files, and a regression in the reader changes
//     them.
//
//   * SYNTHETIC MALFORMED tests. A truncated or corrupt file cannot be found in
//     the asset set - every shipped file parses - so those inputs are built here
//     byte by byte. What they assert is that the reader REFUSES them with a
//     named status and does not read out of bounds or ask for a huge allocation.
//
// ---------------------------------------------------------------------------
// RAN_ASSET_ROOT
// ---------------------------------------------------------------------------
//
// The same convention as modern/client/assets/ClientMxfTests.cpp:308 and its
// siblings - the root of an ASURA client install, supplied by the environment.
// No path is compiled in, so the production loader has no developer path in it
// and the tests run wherever the assets are.
//
//     set RAN_ASSET_ROOT=D:\FILES\project\RanOnline-Build\ASURA CLIENT
//
// When it is unset the asset tests print a loud banner and skip, and the
// synthetic tests still run. A run that skipped them is not evidence of
// anything, which is why the banner is not a quiet note.

#include "TestHarness.h"

#include "navigation/NavigationMesh.h"
#include "navigation/NavigationPath.h"
#include "navigation/NavigationSearchSession.h"
#include "navigation/WldCrypt.h"
#include "navigation/WldNavigationLoader.h"
#include "navigation/WldNavigationReader.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace Modern;
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

	AssetAvailability g_availability = AssetAvailability::RootUnset;
	std::filesystem::path g_mapDirectory;
	int g_assetTestsSkipped = 0;

	const std::filesystem::path& MapDirectory()
	{
		if (g_availability == AssetAvailability::RootUnset)
		{
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
				return g_mapDirectory;
			}

			const std::filesystem::path base(root);

			// The shipped client spells it `Data`; some repacks lowercase it.
			const std::filesystem::path candidates[] = {
			    base / "Data" / "Map",
			    base / "data" / "Map",
			    base / "Map",
			};

			for (const std::filesystem::path& candidate : candidates)
			{
				if (std::filesystem::is_directory(candidate))
				{
					g_mapDirectory     = candidate;
					g_availability     = AssetAvailability::Available;
					return g_mapDirectory;
				}
			}

			g_availability = AssetAvailability::RootWrong;
			return g_mapDirectory;
		}
		return g_mapDirectory;
	}

	// Returns false after printing why, so a skipped run is visible.
	bool RequireAssets(const char* what)
	{
		MapDirectory();

		if (g_availability == AssetAvailability::Available)
		{
			return true;
		}

		++g_assetTestsSkipped;
		std::printf("      SKIPPED %s: RAN_ASSET_ROOT is not set\n", what);
		if (g_availability == AssetAvailability::RootWrong)
		{
			std::printf("      (RAN_ASSET_ROOT is set but has no Data/Map directory)\n");
		}
		return false;
	}

	std::string AssetPath(const char* fileName)
	{
		return (MapDirectory() / fileName).string();
	}

	// ------------------------------------------------------------------------
	// Synthetic WLD builder
	// ------------------------------------------------------------------------
	//
	// Produces the smallest file the reader accepts, so each malformed test can
	// corrupt exactly one field and assert exactly one rejection.
	//
	//   1 vertex-count slot, 3 vertices forming a real right triangle,
	//   1 cell record of 188 bytes, 3 link entries.
	struct SynthWld
	{
		std::vector<std::uint8_t> bytes;
		std::size_t naviBlock = 0;

		SynthWld()
{
			// Sized up front. The first write below is a raw `memcpy` of the file type
			// tag at offset 0, and writing into an empty vector through `data()` is a
			// heap overflow - a default-constructed vector has no writable storage.
			// Every later write goes through `PutU32`, which grows on demand; this one
			// does not, so the buffer exists before the first byte is touched.
			bytes.assign(1200, 0u);

			// The file type and FileID.
			std::memcpy(bytes.data() + 0, "LAND.MAN", 8);
			PutU32(128, 0x0114);

			// Map id at 132: 0 in every shipped file, and never validated.
			PutU32(132, 0);

			// Map name, 136..263: left as zeroes.

			// File mark at 264.
			PutU32(264, 0x0101);
			PutU32(268, 16);

			// Payload order for 0x0101 is NAVI, WEATHER, GATE, COLL.
			PutU32(272, 400);     // dwNAVI_MARK
			PutU32(276, 0);       // dwWEATHER_MARK
			PutU32(280, 0);       // dwGATE_MARK
			PutU32(284, 0);       // dwCOLL_MARK

			naviBlock = 132 + 400;
			Resize(naviBlock + 4 + 4 + 36 + 4 + 188 + 3 * 4);

			// Exactly the bytes a valid file needs: header, bExist, vertex count, three
			// vertices, cell count, one cell record, three link flags.
			//
			// `3 * 4` and not `12 * 4`. The trailing slack matters here, because
			// `Wld_TruncatedInputIsRejectedNotRead` walks every prefix of this buffer
			// and asserts each one is refused - trailing padding would make the longest
			// prefixes parse cleanly, and the test would be asserting against a fixture
			// that is not actually tight.

			PutU32(static_cast<std::size_t>(naviBlock), 1); // bExist
			std::size_t p = static_cast<std::size_t>(naviBlock) + 4;

			PutU32(p, 3); // vertex count
			p += 4;
			PutFloat(p + 0, 0.0f);
			PutFloat(p + 4, 0.0f);
			PutFloat(p + 8, 0.0f);
			PutFloat(p + 12, 100.0f);
			PutFloat(p + 16, 0.0f);
			PutFloat(p + 20, 0.0f);
			PutFloat(p + 24, 0.0f);
			PutFloat(p + 28, 0.0f);
			PutFloat(p + 32, 100.0f);
			p += 36;

			PutU32(p, 1); // cell count
			p += 4;

			// The cell record. Only the id and the three vertex indices are read by
			// the parser; the rest is filled with a coherent plane, centre and
			// midpoints so that a mesh built from it is usable rather than
			// degenerate.
			PutU32(p + NavigationCell::kOffsetCellId, 0);
			PutU32(p + NavigationCell::kOffsetVertex + 0, 0);
			PutU32(p + NavigationCell::kOffsetVertex + 4, 1);
			PutU32(p + NavigationCell::kOffsetVertex + 8, 2);

			// Plane: normal (0,1,0), point at the origin, distance 0. So a point's Y
			// on this cell is 0 - a flat floor, which makes snap assertions readable.
			PutFloat(p + NavigationCell::kOffsetPlane + 0, 0.0f);
			PutFloat(p + NavigationCell::kOffsetPlane + 4, 1.0f);
			PutFloat(p + NavigationCell::kOffsetPlane + 8, 0.0f);
			PutFloat(p + NavigationCell::kOffsetPlane + 12, 0.0f);
			PutFloat(p + NavigationCell::kOffsetPlane + 16, 0.0f);
			PutFloat(p + NavigationCell::kOffsetPlane + 20, 0.0f);
			PutFloat(p + NavigationCell::kOffsetPlane + 24, 0.0f);

			const std::size_t centre = p + NavigationCell::kOffsetCenterPoint;
			PutFloat(centre + 0, 100.0f / 3.0f);
			PutFloat(centre + 4, 0.0f);
			PutFloat(centre + 8, 100.0f / 3.0f);

			p += NavigationCell::kCellRecordBytes;

			// Three link entries, all "no link": a single-cell mesh has no
			// neighbours, so all three edges are solid.
			for (int i = 0; i < 3; ++i)
			{
				PutU32(p, 0);
				p += 4;
			}
		}

		std::uint8_t* At(std::size_t offset) { return bytes.data() + offset; }

		void Resize(std::size_t size) { bytes.resize(size, 0u); }

		void PutU32(std::size_t offset, std::uint32_t value)
		{
			if (offset + 4 > bytes.size())
			{
				bytes.resize(offset + 4, 0u);
			}
			std::uint8_t* p = bytes.data() + offset;
			p[0] = static_cast<std::uint8_t>(value & 0xFF);
			p[1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
			p[2] = static_cast<std::uint8_t>((value >> 16) & 0xFF);
			p[3] = static_cast<std::uint8_t>((value >> 24) & 0xFF);
		}

		void PutFloat(std::size_t offset, float value)
		{
			std::uint32_t bits = 0;
			std::memcpy(&bits, &value, sizeof(bits));
			PutU32(offset, bits);
		}
	};

	// The GOTO destination probe, exactly as `GLChar::MsgGoto` builds it
	// (GLCharMsg.cpp:293-297): a 20-unit vertical span centred on the target.
	Vector3 ProbeTop(const Vector3& target)
	{
		return Vector3{target.x, target.y + NavigationCell::kDestinationProbeHalfHeight,
		               target.z};
	}

	Vector3 ProbeBottom(const Vector3& target)
	{
		return Vector3{target.x, target.y - NavigationCell::kDestinationProbeHalfHeight,
		               target.z};
	}
}

// ===========================================================================
// 1. The cipher, in isolation
// ===========================================================================

MODERN_TEST(WldCrypt_PlainAndEncryptedFileTypesAreDistinguished)
{
	const std::uint8_t plain[8]    = {'L', 'A', 'N', 'D', '.', 'M', 'A', 'N'};
	const std::uint8_t encrypted[8] = {'L', 'a', 'n', 'd', '.', 'M', 'a', 'n'};

	CHECK(!WldCrypt::IsEncrypted(plain, sizeof(plain)));
	CHECK(WldCrypt::IsEncrypted(encrypted, sizeof(encrypted)));

	// Case matters: only the exact `Land.Man` tag means encrypted.
	CHECK(!WldCrypt::IsEncrypted(plain, 4u));
	CHECK(!WldCrypt::IsEncrypted(nullptr, 1024u));
}

MODERN_TEST(WldCrypt_HeaderBytesAreNeverTransformed)
{
	std::vector<std::uint8_t> data(400, 0x00);
	std::memcpy(data.data(), "Land.Man", 8);
	data[128] = 0x14; // a FileID, which Encryption_WLD also leaves alone
	data[131] = 0x01;

	const std::vector<std::uint8_t> before = data;

	WldCrypt::DecryptBody(data);

	// Bytes 0..131 must be identical: `ReadFileType` reads them with raw fread
	// and never through the decrypting path (SerialFile.cpp:55-57).
	for (std::size_t i = 0; i < WldCrypt::kBodyStart; ++i)
	{
		CHECK_EQ(static_cast<int>(data[i]), static_cast<int>(before[i]));
	}

	// And the body must have moved, or the test proves nothing.
	CHECK_NE(static_cast<int>(data[WldCrypt::kBodyStart]), static_cast<int>(before[WldCrypt::kBodyStart]));
}

MODERN_TEST(WldCrypt_TransformIsTheDocumentedThreeSteps)
{
	// b = ((b - 0x10) ^ 0x10), verified against the three legacy statements
	// rather than against a round trip - the cipher is NOT an involution, which
	// WldCrypt.h documents.
	std::vector<std::uint8_t> one(133, 0x00);
	one[132] = 0x00;
	WldCrypt::DecryptBody(one);
	CHECK_EQ(static_cast<int>(one[132]), static_cast<int>((0x00 - 0x10) & 0xFF) ^ 0x10);

	std::vector<std::uint8_t> two(133, 0x00);
	two[132] = 0xFF;
	WldCrypt::DecryptBody(two);
	CHECK_EQ(static_cast<int>(two[132]), static_cast<int>((0xFF - 0x10) & 0xFF) ^ 0x10);

	// A short buffer is a no-op, not a read past the end.
	std::vector<std::uint8_t> tiny(64, 0x5A);
	WldCrypt::DecryptBody(tiny);
	for (std::size_t i = 0; i < tiny.size(); ++i)
	{
		CHECK_EQ(static_cast<int>(tiny[i]), 0x5A);
	}
}

MODERN_TEST(WldCrypt_OnePassCannotUnwrapTwo)
{
	// Two applications of the cipher must differ from one, or the
	// double-encrypted fixture would not need a second pass.
	// 0x00, deliberately. 0x11 is a FIXED POINT of this cipher -
	// ((0x11 - 0x10) ^ 0x10) == 0x11 - so a test built on it would pass with a
	// no-op decryptor and fail for the wrong reason.
	std::vector<std::uint8_t> once(200, 0x00);
	std::vector<std::uint8_t> twice = once;
	WldCrypt::DecryptBody(once);
	WldCrypt::DecryptBody(twice);
	WldCrypt::DecryptBody(twice);

	CHECK_NE(static_cast<int>(once[150]), static_cast<int>(twice[150]));
}

// ===========================================================================
// 2. Real assets - the shipped ASURA client
// ===========================================================================

MODERN_TEST(Wld_PlainNavigationMapLoads)
{
	if (!RequireAssets("plain map")) { return; }

	// bambooforest.wld: 1,212 vertices and 404 cells, measured by 002c. Small
	// enough that a per-cell link table mistake shows up as a count, and it is a
	// PLAIN file, so decryptPasses must be 0.
	const NavigationLoadResult result = LoadWldNavigationMesh(AssetPath("bambooforest.wld"));

	if (!result.Ok())
	{
		std::printf("      %s\n", result.error.Format(AssetPath("bambooforest.wld")).c_str());
	}
	CHECK_EQ(static_cast<int>(result.status), static_cast<int>(NavigationLoadStatus::Loaded));
	CHECK(!result.encrypted);
	CHECK_EQ(result.decryptPasses, 0u);
	CHECK(result.mesh != nullptr);

	if (result.mesh != nullptr)
	{
		CHECK_EQ(result.mesh->CellCount(), static_cast<std::size_t>(404));
		CHECK_EQ(result.mesh->VertexCount(), static_cast<std::size_t>(1212));
		CHECK(result.mesh->Built());
	}
}

MODERN_TEST(Wld_EncryptedNavigationMapLoads)
{
	if (!RequireAssets("encrypted map")) { return; }

	// Ground_Lost.wld: one of the 14 obfuscated files. 14,331 vertices, 4,777 cells
	// (002c §5). A correct single pass is the whole test.
	const NavigationLoadResult result = LoadWldNavigationMesh(AssetPath("Ground_Lost.wld"));

	if (!result.Ok())
	{
		std::printf("      %s\n", result.error.Format(AssetPath("Ground_Lost.wld")).c_str());
	}
	CHECK_EQ(static_cast<int>(result.status), static_cast<int>(NavigationLoadStatus::Loaded));
	CHECK(result.encrypted);
	CHECK_EQ(result.decryptPasses, 1u);

	if (result.mesh != nullptr)
	{
		CHECK_EQ(result.mesh->CellCount(), static_cast<std::size_t>(4777));
		CHECK_EQ(result.mesh->VertexCount(), static_cast<std::size_t>(14331));
	}
}

MODERN_TEST(Wld_DoubleEncryptedFixtureLoadsInTwoPasses)
{
	if (!RequireAssets("double-encrypted fixture")) { return; }

	// es_f41_killbillzone01.wld. The single hardest shipped file: 504 vertices and
	// 168 cells after TWO passes (002c §5), with `dwNAVI_MARK` 231,036.
	//
	// This is also the test that pins down why a second pass is needed at all.
	// One pass leaves a perfectly plausible header - the file-type field and the
	// FileID are both plaintext - and fails on the file mark's version DWORD,
	// which decodes to 0x20202120.
	const NavigationLoadResult result =
	    LoadWldNavigationMesh(AssetPath("es_f41_killbillzone01.wld"));

	if (!result.Ok())
	{
		std::printf("      %s\n",
		            result.error.Format(AssetPath("es_f41_killbillzone01.wld")).c_str());
	}
	CHECK_EQ(static_cast<int>(result.status), static_cast<int>(NavigationLoadStatus::Loaded));
	CHECK(result.encrypted);
	CHECK_EQ(result.decryptPasses, 2u);

	if (result.mesh != nullptr)
	{
		CHECK_EQ(result.mesh->CellCount(), static_cast<std::size_t>(168));
		CHECK_EQ(result.mesh->VertexCount(), static_cast<std::size_t>(504));
	}

	// And the one-pass parse must FAIL, on the file mark specifically. If this
	// ever starts succeeding, the second pass is being applied for the wrong
	// reason and the pass count is not measuring what it claims to.
	std::vector<std::uint8_t> raw;
	{
		std::ifstream file(AssetPath("es_f41_killbillzone01.wld"), std::ios::binary);
		raw.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	}

	WldCrypt::DecryptBody(raw);

	WldNavigationData  parsed;
	WldNavigationError error;
	CHECK(!ParseWldNavigation(raw.data(), raw.size(), parsed, error));
	CHECK_EQ(std::string(error.stage), std::string("file mark"));
}

MODERN_TEST(Wld_NoNavigationMapReportsNoNavigation)
{
	if (!RequireAssets("no-navigation map")) { return; }

	// login.wld is one of the 9 shipped files with `bExist == 0` - the
	// character-select and login maps RAN never walks. It must be a SUCCESSFUL
	// load that reports nothing, not a failure.
	const NavigationLoadResult result = LoadWldNavigationMesh(AssetPath("login.wld"));

	CHECK_EQ(static_cast<int>(result.status), static_cast<int>(NavigationLoadStatus::NoNavigation));
	CHECK(result.mesh == nullptr);
	CHECK_EQ(static_cast<int>(result.error.status),
	         static_cast<int>(NavigationLoadStatus::NoNavigation));
}

MODERN_TEST(Wld_UnsupportedFileIdIsRejectedTheWayLegacyRejectsIt)
{
	if (!RequireAssets("unsupported FileID")) { return; }

	// square_rd.wld carries FileID 0x0202, which is not in
	// `NSLANDMAN_SUPPORT::IsLandManSupported` (DxLandManSaveLoad.cpp:3167-3181).
	// Legacy refuses it at GLLandManSet.cpp:41, so refusing it here is
	// compatibility, not a limitation - and the status has to say which check
	// tripped so an operator does not go looking for corruption.
	const NavigationLoadResult result = LoadWldNavigationMesh(AssetPath("square_rd.wld"));

	CHECK_EQ(static_cast<int>(result.status), static_cast<int>(NavigationLoadStatus::InvalidFile));
	CHECK_EQ(std::string(result.error.stage), std::string("FileID"));
	CHECK(result.mesh == nullptr);

	// The rule itself, on the two values that matter.
	CHECK(IsSupportedLandManVersion(0x0114));
	CHECK(IsSupportedLandManVersion(0x0200)); // DxLandMan::VERSION_WLD
	CHECK(!IsSupportedLandManVersion(0x0202));
	CHECK(!IsSupportedLandManVersion(0));
}

MODERN_TEST(Wld_KnownPointSnapsToItsCell)
{
	if (!RequireAssets("snap")) { return; }

	// w_school_01.wld: 13,206 cells, 24,893 vertices (002c §14). Cell 6603's
	// baked centre is (1031.327, -59.215, 997.280), read out of the parsed
	// navigation block rather than computed from geometry.
	const NavigationLoadResult result = LoadWldNavigationMesh(AssetPath("w_school_01.wld"));
	if (!result.Ok())
	{
		std::printf("      %s\n", result.error.Format(AssetPath("w_school_01.wld")).c_str());
		return;
	}

	CHECK_EQ(result.mesh->CellCount(), static_cast<std::size_t>(13206));
	CHECK_EQ(result.mesh->VertexCount(), static_cast<std::size_t>(24893));

	const NavigationCell* cell = result.mesh->GetCellById(6603);
	CHECK(cell != nullptr);
	if (cell == nullptr)
	{
		return;
	}

	const Vector3 centre = cell->CenterPoint();
	CHECK(std::fabs(centre.x - 1031.327f) < 0.01f);
	CHECK(std::fabs(centre.y + 59.215f) < 0.01f);
	CHECK(std::fabs(centre.z - 997.280f) < 0.01f);

	// Snapping the centre must return that same cell, and a Y from the cell's
	// plane - which for a point already on the plane is the point's own Y.
	const Vector3 snapped = result.mesh->SnapPointToCell(6603, centre);
	CHECK_EQ(result.mesh->FindClosestCell(snapped), 6603u);
	CHECK(std::fabs(snapped.y - centre.y) < 0.01f);
}

MODERN_TEST(Wld_KnownValidDestinationPassesNavigationValidation)
{
	if (!RequireAssets("valid destination")) { return; }

	const NavigationLoadResult result = LoadWldNavigationMesh(AssetPath("w_school_01.wld"));
	if (!result.Ok())
	{
		return;
	}

	// Cell 6604, at (1024.451, -59.604, 1000.140) - `link[0]` of the start cell
	// 6603, so it is adjacent and reachable.
	//
	// The GOTO validation is the vertical probe only
	// (GLCharMsg.cpp:293-319): a miss means no path, no movement and no 3035.
	const Vector3 target(1024.451f, -59.604f, 1000.140f);

	Vector3       collision{};
	std::uint32_t cellId = 0;
	CHECK(result.mesh->IsCollision(ProbeTop(target), ProbeBottom(target), collision, &cellId));
	CHECK(cellId != NavigationMesh::kNoLink);

	// The hit must be inside the probe span, or the probe passed for the wrong
	// reason.
	CHECK(collision.y <= ProbeTop(target).y + 0.001f);
	CHECK(collision.y >= ProbeBottom(target).y - 0.001f);

	// And a path must exist to it from the start cell, which is the other half
	// of the validation: found a cell, now walk to it.
	NavigationSearchSession session(result.mesh->CellCount());
	NavigationPath          path;
	const Vector3           start = result.mesh->GetCellById(6603)->CenterPoint();
	CHECK(result.mesh->BuildNavigationPath(path, session, 6603, start, cellId, collision));
	CHECK_GE(path.Size(), static_cast<std::size_t>(2));
}

MODERN_TEST(Wld_KnownInvalidDestinationFailsNavigationValidation)
{
	if (!RequireAssets("invalid destination")) { return; }

	const NavigationLoadResult result = LoadWldNavigationMesh(AssetPath("w_school_01.wld"));
	if (!result.Ok())
	{
		return;
	}

	// Far outside the mesh: XZ extent is [-2013.497, 1460.000] x
	// [-2438.650, 2374.930] (002c §14), so nothing can be under this point and
	// the probe must miss. This is the SILENT failure path - no path, no
	// movement, no 3035 - and it is the case a server must get right, because
	// reporting success here would move a character into the void.
	const Vector3 offMesh(2460.000f, -59.215f, 3374.930f);

	Vector3       collision{};
	std::uint32_t cellId = 0;
	CHECK(!result.mesh->IsCollision(ProbeTop(offMesh), ProbeBottom(offMesh), collision,
	                                &cellId));

	// The vertical cases, measured against the destination floor at y = -59.604
	// (002c §14). The probe spans target.y + 10 to target.y - 10, so a target 10.4
	// units above the floor already misses it.
	const Vector3 inProbe(1024.451f, -59.215f, 1000.140f);   // span [-49.215, -69.215]
	CHECK(result.mesh->IsCollision(ProbeTop(inProbe), ProbeBottom(inProbe), collision,
	                               &cellId));

	const Vector3 outOfProbe(1024.451f, -48.215f, 1000.140f); // span [-38.215, -58.215]
	CHECK(!result.mesh->IsCollision(ProbeTop(outOfProbe), ProbeBottom(outOfProbe), collision,
	                                &cellId));
}

MODERN_TEST(Wld_LinkTableIsPositionalAndSolidEdgesAreSentinel)
{
	if (!RequireAssets("link table")) { return; }

	const NavigationLoadResult result = LoadWldNavigationMesh(AssetPath("w_school_01.wld"));
	if (!result.Ok())
	{
		return;
	}

	// Every link either names a real cell or is the solid sentinel - never
	// anything else. `w_school_01.wld` is one connected component, so it also has
	// both kinds.
	std::size_t solid = 0;
	std::size_t linked = 0;

	for (std::size_t i = 0; i < result.mesh->CellCount(); ++i)
	{
		const NavigationCell* cell = result.mesh->GetCell(i);
		CHECK(cell != nullptr);
		if (cell == nullptr)
		{
			continue;
		}

		for (int side = 0; side < 3; ++side)
		{
			const NavigationCell* link = cell->Link(side);
			if (link == nullptr)
			{
				++solid;
				continue;
			}
			++linked;
			CHECK(link->CellId() < result.mesh->CellCount());
		}
	}

	CHECK_GT(linked, static_cast<std::size_t>(0));
	CHECK_GT(solid, static_cast<std::size_t>(0));
}

MODERN_TEST(Wld_FullAssetScanMatchesTheMeasuredBaseline)
{
	if (!RequireAssets("full asset scan")) { return; }

	// The WORLD-ENTRY-002c baseline, verified per file rather than trusted:
	//
	//   87 .wld files, 14 obfuscated, 77 navigation meshes,
	//   359,366 cells, 625,216 vertices, 0 anomalies
	//
	// The 10 files that do not produce a mesh are 9 with `bExist == 0` plus
	// `square_rd.wld`, whose FileID legacy itself refuses. Those are counted
	// separately because "no navigation" and "legacy would not load this" are
	// different answers and a test that merged them would stop noticing which is
	// which.
	std::vector<std::filesystem::path> wlds;
	for (const auto& entry : std::filesystem::directory_iterator(MapDirectory()))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".wld")
		{
			wlds.push_back(entry.path());
		}
	}
	std::sort(wlds.begin(), wlds.end());

	std::size_t loaded = 0;
	std::size_t noNavigation = 0;
	std::size_t rejected = 0;
	std::size_t encrypted = 0;
	std::size_t twoPass = 0;
	std::size_t cells = 0;
	std::size_t vertices = 0;
	std::size_t anomalies = 0;

	for (const std::filesystem::path& path : wlds)
	{
		const NavigationLoadResult result = LoadWldNavigationMesh(path.string());

		if (result.encrypted)
		{
			++encrypted;
		}
		if (result.decryptPasses == 2)
		{
			++twoPass;
		}

		switch (result.status)
		{
		case NavigationLoadStatus::Loaded:
			++loaded;
			if (result.mesh == nullptr)
			{
				++anomalies;
				std::printf("      ANOMALY %s: Loaded with no mesh\n", path.filename().string().c_str());
				break;
			}
			cells += result.mesh->CellCount();
			vertices += result.mesh->VertexCount();

			// Per-file invariants, so a regression names the file.
			if (result.mesh->CellCount() == 0 || result.mesh->VertexCount() == 0)
			{
				++anomalies;
				std::printf("      ANOMALY %s: empty mesh\n", path.filename().string().c_str());
			}
			break;

		case NavigationLoadStatus::NoNavigation:
			++noNavigation;
			break;

		case NavigationLoadStatus::InvalidFile:
			++rejected;
			// Only square_rd.wld, and only for its FileID.
			if (std::string(result.error.stage) != std::string("FileID"))
			{
				++anomalies;
				std::printf("      ANOMALY %s: rejected at stage %s (%s)\n",
				            path.filename().string().c_str(), result.error.stage.c_str(),
				            result.error.detail.c_str());
			}
			break;

		default:
			++anomalies;
			std::printf("      ANOMALY %s: %s at %s\n", path.filename().string().c_str(),
			            ToString(result.status), result.error.stage.c_str());
			break;
		}
	}

	CHECK_EQ(wlds.size(), static_cast<std::size_t>(87));
	CHECK_EQ(encrypted, static_cast<std::size_t>(14));
	CHECK_EQ(twoPass, static_cast<std::size_t>(1));
	CHECK_EQ(loaded, static_cast<std::size_t>(77));
	CHECK_EQ(noNavigation, static_cast<std::size_t>(9));
	CHECK_EQ(rejected, static_cast<std::size_t>(1));
	CHECK_EQ(cells, static_cast<std::size_t>(359366));
	CHECK_EQ(vertices, static_cast<std::size_t>(625216));
	CHECK_EQ(anomalies, static_cast<std::size_t>(0));

	// 77 + 9 + 1 == 87: every shipped file is accounted for, none silently
	// dropped.
	CHECK_EQ(loaded + noNavigation + rejected, wlds.size());
}

MODERN_TEST(Wld_EveryShippedMapIsEitherLoadedOrExplained)
{
	if (!RequireAssets("per-file explanation")) { return; }

	// The nine `bExist == 0` files, named. WORLD-ENTRY-002c §6.1 lists them, and
	// naming them here means a change to WHICH files lack navigation is a test
	// failure rather than a count that happens to still add up.
	const char* expectedNoNavigation[] = {
	    "character1_slt.wld", "character_slt.wld",  "character_slt_main.wld",
	    "character_slt_old.wld", "character_slt_s01.wld", "character_slt_s02.wld",
	    "character_slt_s03.wld", "log_in.wld",      "login.wld",
	};

	for (const char* name : expectedNoNavigation)
	{
		const NavigationLoadResult result = LoadWldNavigationMesh(AssetPath(name));
		if (result.status != NavigationLoadStatus::NoNavigation)
		{
			std::printf("      %s -> %s (%s)\n", name, ToString(result.status),
			            result.error.stage.c_str());
		}
		CHECK_EQ(static_cast<int>(result.status),
		         static_cast<int>(NavigationLoadStatus::NoNavigation));
	}
}

MODERN_TEST(Wld_AssetDirectoryIsConfigured)
{
	// If RAN_ASSET_ROOT is set but unusable, that is a CONFIGURATION error and
	// must fail loudly - otherwise every asset test above silently skips and the
	// run looks green.
	MapDirectory();

	if (g_availability == AssetAvailability::RootUnset)
	{
		++g_assetTestsSkipped;
		std::printf("      RAN_ASSET_ROOT is not set: the asset tests above DID NOT RUN\n");
		return;
	}

	CHECK(g_availability == AssetAvailability::Available);
	CHECK(std::filesystem::is_directory(MapDirectory()));
}

// ===========================================================================
// 3. Synthetic malformed input
// ===========================================================================

MODERN_TEST(Wld_SyntheticValidFileLoads)
{
	// The control for every malformed case below: if the good fixture does not
	// load, "the reader rejected it" proves nothing.
	SynthWld synth;

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());

	if (!result.Ok())
	{
		std::printf("      %s\n", result.error.Format("synthetic").c_str());
	}
	CHECK_EQ(static_cast<int>(result.status), static_cast<int>(NavigationLoadStatus::Loaded));

	if (result.mesh != nullptr)
	{
		CHECK_EQ(result.mesh->CellCount(), static_cast<std::size_t>(1));
		CHECK_EQ(result.mesh->VertexCount(), static_cast<std::size_t>(3));
	}
}

MODERN_TEST(Wld_TruncatedInputIsRejectedNotRead)
{
	// Every prefix of a valid file, short enough to matter. Each must produce a
	// controlled status and never a crash - this is the loop that would catch an
	// out-of-bounds read that a single truncation point happened to miss.
	SynthWld synth;
	const std::size_t full = synth.bytes.size();

	for (std::size_t size = 0; size < full; size += 7)
	{
		const NavigationLoadResult result =
		    BuildNavigationMeshFromBuffer(synth.bytes.data(), size);

		CHECK(!result.Ok());
		CHECK(result.mesh == nullptr);

		const int status = static_cast<int>(result.status);
		CHECK(status == static_cast<int>(NavigationLoadStatus::InvalidFile) ||
		      status == static_cast<int>(NavigationLoadStatus::Truncated) ||
		      status == static_cast<int>(NavigationLoadStatus::Inconsistent));
	}

	// And the full length still works, so the loop is not passing because
	// everything fails.
	const NavigationLoadResult ok =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), full);
	CHECK(ok.Ok());
}

MODERN_TEST(Wld_TruncatedCellBlockIsRejected)
{
	SynthWld synth;

	// Cut in the middle of the 188-byte cell record: the cell count promises one
	// cell and the bytes are not there.
	const std::size_t cellRecordStart = synth.naviBlock + 4 + 4 + 36 + 4;
	synth.bytes.resize(cellRecordStart + 100);

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("cell records"));
	CHECK_EQ(static_cast<int>(result.error.status), static_cast<int>(NavigationLoadStatus::Truncated));
}

MODERN_TEST(Wld_TruncatedLinkTableIsRejected)
{
	SynthWld synth;
	// Keep the cell record, drop the 12-byte link table.
	const std::size_t linkStart = synth.naviBlock + 4 + 4 + 36 + 4 + NavigationCell::kCellRecordBytes;
	synth.bytes.resize(linkStart);

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("cell links"));
	CHECK_EQ(static_cast<int>(result.error.status), static_cast<int>(NavigationLoadStatus::Truncated));
}

MODERN_TEST(Wld_BogusCellCountDoesNotAllocate)
{
	SynthWld synth;

	// 0x7FFFFFFF cells. A reader that multiplies before bounds-checking asks for
	// 64 GB here; this one must notice that the bytes are not there.
	const std::size_t cellCountAt = synth.naviBlock + 4 + 4 + 36;
	synth.PutU32(cellCountAt, 0x7FFFFFFFu);

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("cell records"));
	CHECK_EQ(static_cast<int>(result.error.status), static_cast<int>(NavigationLoadStatus::Truncated));
}

MODERN_TEST(Wld_NegativeVertexCountIsRejected)
{
	SynthWld synth;

	// `int m_nNaviVertex`, so a negative count is representable. Read as
	// unsigned it would become ~4 billion and ask for 51 GB.
	const std::size_t vertexCountAt = synth.naviBlock + 4;
	synth.PutU32(vertexCountAt, 0xFFFFFFFFu);

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("vertex count"));
	CHECK_EQ(static_cast<int>(result.error.status), static_cast<int>(NavigationLoadStatus::Inconsistent));
}

MODERN_TEST(Wld_NavigationMarkerOverflowIsRejected)
{
	SynthWld synth;

	// `132 + dwNAVI_MARK` must not wrap, and a mark past the end must be a
	// failure rather than a read at the end of the file.
	synth.PutU32(272, 0xFFFFFFFFu);
	const NavigationLoadResult overflow =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!overflow.Ok());
	CHECK(std::string(overflow.error.stage) == std::string("navigation marker"));
	CHECK_EQ(static_cast<int>(overflow.error.status),
	         static_cast<int>(NavigationLoadStatus::Inconsistent));

	synth.PutU32(272, 0x7FFFFFFFu);
	const NavigationLoadResult past =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!past.Ok());
	CHECK(std::string(past.error.stage) == std::string("navigation marker"));
}

MODERN_TEST(Wld_InvalidFileTypeIsRejected)
{
	SynthWld synth;
	std::memcpy(synth.bytes.data(), "NOTAWLD!", 8);

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("file type"));
	CHECK_EQ(static_cast<int>(result.error.status), static_cast<int>(NavigationLoadStatus::InvalidFile));
}

MODERN_TEST(Wld_UnsupportedFileMarkVersionIsRejected)
{
	SynthWld synth;

	// 0x20202120 - four ASCII spaces - is what ONE decryption pass on the
	// double-encrypted fixture decodes its file-mark version to. Asserted here so
	// the reason that fixture needs two passes is pinned in the test suite rather
	// than only in a comment.
	synth.PutU32(264, 0x20202120u);
	const NavigationLoadResult spaces =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!spaces.Ok());
	CHECK_EQ(std::string(spaces.error.stage), std::string("file mark"));
	CHECK_EQ(static_cast<int>(spaces.error.status),
	         static_cast<int>(NavigationLoadStatus::UnsupportedFormat));

	// And a genuinely unknown version takes the same path, which is what
	// `SLAND_FILEMARK::LoadSet` does (DxLandDef.cpp:46-50).
	synth.PutU32(264, 0x0999u);
	const NavigationLoadResult unknown =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!unknown.Ok());
	CHECK_EQ(static_cast<int>(unknown.error.status),
	         static_cast<int>(NavigationLoadStatus::UnsupportedFormat));
}

MODERN_TEST(Wld_CellIdMustEqualItsIndex)
{
	SynthWld synth;

	// The invariant `NavigationMesh::GetCell` and the id comparisons inside
	// `QueryForPath` both depend on. A wrong record stride produces exactly this
	// failure, so it is worth proving the reader catches it rather than builds a
	// subtly wrong mesh.
	const std::size_t cellAt = synth.naviBlock + 4 + 4 + 36 + 4;
	synth.PutU32(cellAt + NavigationCell::kOffsetCellId, 7u);

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("cell records"));
	CHECK_EQ(static_cast<int>(result.error.status), static_cast<int>(NavigationLoadStatus::Inconsistent));
}

MODERN_TEST(Wld_OutOfRangeVertexIndexIsRejected)
{
	SynthWld synth;

	const std::size_t cellAt = synth.naviBlock + 4 + 4 + 36 + 4;
	synth.PutU32(cellAt + NavigationCell::kOffsetVertex + 8, 99u);

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("cell records"));
	CHECK_EQ(static_cast<int>(result.error.status), static_cast<int>(NavigationLoadStatus::Inconsistent));
}

MODERN_TEST(Wld_OutOfRangeLinkIdIsRejected)
{
	SynthWld synth;

	// A present flag with an id no cell has. Legacy would pass this to `GetCell`,
	// which tests `size() < index` and then calls `.at(index)` - so id == count
	// throws and a larger id reads out of bounds.
	const std::size_t linkAt = synth.naviBlock + 4 + 4 + 36 + 4 + NavigationCell::kCellRecordBytes;
	synth.PutU32(linkAt, 1u);          // present
	synth.PutU32(linkAt + 4, 500u);    // id

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("cell links"));
	CHECK_EQ(static_cast<int>(result.error.status), static_cast<int>(NavigationLoadStatus::Inconsistent));
}

MODERN_TEST(Wld_NonBooleanLinkFlagIsRejected)
{
	SynthWld synth;

	const std::size_t linkAt = synth.naviBlock + 4 + 4 + 36 + 4 + NavigationCell::kCellRecordBytes;
	synth.PutU32(linkAt, 0xDEADBEEFu);

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("cell links"));
}

MODERN_TEST(Wld_NonBooleanExistFlagIsRejected)
{
	SynthWld synth;

	// `bExist` is a BOOL. Anything else means the navigation offset is wrong or
	// the body is still ciphered - and reporting it as "no navigation" would turn
	// a decryption failure into a silently unwalkable map.
	synth.PutU32(synth.naviBlock, 0x7FFFFFFFu);

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("bExist"));
	CHECK_EQ(static_cast<int>(result.error.status), static_cast<int>(NavigationLoadStatus::Inconsistent));
}

MODERN_TEST(Wld_ExistFlagOneWithZeroCellsIsRejected)
{
	SynthWld synth;

	// `bExist == 1` and no cells. `NavigationMesh::LoadFile` would create an empty
	// mesh and `MakeAABBTree` would then walk a null root.
	const std::size_t cellCountAt = synth.naviBlock + 4 + 4 + 36;
	synth.PutU32(cellCountAt, 0u);

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	CHECK(!result.Ok());
	CHECK_EQ(std::string(result.error.stage), std::string("cell count"));
	CHECK_EQ(static_cast<int>(result.error.status), static_cast<int>(NavigationLoadStatus::Inconsistent));
}

MODERN_TEST(Wld_GarbageEncryptedInputFailsCleanlyAndWithinThePassCeiling)
{
	// A buffer that CLAIMS to be encrypted - correct `Land.Man` tag - and is
	// otherwise noise. The reader must try at most `kMaxDecryptPasses` and then
	// report `DecryptionFailed` rather than looping until the bytes look
	// plausible, which is how a loader ends up "successfully" decoding noise.
	std::vector<std::uint8_t> noise(4096);
	for (std::size_t i = 0; i < noise.size(); ++i)
	{
		noise[i] = static_cast<std::uint8_t>((i * 31u + 7u) & 0xFFu);
	}
	std::memcpy(noise.data(), "Land.Man", 8);
	noise[128] = 0x14;
	noise[129] = 0x01;

	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(noise.data(), noise.size());

	CHECK(!result.Ok());
	CHECK(result.mesh == nullptr);
	CHECK_EQ(static_cast<int>(result.status), static_cast<int>(NavigationLoadStatus::DecryptionFailed));
	CHECK(result.decryptPasses <= static_cast<std::uint32_t>(kMaxDecryptPasses));
	CHECK(result.error.detail.size() < 256); // no buffer dumps in a message
}

MODERN_TEST(Wld_NullAndTinyBuffersAreRejected)
{
	CHECK(!BuildNavigationMeshFromBuffer(nullptr, 1024).Ok());
	CHECK(!BuildNavigationMeshFromBuffer(nullptr, 0).Ok());

	std::vector<std::uint8_t> tiny(16, 0u);
	CHECK(!BuildNavigationMeshFromBuffer(tiny.data(), tiny.size()).Ok());
	CHECK_EQ(static_cast<int>(BuildNavigationMeshFromBuffer(tiny.data(), tiny.size()).status),
	         static_cast<int>(NavigationLoadStatus::InvalidFile));
}

MODERN_TEST(Wld_MissingFileIsReportedNotCrashed)
{
	const NavigationLoadResult result =
	    LoadWldNavigationMesh("this-file-does-not-exist-002d.wld");

	CHECK(!result.Ok());
	CHECK(result.mesh == nullptr);
	CHECK_EQ(std::string(result.error.stage), std::string("open"));
}

MODERN_TEST(Wld_TheSameBufferLoadsIdenticallyTwice)
{
	// The loader copies before it decrypts, so a caller can reuse its buffer -
	// and two loads of one buffer must agree. If the loader decrypted in place
	// this would fail, which is the point.
	SynthWld synth;

	const NavigationLoadResult first =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	const std::vector<std::uint8_t> afterFirst = synth.bytes;
	const NavigationLoadResult second =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());

	CHECK(first.Ok());
	CHECK(second.Ok());
	CHECK(synth.bytes == afterFirst);
	CHECK_EQ(first.mesh->CellCount(), second.mesh->CellCount());
	CHECK_EQ(first.mesh->VertexCount(), second.mesh->VertexCount());
}

MODERN_TEST(Wld_LoadedMeshIsImmutableAndReusableAcrossSessions)
{
	// The thread-safety claim, stated as a test: two searches over ONE mesh with
	// SEPARATE sessions must both find the same path, and the mesh must be
	// unchanged afterwards. Under legacy's per-cell A* state the two searches
	// would corrupt each other.
	SynthWld synth;
	const NavigationLoadResult result =
	    BuildNavigationMeshFromBuffer(synth.bytes.data(), synth.bytes.size());
	if (!result.Ok())
	{
		return;
	}

	const NavigationCell* cell = result.mesh->GetCellById(0);
	if (cell == nullptr)
	{
		CHECK(false);
		return;
	}

	NavigationSearchSession sessionA(result.mesh->CellCount());
	NavigationSearchSession sessionB(result.mesh->CellCount());
	NavigationPath          pathA;
	NavigationPath          pathB;

	const Vector3 from = cell->CenterPoint();
	const Vector3 to   = Vector3{10.0f, 0.0f, 10.0f};

	CHECK(result.mesh->BuildNavigationPath(pathA, sessionA, 0, from, 0, to));
	CHECK(result.mesh->BuildNavigationPath(pathB, sessionB, 0, from, 0, to));
	CHECK_EQ(pathA.Size(), pathB.Size());
	// Start and goal are the same cell, so the waypoint walk adds nothing
	// (navigationmesh.cpp:204: `while (TestCell && TestCell != EndCell)` never
	// runs) and the list is just the start point from `Setup` plus the end point
	// from `EndPath`. Two entries, not one - the degenerate case worth pinning,
	// because a path with no waypoints at all would leave a character with
	// nothing to walk to.
	CHECK_EQ(pathA.Size(), static_cast<std::size_t>(2));

	// Queries stay available after the searches, i.e. nothing was consumed.
	Vector3       collision{};
	std::uint32_t id = 0;
	CHECK(result.mesh->IsCollision(Vector3{1.0f, 5.0f, 1.0f}, Vector3{1.0f, -5.0f, 1.0f},
	                               collision, &id));
}
