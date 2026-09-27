// CLIENT-012: RAN MTF texture transform tests.
//
// These cases pin down the MTF container transform boundary: that
// a valid MTF file decrypts to the exact DDS bytes that were encoded,
// that every malformed container is refused with InvalidArgument, and
// that the transform produces valid DDS bytes the existing decoder
// can consume. No real RAN installation is required.

#include "TestHarness.h"

#include "assets/MtfTextureTransform.h"
#include "assets/DdsImageDecoder.h"
#include "assets/ImageAsset.h"
#include "assets/ImageDecoder.h"
#include "resources/ResourceData.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

namespace
{
    // ---------------------------------------------------------------------------
    // MTF assembly
    // ---------------------------------------------------------------------------
    //
    // A valid MTF file is a 12-byte header followed by encrypted payload.
    // The encryption is the inverse of the transform:
    //   encoded = (plain ^ 0x26) - 0x09
    // with the 12-byte header:
    //   version    = 0x100
    //   payloadSize = encodedPayloadSize
    //   fileType   = 0 (DDS)

    void PushU16(std::vector<uint8_t>& bytes, uint16_t value)
    {
        bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
        bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
    }

    void PushU32(std::vector<uint8_t>& bytes, uint32_t value)
    {
        bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
        bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
        bytes.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
        bytes.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
    }

    // Encrypt a plain DDS payload into MTF format.
    std::vector<uint8_t> MakeMtfFile(const std::vector<uint8_t>& plainDds)
    {
        const size_t payloadSize = plainDds.size();
        std::vector<uint8_t> encoded(payloadSize);
        for (size_t i = 0; i < payloadSize; ++i)
        {
            encoded[i] = static_cast<uint8_t>((plainDds[i] ^ 0x26) - 0x09);
        }

        std::vector<uint8_t> mtf;
        // Header: int32 version (0x100), int32 payloadSize, int32 fileType (0 = DDS)
        PushU32(mtf, 0x100u);
        PushU32(mtf, static_cast<uint32_t>(payloadSize));
        PushU32(mtf, 0u); // fileType = DDS

        mtf.insert(mtf.end(), encoded.begin(), encoded.end());
        return mtf;
    }

    // Creates a minimal valid DDS file (4x4 DXT1).
    std::vector<uint8_t> MakeMinimalDds()
    {
        std::vector<uint8_t> dds;
        dds.push_back('D');
        dds.push_back('D');
        dds.push_back('S');
        dds.push_back(' ');
        PushU32(dds, 124u); // dwSize
        PushU32(dds, 0x00001007u); // dwFlags
        PushU32(dds, 4u); // dwHeight
        PushU32(dds, 4u); // dwWidth
        PushU32(dds, 8u); // dwPitchOrLinearSize
        PushU32(dds, 0u); // dwDepth
        PushU32(dds, 0u); // dwMipMapCount
        for (int i = 0; i < 11; ++i) PushU32(dds, 0u);
        PushU32(dds, 32u); // ddspf.dwSize
        PushU32(dds, 0x4u); // ddspf.dwFlags (FOURCC)
        PushU32(dds, 0x31545844u); // 'D','X','T','1'
        PushU32(dds, 0u); // dwRGBBitCount
        PushU32(dds, 0u); // masks
        PushU32(dds, 0u);
        PushU32(dds, 0u);
        PushU32(dds, 0u);
        PushU32(dds, 0x00001000u); // dwCaps
        PushU32(dds, 0u); // dwCaps2
        PushU32(dds, 0u); // dwCaps3
        PushU32(dds, 0u); // dwCaps4
        PushU32(dds, 0u); // dwReserved2
        PushU32(dds, 0xF800u); // colour0: red
        PushU32(dds, 0x001Fu); // colour1: blue
        PushU32(dds, 0u); // indices
        return dds;
    }

    // Creates a DDS file with wrong magic (not 'D','D','S',' ').
    std::vector<uint8_t> MakeBadMagicDds()
    {
        auto dds = MakeMinimalDds();
        dds[0] = 'X';
        return dds;
    }
} // namespace

// ---------------------------------------------------------------------------
// 1. Valid synthetic MTF -> exact DDS byte recovery
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_ValidMtfDecodesToExactDds)
{
    const std::vector<uint8_t> plainDds = MakeMinimalDds();
    const std::vector<uint8_t> mtf = MakeMtfFile(plainDds);

    MtfTextureTransform transform;
    const Result<ResourceData> result = transform.Transform(ResourceData(mtf));
    CHECK(result.IsOk());
    if (result.IsError()) return;

    const std::vector<uint8_t>& ddsBytes = result.GetValue().GetBytes();
    CHECK_EQ(ddsBytes.size(), plainDds.size());
    CHECK(std::equal(ddsBytes.begin(), ddsBytes.end(), plainDds.begin()));
}

