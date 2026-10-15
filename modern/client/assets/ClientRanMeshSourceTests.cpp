// ASURA-CHARACTER-001B: RAN mesh source routing tests.
//
// RAN ships the same DirectX `.x` content in two containers: an obfuscated
// `.mxf`, and a plain `.X` with no container at all. The Male Brawler base
// body is the plain kind -- `data/skin/s_m_hb.X` starts `xof 0303bin 0032`
// with nothing in front of it -- while the existing regression asset
// `data/skin/m_luffy_timeskip.mxf` is the container kind.
//
// These cases pin down the routing: plain `.X` passes through byte-identical,
// `.mxf` still goes through `MxfMeshTransform`, and the decision comes from
// the bytes rather than from the filename. The `.mxf` half also re-asserts
// the CLIENT-013 transform round trip so a regression there cannot hide behind
// the new routing.

#include "TestHarness.h"

#include "assets/MxfMeshTransform.h"
#include "assets/RanMaterialSlot.h"
#include "assets/TextureLoader.h"
#include "assets/RanMeshSource.h"
#include "resources/ResourceData.h"
#include "assets/XMeshDecoder.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

namespace
{

// A RAN skin .X stream prefix. The decoder needs the full 16-byte header to
// get past its own check, so this is the header plus a token stream that the
// decoder will then refuse -- which is fine: the routing tests are about
// which bytes arrive at the decoder, not about the decoder's verdict.
ResourceData PlainXBytes()
{
    const char* prefix = "xof 0303bin 0032";
    std::vector<uint8_t> bytes(prefix, prefix + std::char_traits<char>::length(prefix));
    bytes.resize(64, 0);
    return ResourceData(bytes);
}

// The inverse of MxfMeshTransform, for building a valid container.
//   transform : plain -> (plain + 0xEA) ^ 0xEB
//   so encode  : plain -> (plain ^ 0xEB) - 0xEA
ResourceData MxfBytes(const ResourceData& plain)
{
    std::vector<uint8_t> encoded(plain.GetSize());
    for (size_t i = 0; i < plain.GetSize(); ++i)
    {
        const uint8_t value = static_cast<uint8_t>(plain.GetData()[i] ^ 0xEB);
        encoded[i] = static_cast<uint8_t>((value - 0xEA) & 0xFF);
    }

    std::vector<uint8_t> container;
    container.resize(12, 0);
    container[0] = 0x00; container[1] = 0x01; container[2] = 0x00; container[3] = 0x00; // 0x100
    const uint32_t payloadSize = static_cast<uint32_t>(encoded.size());
    container[4]  = static_cast<uint8_t>(payloadSize & 0xFF);
    container[5]  = static_cast<uint8_t>((payloadSize >> 8) & 0xFF);
    container[6]  = static_cast<uint8_t>((payloadSize >> 16) & 0xFF);
    container[7]  = static_cast<uint8_t>((payloadSize >> 24) & 0xFF);
    // bytes 8..11 fileType = 0 (skin), already zero.
    container.insert(container.end(), encoded.begin(), encoded.end());
    return ResourceData(std::move(container));
}

std::filesystem::path RealAssetRoot()
{
    #if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    const char* root = std::getenv("RAN_ASSET_ROOT");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    if (root == nullptr || root[0] == '\0') { return {}; }
    return std::filesystem::path(root);
}

bool ReadFile(const std::filesystem::path& path, std::vector<uint8_t>& out)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) { return false; }
    out.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return !out.empty();
}

} // namespace

// ===========================================================================
// Classification
// ===========================================================================

MODERN_TEST(RanMeshSource_PlainXIsClassifiedAsPlain)
{
    const Result<RanMeshContainer> kind = RanMeshSource::Classify(PlainXBytes());
    REQUIRE(kind.IsOk());
    CHECK(kind.GetValue() == RanMeshContainer::PlainX);
}

MODERN_TEST(RanMeshSource_ContainerIsClassifiedAsMxf)
{
    const Result<RanMeshContainer> kind =
        RanMeshSource::Classify(MxfBytes(PlainXBytes()));
    REQUIRE(kind.IsOk());
    CHECK(kind.GetValue() == RanMeshContainer::Mxf);
}

MODERN_TEST(RanMeshSource_ShortInputIsRefused)
{
    CHECK(RanMeshSource::Classify(ResourceData()).IsError());
    CHECK(RanMeshSource::Classify(ResourceData(std::vector<uint8_t>{ 'x', 'o' })).IsError());
    CHECK(!RanMeshSource::IsPlainX(ResourceData()));
}

MODERN_TEST(RanMeshSource_ContainersHaveStableNames)
{
    CHECK_EQ(std::string(ToString(RanMeshContainer::PlainX)), std::string("plain .X"));
    CHECK_EQ(std::string(ToString(RanMeshContainer::Mxf)),    std::string(".mxf container"));
}

// ===========================================================================
// Normalisation
// ===========================================================================

