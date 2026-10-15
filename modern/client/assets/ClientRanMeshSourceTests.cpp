// ASURA-CHARACTER-001C: RAN mesh source routing and material tests.
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

#include <algorithm>
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

// ASURA-CHARACTER-001C recovery assertions.
//
// What the raw bytes show, and what the decoder now does with them:
//
//   * four Mesh containers -- m2_hb_body, m2_hb_leg, m2_hb_foot, m2_hb_hand
//   * each with its own MeshMaterialList whose first two integers are
//     (materialCount, faceIndexCount); the body list is (2, 418)
//   * each list carries NO named material references
//   * the textures live on UNNAMED Material instances whose TextureFilename
//     children are m1_hb_body.dds, m2_bs_body.DDS, m1_khb_foot.dds,
//     m1_hb_leg.dds and m1_hb_hand.dds
//
// Two separate defects had to be fixed, and they are independent:
//
//   1. NESTED MATERIALS SHARED ONE TEXTURE SLOT. The decoder held a single
//      pending texture string, so a Material nested inside another completed
//      first and took the shared slot, leaving the outer one with no texture.
//      It now keeps one slot per open Material.
//
//   2. UNNAMED LISTS WERE FORCED ONTO THE FALLBACK. XMeshDecoder resolves a
//      list's slots by material NAME, and its unnamed-list path (added for
//      b_3boss_01.x, which genuinely has no materials) mapped every slot onto
//      one shared untextured material -- which is why the body drew in the
//      magenta diagnostic colour.
//
// For (2) the positional rule was established from the file's bytes, not
// assumed. Instrumenting the parse showed that ALL FOUR lists CLOSE BEFORE THE
// FIRST `Material` IS RECORDED, so no rule keyed on a list's position relative
// to its own materials can work -- every such rule finds zero. What the format
// does support, and what the file confirms, is a SEQUENTIAL CURSOR IN DOCUMENT
// ORDER: walk the lists in parse order and let each consume the next
// `materialCount` instances. With counts 2, 1, 1, 1 over five instances that
// reproduces every mapping exactly:
//
//     body list (2) -> m1_hb_body.dds, m2_bs_body.DDS
//     foot list (1) -> m1_khb_foot.dds
//     leg  list (1) -> m1_hb_leg.dds
//     hand list (1) -> m1_hb_hand.dds

MODERN_TEST(RanMeshSource_RealAsuraBrawlerBodyRecoversItsTextures)
{
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        std::printf("    SKIP (no RAN_ASSET_ROOT): s_m_hb.X material recovery\n");
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

    // The raw bytes still carry the real texture filenames -- nothing is lost
    // in reading or transforming the file. This was ASURA-CHARACTER-001B's
    // evidence that the gap was in the decoder, not in the asset.
    const std::string raw(plain.GetValue().AsStringView());
    for (const char* texture : { "m1_hb_body.dds", "m1_hb_leg.dds",
                                 "m1_khb_foot.dds", "m1_hb_hand.dds",
                                 "m2_bs_body.DDS" })
    {
        CHECK(raw.find(texture) != std::string::npos);
    }

    // The gap is closed: every filename the mesh declares is now reachable
    // through the decoded material table. Zero was the pinned value.
    size_t textured = 0;
    for (const MeshMaterial& material : asset.GetMaterials())
    {
        std::printf("    s_m_hb.X material '%s' -> '%s'\n",
                    material.name.c_str(), material.textureFilename.c_str());
        if (!material.textureFilename.empty()) { ++textured; }
    }
    std::printf("    s_m_hb.X materials: %zu total, %zu textured\n",
                asset.GetMaterials().size(), textured);
    CHECK_EQ(textured, size_t{5});
    CHECK_EQ(asset.GetMaterials().size(), size_t{5});

    // Distinct textures must stay distinct entries. Every material here is
    // UNNAMED, so the empty name is shared by all five -- the name cannot be
    // what keeps them apart, and a merge on name is exactly the failure this
    // milestone had to avoid.
    for (size_t a = 0; a < asset.GetMaterials().size(); ++a)
    {
        for (size_t b = a + 1; b < asset.GetMaterials().size(); ++b)
        {
            CHECK(!(asset.GetMaterials()[a] == asset.GetMaterials()[b]));
        }
    }

    // The recovered names are exactly the ones TextureLoader resolves over the
    // real asset root, so the application needs no new lookup path.
    TextureLoader loader;
    REQUIRE(loader.Initialize(root / "textures").IsOk());
    for (const MeshMaterial& material : asset.GetMaterials())
    {
        const std::filesystem::path resolved = loader.Find(material.textureFilename);
        std::printf("    %-18s -> %s\n", material.textureFilename.c_str(),
                    resolved.empty() ? "UNRESOLVED" : resolved.string().c_str());
        CHECK(!resolved.empty());
    }
}