// ---------------------------------------------------------------------------
// 2. Truncated header
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_TruncatedHeaderIsRefused)
{
    MtfTextureTransform transform;
    for (size_t size = 0; size < 12u; ++size)
    {
        const std::vector<uint8_t> truncated(size, 0xFF);
        const Result<ResourceData> result = transform.Transform(ResourceData(truncated));
        CHECK_EQ(result.GetError(), ErrorCode::InvalidArgument);
    }
}

// ---------------------------------------------------------------------------
// 3. Invalid version
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_InvalidVersionIsRefused)
{
    MtfTextureTransform transform;
    const std::vector<uint8_t> dds = MakeMinimalDds();
    const std::vector<uint8_t> mtf = MakeMtfFile(dds);

    // Version 0
    std::vector<uint8_t> bad = mtf;
    bad[0] = 0x00; bad[1] = 0x00; bad[2] = 0x00; bad[3] = 0x00;
    CHECK_EQ(transform.Transform(ResourceData(bad)).GetError(), ErrorCode::InvalidArgument);

    // Version 1
    bad = mtf;
    bad[0] = 0x01;
    CHECK_EQ(transform.Transform(ResourceData(bad)).GetError(), ErrorCode::InvalidArgument);

    // Version 0xFFFF
    bad = mtf;
    bad[0] = 0xFF; bad[1] = 0xFF; bad[2] = 0xFF; bad[3] = 0xFF;
    CHECK_EQ(transform.Transform(ResourceData(bad)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 4. Payload size mismatch
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_PayloadSizeMismatchIsRefused)
{
    MtfTextureTransform transform;
    const std::vector<uint8_t> dds = MakeMinimalDds();
    const std::vector<uint8_t> mtf = MakeMtfFile(dds);

    // Payload size too small: header says 4 bytes less
    std::vector<uint8_t> tooSmall = mtf;
    int32_t& psSmall = reinterpret_cast<int32_t&>(tooSmall[4]);
    psSmall -= 4;
    CHECK_EQ(transform.Transform(ResourceData(tooSmall)).GetError(), ErrorCode::InvalidArgument);

    // Payload size too large
    std::vector<uint8_t> tooLarge = mtf;
    int32_t& psLarge = reinterpret_cast<int32_t&>(tooLarge[4]);
    psLarge += 4;
    CHECK_EQ(transform.Transform(ResourceData(tooLarge)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 5. Truncated payload
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_TruncatedPayloadIsRefused)
{
    MtfTextureTransform transform;
    const std::vector<uint8_t> plainDds = MakeMinimalDds();
    std::vector<uint8_t> mtf = MakeMtfFile(plainDds);

    // Cut 1 byte from the end
    mtf.pop_back();
    CHECK_EQ(transform.Transform(ResourceData(mtf)).GetError(), ErrorCode::InvalidArgument);

    // Cut all but header
    std::vector<uint8_t> headerOnly(mtf.begin(), mtf.begin() + 12);
    CHECK_EQ(transform.Transform(ResourceData(headerOnly)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 6. Extra trailing bytes
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_TrailingBytesAreRefused)
{
    MtfTextureTransform transform;
    const std::vector<uint8_t> plainDds = MakeMinimalDds();
    std::vector<uint8_t> mtf = MakeMtfFile(plainDds);
    mtf.push_back(0xAB);
    CHECK_EQ(transform.Transform(ResourceData(mtf)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 7. Zero payload
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_ZeroPayloadIsRefused)
{
    MtfTextureTransform transform;
    std::vector<uint8_t> mtf(12, 0);
    mtf[0] = 0; mtf[1] = 0x01; mtf[2] = 0; mtf[3] = 0; // version = 0x100
    CHECK_EQ(transform.Transform(ResourceData(mtf)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 8. Unsupported TGA type
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_UnsupportedTgaIsRefused)
{
    MtfTextureTransform transform;
    const std::vector<uint8_t> plainDds = MakeMinimalDds();
    std::vector<uint8_t> mtf = MakeMtfFile(plainDds);
    mtf[8] = 1; // fileType = TGA
    CHECK_EQ(transform.Transform(ResourceData(mtf)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 9. Unsupported BMP type
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_UnsupportedBmpIsRefused)
{
    MtfTextureTransform transform;
    const std::vector<uint8_t> plainDds = MakeMinimalDds();
    std::vector<uint8_t> mtf = MakeMtfFile(plainDds);
    mtf[8] = 2; // fileType = BMP
    CHECK_EQ(transform.Transform(ResourceData(mtf)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 10. Unknown file type
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_UnknownFileTypeIsRefused)
{
    MtfTextureTransform transform;
    const std::vector<uint8_t> plainDds = MakeMinimalDds();
    std::vector<uint8_t> mtf = MakeMtfFile(plainDds);
    mtf[8] = 0xFF; // unknown fileType
    CHECK_EQ(transform.Transform(ResourceData(mtf)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 11. Bad decrypted DDS magic
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_BadDecryptedMagicIsRefused)
{
    MtfTextureTransform transform;
    std::vector<uint8_t> dds = MakeMinimalDds();
    dds[0] = 'X'; // corrupt magic
    dds[1] = 'X';
    std::vector<uint8_t> mtf = MakeMtfFile(dds);
    CHECK_EQ(transform.Transform(ResourceData(mtf)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 12. Valid MTF -> DdsImageDecoder -> ImageAsset
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_MtfThroughDdsDecoderToImageAsset)
{
    const std::vector<uint8_t> plainDds = MakeMinimalDds();
    const std::vector<uint8_t> mtf = MakeMtfFile(plainDds);

    // Step 1: MTF transform
    MtfTextureTransform transform;
    const Result<ResourceData> ddsResult = transform.Transform(ResourceData(mtf));
    CHECK(ddsResult.IsOk());
    if (ddsResult.IsError()) return;

    // Step 2: DdsImageDecoder
    DdsImageDecoder decoder;
    const Result<ImageAsset> imageResult = decoder.DecodeImage(ddsResult.GetValue());
    CHECK(imageResult.IsOk());
    if (imageResult.IsError()) return;

    const ImageAsset& image = imageResult.GetValue();
    CHECK_EQ(image.GetWidth(), 4u);
    CHECK_EQ(image.GetHeight(), 4u);
    CHECK_EQ(image.GetFormat(), ImageFormat::R8G8B8A8_UNorm);
}

// ---------------------------------------------------------------------------
// 13. Invalid MTF must not reach DDS decoding as valid data
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_InvalidMtfMustNotReachDdsDecoder)
{
    // An MTF with fileType = 1 (TGA) must not produce DDS bytes
    // that happen to look like valid DDS to the decoder.
    MtfTextureTransform transform;
    const std::vector<uint8_t> plainDds = MakeMinimalDds();
    std::vector<uint8_t> mtf = MakeMtfFile(plainDds);
    mtf[8] = 1; // TGA
    const Result<ResourceData> result = transform.Transform(ResourceData(mtf));
    CHECK(result.IsError());
}

// ---------------------------------------------------------------------------
// 14. Real RAN asset validation, when available
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMtf_RealRanAssetsDecodeWhenAvailable)
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
        std::printf("      (real RAN MTF validation skipped: RAN_ASSET_ROOT not set)\n");
        return;
    }

    const std::filesystem::path textureRoot = std::filesystem::path(root) / "textures";
    if (!std::filesystem::is_directory(textureRoot))
    {
        std::printf("      (real RAN validation skipped: no textures directory)\n");
        return;
    }

    // Find MTF files
    std::vector<std::filesystem::path> candidates;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(textureRoot))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".mtf")
        {
            candidates.push_back(entry.path());
        }
    }
    std::sort(candidates.begin(), candidates.end());
    if (candidates.size() > 50u) candidates.resize(50u);

    MtfTextureTransform transform;
    DdsImageDecoder decoder;
    size_t validCount = 0;
    size_t invalidCount = 0;
    size_t decodedCount = 0;

    for (const auto& path : candidates)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) continue;
        std::vector<uint8_t> bytes{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};

        const Result<ResourceData> result = transform.Transform(ResourceData(bytes));
        if (result.IsError())
        {
            ++invalidCount;
            continue;
        }
        ++validCount;

        const Result<ImageAsset> decoded = decoder.DecodeImage(result.GetValue());
        if (decoded.IsOk()) ++decodedCount;
    }

    std::printf("      (real RAN MTF: %zu files, %zu valid, %zu invalid, %zu decoded)\n",
        candidates.size(), validCount, invalidCount, decodedCount);

    CHECK(validCount > 0u);
}

int main()
{
    std::printf("Modern CLIENT-012 RAN MTF texture transform tests\n\n");

    const int failedCases = ModernTests::RunAll();

    if (failedCases == 0)
    {
        std::printf("\nAll %d test cases passed.\n", static_cast<int>(ModernTests::Registry().size()));
        return 0;
    }

    std::printf("\n%d of %d test cases FAILED (%d checks).\n",
        failedCases,
        static_cast<int>(ModernTests::Registry().size()),
        ModernTests::FailureCount());
    return 1;
}
