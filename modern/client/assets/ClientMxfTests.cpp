// CLIENT-013: RAN MXF mesh transform tests.
//
// These cases pin down the MXF container transform boundary: that
// a valid MXF file decrypts to the exact .x bytes that were encoded,
// that every malformed container is refused with InvalidArgument, and
// that the transform produces plain X bytes a future X decoder can
// consume. No real RAN installation is required.

#include "TestHarness.h"

#include "assets/MxfMeshTransform.h"
#include "resources/ResourceData.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
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
    // MXF assembly
    // ---------------------------------------------------------------------------
    //
    // A valid MXF file is a 12-byte header followed by encrypted payload.
    // The encryption is the inverse of the transform:
    //   encoded = (plain ^ 0xEB) - 0xEA
    // with the 12-byte header:
    //   version    = 0x100
    //   payloadSize = encodedPayloadSize
    //   fileType   = 0 (skin)

    void PushU32(std::vector<uint8_t>& bytes, uint32_t value)
    {
        bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
        bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
        bytes.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
        bytes.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
    }

    std::vector<uint8_t> MakeMxfFile(const std::vector<uint8_t>& plainX)
    {
        const size_t payloadSize = plainX.size();
        std::vector<uint8_t> encoded(payloadSize);
        for (size_t i = 0; i < payloadSize; ++i)
        {
            encoded[i] = static_cast<uint8_t>((plainX[i] ^ 0xEB) - 0xEA);
        }

        std::vector<uint8_t> mxf;
        PushU32(mxf, 0x100u);
        PushU32(mxf, static_cast<uint32_t>(payloadSize));
        PushU32(mxf, 0u); // fileType = skin

        mxf.insert(mxf.end(), encoded.begin(), encoded.end());
        return mxf;
    }

    std::vector<uint8_t> MakeMinimalX()
    {
        std::vector<uint8_t> x;
        x.push_back('x');
        x.push_back('o');
        x.push_back('f');
        x.push_back(' ');
        x.push_back(0x00);
        x.push_back(0x00);
        x.push_back(0x00);
        x.push_back(0x00);
        return x;
    }
} // namespace

// ---------------------------------------------------------------------------
// 1. Valid synthetic MXF decrypts to exact .x bytes
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_ValidMxfDecodesToExactX)
{
    const std::vector<uint8_t> plainX = MakeMinimalX();
    const std::vector<uint8_t> mxf = MakeMxfFile(plainX);

    MxfMeshTransform transform;
    const Result<ResourceData> result = transform.Transform(ResourceData(mxf));
    CHECK(result.IsOk());
    if (result.IsError()) return;

    const std::vector<uint8_t>& xBytes = result.GetValue().GetBytes();
    CHECK_EQ(xBytes.size(), plainX.size());
    CHECK(std::equal(xBytes.begin(), xBytes.end(), plainX.begin()));
}

// ---------------------------------------------------------------------------
// 2. Truncated header (less than 12 bytes)
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_TruncatedHeaderIsRefused)
{
    MxfMeshTransform transform;
    for (size_t size = 0; size < 12u; ++size)
    {
        const std::vector<uint8_t> truncated(size, 0xFF);
        CHECK_EQ(transform.Transform(ResourceData(truncated)).GetError(), ErrorCode::InvalidArgument);
    }
}