MODERN_TEST(RanMeshSource_RealAsuraBrawlerSectionsKeepDistinctMaterials)
{
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        std::printf("    SKIP (no RAN_ASSET_ROOT): s_m_hb.X section slots\n");
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

    REQUIRE(asset.HasSections());
    CHECK_EQ(asset.GetSections().size(), size_t{5});

    // Sections must tile the index buffer exactly and every slot must exist.
    uint32_t covered = 0;
    std::vector<uint32_t> used;
    for (const MeshSection& section : asset.GetSections())
    {
        CHECK(section.indexOffset == covered);
        CHECK(section.materialIndex < asset.GetMaterials().size());
        CHECK_GT(section.indexCount, uint32_t{0});
        CHECK_EQ(section.indexCount % 3, uint32_t{0});
        used.push_back(section.materialIndex);
        covered += section.indexCount;
    }
    CHECK_EQ(covered, static_cast<uint32_t>(asset.GetIndexCount()));

    // All five slots in use, none collapsed onto a shared entry: this is what
    // "the material index assigned to every face is preserved" means in
    // practice.
    std::vector<uint32_t> distinct(used);
    std::sort(distinct.begin(), distinct.end());
    distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());
    std::printf("    distinct material slots used by %zu sections: %zu\n",
                used.size(), distinct.size());
    CHECK_EQ(distinct.size(), size_t{5});

    // And the textures the sections draw are distinct too, which is the point:
    // body, body-alt, foot, leg and hand must not share one image.
    for (size_t s = 0; s < used.size(); ++s)
    {
        for (size_t t = s + 1; t < used.size(); ++t)
        {
            CHECK(asset.GetMaterials()[used[s]].textureFilename !=
                  asset.GetMaterials()[used[t]].textureFilename);
        }
    }
}

MODERN_TEST(RanMeshSource_RealBossMeshKeepsTheMaterialFreeFallback)
{
    // `data/skeleton/b_3boss_01.x` is the large unnamed-list regression asset,
    // and it is NOT the same shape as the Brawler body: 116 lists against three
    // Material instances, so almost every list declares no material at all.
    //
    // The cursor is only trusted when it accounts for the whole file, which is
    // why it binds those three here without disturbing anything else -- and
    // why the lists that declared no slots must all land on ONE shared
    // untextured material rather than on 113 separate entries or on an empty
    // slot vector.
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        std::printf("    SKIP (no RAN_ASSET_ROOT): b_3boss_01.x\n");
        return;
    }
    std::vector<uint8_t> bytes;
    if (!ReadFile(root / "data" / "skeleton" / "b_3boss_01.x", bytes))
    {
        std::printf("    SKIP: b_3boss_01.x absent\n");
        return;
    }

    const Result<ResourceData> plain = RanMeshSource::ToPlainX(ResourceData(bytes));
    if (plain.IsError())
    {
        std::printf("    SKIP: b_3boss_01.x refused by the source router\n");
        return;
    }
    XMeshDecoder decoder;
    const Result<MeshAsset> mesh = decoder.DecodeMesh(plain.GetValue());
    REQUIRE(mesh.IsOk());
    const MeshAsset& asset = mesh.GetValue();

    std::printf("    b_3boss_01.x: %zu materials, %zu sections\n",
                asset.GetMaterials().size(), asset.GetSections().size());

    // One material table for the whole file, not one per list: 116 lists must
    // not become 116 entries.
    CHECK_LT(asset.GetMaterials().size(), asset.GetSections().size());

    // Internally consistent: every slot exists and the sections tile exactly.
    uint32_t covered = 0;
    for (const MeshSection& section : asset.GetSections())
    {
        CHECK(section.materialIndex < asset.GetMaterials().size());
        covered += section.indexCount;
    }
    CHECK_EQ(covered, static_cast<uint32_t>(asset.GetIndexCount()));

    // Exactly one shared untextured material, and it is what the overwhelming
    // majority of sections draw. This is the ASURA-CHARACTER-001B behaviour for
    // a list that declared no slots, and it must survive.
    std::vector<uint32_t> fallbackUse;
    size_t untexturedSlots = 0;
    for (size_t m = 0; m < asset.GetMaterials().size(); ++m)
    {
        if (asset.GetMaterials()[m].textureFilename.empty()) { ++untexturedSlots; }
    }
    CHECK_EQ(untexturedSlots, size_t{1});

    for (const MeshSection& section : asset.GetSections())
    {
        if (asset.GetMaterials()[section.materialIndex].textureFilename.empty())
        {
            fallbackUse.push_back(section.materialIndex);
        }
    }
    std::printf("    b_3boss_01.x: %zu of %zu sections use the shared "
                "untextured fallback\n",
                fallbackUse.size(), asset.GetSections().size());
    CHECK_GT(fallbackUse.size(), asset.GetSections().size() / 2);
    for (uint32_t slot : fallbackUse) { CHECK_EQ(slot, fallbackUse.front()); }
}