MODERN_TEST(RanMeshSource_PlainXPassesThroughUnchanged)
{
    const ResourceData plain = PlainXBytes();
    const Result<ResourceData> out = RanMeshSource::ToPlainX(plain);
    REQUIRE(out.IsOk());
    CHECK_EQ(out.GetValue().GetSize(), plain.GetSize());
    CHECK(out.GetValue() == plain);
}

MODERN_TEST(RanMeshSource_MxfStillGoesThroughTheTransform)
{
    const ResourceData plain = PlainXBytes();
    const Result<ResourceData> out = RanMeshSource::ToPlainX(MxfBytes(plain));
    REQUIRE(out.IsOk());
    // The transform is exactly the CLIENT-013 one, so this doubles as a
    // round-trip regression for that boundary.
    CHECK(out.GetValue() == plain);
}

MODERN_TEST(RanMeshSource_MalformedContainerIsRefusedNotPassedThrough)
{
    // Neither a plain .X nor a valid .mxf: the first bytes are 'Z'.
    std::vector<uint8_t> junk = { 'Z', 'Z', 'Z', 'Z', 0, 0, 0, 0, 0, 0, 0, 0, 0x01 };
    CHECK(RanMeshSource::ToPlainX(ResourceData(std::move(junk))).IsError());
}

// ===========================================================================
// Real ASURA assets
// ===========================================================================

MODERN_TEST(RanMeshSource_RealAsuraBrawlerBodyIsPlainX)
{
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        std::printf("    SKIP (no RAN_ASSET_ROOT): data/skin/s_m_hb.X\n");
        return;
    }
    std::vector<uint8_t> bytes;
    const std::filesystem::path path = root / "data" / "skin" / "s_m_hb.X";
    if (!ReadFile(path, bytes))
    {
        std::printf("    SKIP: s_m_hb.X absent\n");
        return;
    }

    const ResourceData data(bytes);
    CHECK(RanMeshSource::IsPlainX(data));
    const Result<ResourceData> plain = RanMeshSource::ToPlainX(data);
    REQUIRE(plain.IsOk());
    // Passed through byte-for-byte: no container was stripped or invented.
    CHECK(plain.GetValue().GetSize() == bytes.size());
}

MODERN_TEST(RanMeshSource_RealAsuraLuffyMxfStillWorksAsARegressionAsset)
{
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        std::printf("    SKIP (no RAN_ASSET_ROOT): data/skin/m_luffy_timeskip.mxf\n");
        return;
    }
    std::vector<uint8_t> bytes;
    const std::filesystem::path path = root / "data" / "skin" / "m_luffy_timeskip.mxf";
    if (!ReadFile(path, bytes))
    {
        std::printf("    SKIP: m_luffy_timeskip.mxf absent\n");
        return;
    }

    const ResourceData data(bytes);
    CHECK(!RanMeshSource::IsPlainX(data));

    const Result<ResourceData> plain = RanMeshSource::ToPlainX(data);
    REQUIRE(plain.IsOk());
    CHECK(RanMeshSource::IsPlainX(plain.GetValue()));
}

MODERN_TEST(RanMeshSource_RealAsuraBrawlerBodyDecodesToASectionedMesh)
{
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        std::printf("    SKIP (no RAN_ASSET_ROOT): decoding data/skin/s_m_hb.X\n");
        return;
    }
    std::vector<uint8_t> bytes;
    if (!ReadFile(root / "data" / "skin" / "s_m_hb.X", bytes))
    {
        std::printf("    SKIP: s_m_hb.X absent\n");
        return;
    }

    const Result<ResourceData> plain = RanMeshSource::ToPlainX(ResourceData(bytes));
    REQUIRE(plain.IsOk());

    XMeshDecoder decoder;
    const Result<MeshAsset> mesh = decoder.DecodeMesh(plain.GetValue());
    REQUIRE(mesh.IsOk());

    const MeshAsset& asset = mesh.GetValue();
    std::printf("    s_m_hb.X: %zu vertices, %zu indices, %zu triangles, "
                "%zu materials, %zu sections\n",
                asset.GetVertexCount(), asset.GetIndexCount(), asset.GetTriangleCount(),
                asset.GetMaterials().size(), asset.GetSections().size());

    CHECK_GT(asset.GetVertexCount(), size_t{0});
    CHECK_GT(asset.GetIndexCount(), size_t{0});
    CHECK_EQ(asset.GetIndexCount() % 3, size_t{0});

    // Every section must name a material that exists, and the sections must
    // tile the index buffer exactly: that is the contract the GPU upload and
    // the draw loop rely on.
    CHECK(asset.HasSections());
    uint32_t covered = 0;
    for (const MeshSection& section : asset.GetSections())
    {
        CHECK(section.indexOffset == covered);
        CHECK(section.materialIndex < asset.GetMaterials().size());
        CHECK_GT(section.indexCount, uint32_t{0});
        covered += section.indexCount;
    }
    CHECK_EQ(covered, static_cast<uint32_t>(asset.GetIndexCount()));

    // Positions must be finite: both the camera framing and the GPU upload
    // depend on it, and it is the cheapest proof the geometry is real.
    bool finite = true;
    for (const MeshVertex& vertex : asset.GetVertices())
    {
        if (!vertex.IsFinite()) { finite = false; break; }
    }
    CHECK(finite);
}