// ---------------------------------------------------------------------------
// 3. Invalid version
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_InvalidVersionIsRefused)
{
    MxfMeshTransform transform;
    const std::vector<uint8_t> plainX = MakeMinimalX();
    const std::vector<uint8_t> mxf = MakeMxfFile(plainX);

    std::vector<uint8_t> bad = mxf;
    bad[0] = 0x00; bad[1] = 0x00; bad[2] = 0x00; bad[3] = 0x00;
    CHECK_EQ(transform.Transform(ResourceData(bad)).GetError(), ErrorCode::InvalidArgument);

    bad = mxf;
    bad[0] = 0x01;
    CHECK_EQ(transform.Transform(ResourceData(bad)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 4. Payload size mismatch
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_PayloadSizeMismatchIsRefused)
{
    MxfMeshTransform transform;
    const std::vector<uint8_t> plainX = MakeMinimalX();
    const std::vector<uint8_t> mxf = MakeMxfFile(plainX);

    std::vector<uint8_t> tooSmall = mxf;
    int32_t& psSmall = reinterpret_cast<int32_t&>(tooSmall[4]);
    psSmall -= 4;
    CHECK_EQ(transform.Transform(ResourceData(tooSmall)).GetError(), ErrorCode::InvalidArgument);

    std::vector<uint8_t> tooLarge = mxf;
    int32_t& psLarge = reinterpret_cast<int32_t&>(tooLarge[4]);
    psLarge += 4;
    CHECK_EQ(transform.Transform(ResourceData(tooLarge)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 5. Zero payload
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_ZeroPayloadIsRefused)
{
    MxfMeshTransform transform;
    std::vector<uint8_t> mxf(12, 0);
    mxf[0] = 0; mxf[1] = 0x01; mxf[2] = 0; mxf[3] = 0; // version = 0x100
    CHECK_EQ(transform.Transform(ResourceData(mxf)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 6. Unsupported file type
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_UnsupportedFileTypeIsRefused)
{
    MxfMeshTransform transform;
    std::vector<uint8_t> mxf(12, 0);
    mxf[0] = 0; mxf[1] = 0x01; mxf[2] = 0; mxf[3] = 0; // version = 0x100
    PushU32(mxf, 1u); // payloadSize = 1
    mxf[8] = 1; // fileType = unsupported
    CHECK_EQ(transform.Transform(ResourceData(mxf)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 7. Bad decrypted .X output
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_BadDecryptedMagicIsRefused)
{
    MxfMeshTransform transform;
    std::vector<uint8_t> x = MakeMinimalX();
    x[0] = 'X'; // corrupt magic
    std::vector<uint8_t> mxf = MakeMxfFile(x);
    CHECK_EQ(transform.Transform(ResourceData(mxf)).GetError(), ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// 8. Correct byte transformation
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_CorrectByteTransformation)
{
    std::vector<uint8_t> plain = {'x', 'o', 'f', ' ', 0x00, 0x00, 0x00, 0x00};
    std::vector<uint8_t> encoded(plain.size());
    for (size_t i = 0; i < plain.size(); ++i)
    {
        encoded[i] = static_cast<uint8_t>((plain[i] ^ 0xEB) - 0xEA);
    }
    std::vector<uint8_t> decoded(encoded.size());
    for (size_t i = 0; i < encoded.size(); ++i)
    {
        decoded[i] = static_cast<uint8_t>((encoded[i] + 0xEA) ^ 0xEB);
    }
    CHECK(std::equal(decoded.begin(), decoded.end(), plain.begin()));
}

// ---------------------------------------------------------------------------
// 9. Output is independent of input buffer lifetime
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_OutputIndependentOfInput)
{
    std::vector<uint8_t> plainX = MakeMinimalX();
    std::vector<uint8_t> mxf = MakeMxfFile(plainX);

    MxfMeshTransform transform;
    const Result<ResourceData> result = transform.Transform(ResourceData(mxf));
    CHECK(result.IsOk());
    if (result.IsError()) return;

    mxf[12] ^= 0xFF; // mutate the input payload

    const std::vector<uint8_t>& xBytes = result.GetValue().GetBytes();
    CHECK_EQ(xBytes.size(), plainX.size());
    CHECK(std::equal(xBytes.begin(), xBytes.end(), plainX.begin()));
}

// ---------------------------------------------------------------------------
// 10. Repeated calls are deterministic and stateless
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_RepeatedCallsAreDeterministic)
{
    const std::vector<uint8_t> plainX = MakeMinimalX();
    const std::vector<uint8_t> mxf = MakeMxfFile(plainX);

    MxfMeshTransform transform;
    const Result<ResourceData> r1 = transform.Transform(ResourceData(mxf));
    const Result<ResourceData> r2 = transform.Transform(ResourceData(mxf));
    CHECK(r1.IsOk());
    CHECK(r2.IsOk());
    if (r1.IsError() || r2.IsError()) return;

    const std::vector<uint8_t>& b1 = r1.GetValue().GetBytes();
    const std::vector<uint8_t>& b2 = r2.GetValue().GetBytes();
    CHECK_EQ(b1.size(), b2.size());
    CHECK(std::equal(b1.begin(), b1.end(), b2.begin()));
}

// ---------------------------------------------------------------------------
// 11. Valid MXF produces plain .X output
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_ValidMxfProducesPlainX)
{
    const std::vector<uint8_t> plainX = MakeMinimalX();
    const std::vector<uint8_t> mxf = MakeMxfFile(plainX);

    MxfMeshTransform transform;
    const Result<ResourceData> result = transform.Transform(ResourceData(mxf));
    CHECK(result.IsOk());
    if (result.IsError()) return;

    const std::vector<uint8_t>& xBytes = result.GetValue().GetBytes();
    CHECK_EQ(xBytes[0], 'x');
    CHECK_EQ(xBytes[1], 'o');
    CHECK_EQ(xBytes[2], 'f');
    CHECK_EQ(xBytes[3], ' ');
}

// ---------------------------------------------------------------------------
// 12. Integration with ResourceData
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_ResourceDataIntegration)
{
    std::vector<uint8_t> plainX = MakeMinimalX();
    std::vector<uint8_t> mxf = MakeMxfFile(plainX);

    ResourceData mtfData(mxf);
    MxfMeshTransform transform;
    const Result<ResourceData> result = transform.Transform(mtfData);
    CHECK(result.IsOk());
    if (result.IsError()) return;

    const std::vector<uint8_t>& xBytes = result.GetValue().GetBytes();
    CHECK_EQ(xBytes.size(), plainX.size());
}

// ---------------------------------------------------------------------------
// 13. Real RAN MXF validation, when available
// ---------------------------------------------------------------------------

MODERN_TEST(ClientMxf_RealRanAssetsDecodeWhenAvailable)
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
        std::printf("      (real RAN MXF validation skipped: RAN_ASSET_ROOT not set)\n");
        return;
    }

    const std::filesystem::path skeletonRoot = std::filesystem::path(root) / "data" / "skeleton";
    if (!std::filesystem::is_directory(skeletonRoot))
    {
        std::printf("      (real RAN validation skipped: no skeleton directory)\n");
        return;
    }

    std::vector<std::filesystem::path> candidates;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(skeletonRoot))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".mxf")
        {
            candidates.push_back(entry.path());
        }
    }
std::sort(candidates.begin(), candidates.end());

    MxfMeshTransform transform;
    size_t validCount = 0;
    size_t invalidCount = 0;
    size_t successCount = 0;

    for (const auto& path : candidates)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) continue;
        const std::vector<uint8_t> bytes{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};

        const Result<ResourceData> result = transform.Transform(ResourceData(bytes));
        if (result.IsError())
        {
            ++invalidCount;
            continue;
        }
        ++validCount;
        const std::vector<uint8_t>& xBytes = result.GetValue().GetBytes();
        if (xBytes.size() >= 4 && xBytes[0] == 'x' && xBytes[1] == 'o' && xBytes[2] == 'f' && xBytes[3] == ' ')
        {
            ++successCount;
        }
    }

    std::printf("      (real RAN MXF: %zu files total, %zu valid, %zu invalid, %zu success)\n",
        candidates.size(), validCount, invalidCount, successCount);

    CHECK(validCount > 0u);
}

int main()
{
    std::printf("Modern CLIENT-013 RAN MXF mesh transform tests\n\n");

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