MODERN_TEST(RanMeshSource_MalformedMaterialDataIsRefusedNotHalfBound)
{
    // Truncating the real body mesh must never yield a mesh whose sections
    // reference slots that do not exist, and must never throw. Either the
    // decoder refuses, or what it returns is self-consistent.
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        std::printf("    SKIP (no RAN_ASSET_ROOT): s_m_hb.X truncation sweep\n");
        return;
    }
    std::vector<uint8_t> bytes;
    if (!ReadFile(root / "data" / "skin" / "s_m_hb.X", bytes))
    {
        std::printf("    SKIP: s_m_hb.X absent\n");
        return;
    }
    REQUIRE(bytes.size() > 1024);

    const size_t step = bytes.size() / 24;
    size_t accepted = 0;
    for (size_t length = 0; length < bytes.size(); length += step)
    {
        XMeshDecoder decoder;
        const Result<MeshAsset> mesh =
            decoder.DecodeMesh(ResourceData(std::vector<uint8_t>(
                bytes.begin(), bytes.begin() + static_cast<long>(length))));
        if (mesh.IsError()) { continue; }
        ++accepted;

        const MeshAsset& asset = mesh.GetValue();
        uint32_t covered = 0;
        for (const MeshSection& section : asset.GetSections())
        {
            CHECK(section.materialIndex < asset.GetMaterials().size());
            covered += section.indexCount;
        }
        CHECK_EQ(covered, static_cast<uint32_t>(asset.GetIndexCount()));
    }
    std::printf("    s_m_hb.X truncation sweep: %zu of %zu prefixes accepted\n",
                accepted, bytes.size() / step + 1);
}

MODERN_TEST(RanMeshSource_RealAsuraLuffyRegressionIsUnchanged)
{
    // The named-reference path is untouched by ASURA-CHARACTER-001C, so every
    // figure CLIENT-RENDER-002 recorded must still hold exactly.
    const std::filesystem::path root = RealAssetRoot();
    if (root.empty())
    {
        std::printf("    SKIP (no RAN_ASSET_ROOT): m_luffy_timeskip.mxf regression\n");
        return;
    }
    std::vector<uint8_t> bytes;
    if (!ReadFile(root / "data" / "skin" / "m_luffy_timeskip.mxf", bytes))
    {
        std::printf("    SKIP: m_luffy_timeskip.mxf absent\n");
        return;
    }

    const Result<ResourceData> plain = RanMeshSource::ToPlainX(ResourceData(bytes));
    REQUIRE(plain.IsOk());
    XMeshDecoder decoder;
    const Result<MeshAsset> mesh = decoder.DecodeMesh(plain.GetValue());
    REQUIRE(mesh.IsOk());
    const MeshAsset& asset = mesh.GetValue();

    std::printf("    m_luffy_timeskip.mxf: %zu v, %zu i, %zu tri, %zu materials, "
                "%zu sections\n",
                asset.GetVertexCount(), asset.GetIndexCount(),
                asset.GetTriangleCount(), asset.GetMaterials().size(),
                asset.GetSections().size());

    CHECK_EQ(asset.GetVertexCount(), size_t{8868});
    CHECK_EQ(asset.GetIndexCount(), size_t{36741});
    CHECK_EQ(asset.GetTriangleCount(), size_t{12247});
    CHECK_EQ(asset.GetMaterials().size(), size_t{17});
    CHECK_EQ(asset.GetSections().size(), size_t{22});

    // Every material is still reached by NAME here -- the path this milestone
    // deliberately did not change -- and still resolves.
    TextureLoader loader;
    REQUIRE(loader.Initialize(root / "textures").IsOk());
    std::vector<std::string> distinctTextures;
    for (const MeshMaterial& material : asset.GetMaterials())
    {
        CHECK(!material.name.empty());
        const std::filesystem::path resolved = loader.Find(material.textureFilename);
        std::printf("    %-34s -> %s\n", material.textureFilename.c_str(),
                    resolved.empty() ? "UNRESOLVED" : resolved.string().c_str());
        CHECK(!resolved.empty());

        bool seen = false;
        for (const std::string& existing : distinctTextures)
        {
            if (existing == material.textureFilename) { seen = true; }
        }
        if (!seen) { distinctTextures.push_back(material.textureFilename); }
    }
    CHECK_EQ(distinctTextures.size(), size_t{2});

    uint32_t covered = 0;
    for (const MeshSection& section : asset.GetSections())
    {
        CHECK(section.indexOffset == covered);
        covered += section.indexCount;
    }
    CHECK_EQ(covered, static_cast<uint32_t>(asset.GetIndexCount()));
}

int main()
{
    std::printf("Modern ASURA-CHARACTER-001C RAN mesh source tests\n\n");

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