// A KNOWN, DELIBERATELY PINNED GAP -- ASURA-CHARACTER-001B.
//
// The RAN body mesh decodes structurally (above), but its textures do NOT
// come out of the decoder. This is not an accident of the fixture and not
// something the application can work around: it is a shape XMeshDecoder does
// not yet resolve, and this test records it so it cannot be forgotten.
//
// What the file actually contains (verified byte by byte in
// data/skin/s_m_hb.X):
//
//   * four Mesh containers -- m2_hb_body, m2_hb_leg, m2_hb_foot, m2_hb_hand
//   * each with its own MeshMaterialList whose first two integers are
//     (materialCount, faceIndexCount); the body list is (2, 418)
//   * each list carries NO named material references
//   * the textures live on UNNAMED Material instances declared after the
//     list: Material -> TextureFilename -> "m1_hb_body.dds" and
//     Material -> TextureFilename -> "m2_bs_body.DDS", and likewise
//     m1_khb_foot.dds / m1_hb_leg.dds / m1_hb_hand.dds for the others
//
// XMeshDecoder resolves a list's slots by material NAME, and its unnamed-list
// path (added for b_3boss_01.x, which has no materials at all) maps every
// slot onto one shared untextured material. So s_m_hb.X arrives with one
// unnamed, untextured material and every section draws the diagnostic
// fallback colour.
//
// The correct binding is positional -- list slot i belongs to the i-th
// unnamed Material declared for that mesh -- but implementing it means
// changing shared material resolution inside XMeshDecoder, which this
// milestone was told not to rewrite. It is recorded here and left as the next
// task.
MODERN_TEST(RanMeshSource_RealAsuraBrawlerBodyMaterialGapIsPinned)
{
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        std::printf("    SKIP (no RAN_ASSET_ROOT): s_m_hb.X material extraction\n");
        return;
    }
    std::vector<uint8_t> bytes;
    if (!ReadFile(root / "data" / "skin" / "s_m_hb.X", bytes))
    {
        std::printf("    SKIP: s_m_hb.X absent\n");
        return;
    }

    const Result<ResourceData> plain = RanMeshSource::ToPlainX(ResourceData(bytes));
    REQUIRE(plain.IsOk());
    XMeshDecoder decoder;
    const Result<MeshAsset> mesh = decoder.DecodeMesh(plain.GetValue());
    REQUIRE(mesh.IsOk());

    // The raw bytes DO carry the real texture filenames, so nothing is lost
    // in reading or transforming the file.
    const std::string raw(plain.GetValue().AsStringView());
    for (const char* texture : { "m1_hb_body.dds", "m1_hb_leg.dds",
                                 "m1_khb_foot.dds", "m1_hb_hand.dds",
                                 "m2_bs_body.DDS" })
    {
        CHECK(raw.find(texture) != std::string::npos);
    }

    // ...but the decoded material table does not surface them yet. Zero is
    // the CURRENT value, pinned on purpose: when the decoder is taught this
    // shape, this test fails and the expectation moves to the real textures.
    size_t textured = 0;
    for (const MeshMaterial& material : mesh.GetValue().GetMaterials())
    {
        if (!material.textureFilename.empty()) { ++textured; }
    }
    std::printf("    s_m_hb.X materials: %zu total, %zu with a texture filename "
                "(KNOWN GAP: expected 0 until XMeshDecoder binds unnamed "
                "Material instances positionally)\n",
                mesh.GetValue().GetMaterials().size(), textured);
    CHECK_EQ(textured, size_t{0});

    // The filenames themselves are valid bare image names, so the moment the
    // decoder surfaces them the existing TextureLoader resolves them with no
    // further work. Assert that on the names taken from the file.
    TextureLoader loader;
    REQUIRE(loader.Initialize(root / "textures").IsOk());
    for (const char* texture : { "m1_hb_body.dds", "m1_hb_leg.dds",
                                 "m1_khb_foot.dds", "m1_hb_hand.dds",
                                 "m2_bs_body.DDS" })
    {
        const std::filesystem::path resolved = loader.Find(texture);
        std::printf("    %s -> %s\n", texture,
                    resolved.empty() ? "UNRESOLVED" : resolved.string().c_str());
        CHECK(!resolved.empty());
    }
}

int main()
{
    std::printf("Modern ASURA-CHARACTER-001B RAN mesh source tests\n\n");

    const int failedCases = ModernTests::RunAll();

    if (failedCases == 0)
    {
        std::printf("\nAll %d test cases passed.\n",
                    static_cast<int>(ModernTests::Registry().size()));
        return 0;
    }

    std::printf("\n%d of %d test cases FAILED (%d checks).\n",
                failedCases,
                static_cast<int>(ModernTests::Registry().size()),
                ModernTests::FailureCount());
    return 1;
}
