// ASURA-TEXTURE-001: Targa / BMP decoders, content dispatch, the texture cache
// and the ASURA inventory sweep.
//
// CLIENT-RENDER-002 covered PNG, DDS and MTF. ASURA-TEXTURE-001 adds the other
// two signed/parseable image formats the ASURA `textures/` tree actually ships
// -- Targa (.tga) and Windows BMP (.bmp) -- wires them into the content-magic
// dispatch, adds a decode-once `TextureCache`, and proves the whole pipeline
// against the real asset tree when `RAN_ASSET_ROOT` names one.
//
// The Targa and BMP fixtures below are assembled byte by byte from the format
// descriptions (the Targa 18-byte header + descriptor origin/RLE rules; the
// Windows BITMAPFILEHEADER + BITMAPINFOHEADER row-stride/orientation rules).
// Nothing here calls a decoder to produce input for a decoder: the expected
// pixels are written as literals, so a decoder that mirrored them would prove
// nothing. The real-asset case is separate and reads only a developer-supplied
// tree, so ordinary CI stays hermetic and no proprietary asset is committed.

#include "TestHarness.h"

#include "assets/BmpImageDecoder.h"
#include "assets/TgaImageDecoder.h"
#include "assets/TextureCache.h"
#include "assets/TextureLoader.h"
#include "resources/ResourceData.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

namespace
{

void PushU16(std::vector<uint8_t>& v, uint16_t x)
{
	v.push_back(static_cast<uint8_t>(x & 0xFFu));
	v.push_back(static_cast<uint8_t>((x >> 8) & 0xFFu));
}

void PushU32(std::vector<uint8_t>& v, uint32_t x)
{
	v.push_back(static_cast<uint8_t>(x & 0xFFu));
	v.push_back(static_cast<uint8_t>((x >> 8) & 0xFFu));
	v.push_back(static_cast<uint8_t>((x >> 16) & 0xFFu));
	v.push_back(static_cast<uint8_t>((x >> 24) & 0xFFu));
}

void PushI32(std::vector<uint8_t>& v, int32_t x)
{
	PushU32(v, static_cast<uint32_t>(x));
}

// Asserts one RGBA pixel of a decoded image (layout: row-major, top row first).
void CheckPixel(const ImageAsset& image, uint32_t x, uint32_t y,
                uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	const size_t at = (static_cast<size_t>(y) * image.GetWidth() + x) * 4u;
	const std::vector<uint8_t>& px = image.GetPixels();
	CHECK_EQ(px[at + 0], r);
	CHECK_EQ(px[at + 1], g);
	CHECK_EQ(px[at + 2], b);
	CHECK_EQ(px[at + 3], a);
}

// ---------------------------------------------------------------------------
// Targa fixtures
// ---------------------------------------------------------------------------

// Builds an uncompressed truecolour/gray Targa. `rows` are the pixel rows in
// file order (row 0 first): they are the BOTTOM row unless `topOrigin` is set,
// which is what the origin flag means. `channels` is 3, 4 or 1 (gray).
std::vector<uint8_t> MakeTgaUncompressed(uint16_t width, uint16_t height,
                                         uint8_t bitsPerPixel, bool topOrigin,
                                         const std::vector<uint8_t>& body,
                                         bool withFooter)
{
	std::vector<uint8_t> t;
	t.push_back(0);   // id length
	t.push_back(0);   // colour map type: none
	t.push_back(bitsPerPixel == 8 ? 3 : 2);  // image type: gray=3, truecolour=2
	PushU16(t, 0);    // colour map first index
	PushU16(t, 0);    // colour map length
	t.push_back(0);   // colour map entry bits
	PushU16(t, 0);    // x origin
	PushU16(t, 0);    // y origin
	PushU16(t, width);
	PushU16(t, height);
	t.push_back(bitsPerPixel);
	const uint32_t alphaBits = (bitsPerPixel == 32) ? 8u : 0u;
	const uint32_t descriptor = alphaBits | (topOrigin ? 0x20u : 0x00u);
	t.push_back(static_cast<uint8_t>(descriptor));
	t.insert(t.end(), body.begin(), body.end());
	if (withFooter)
	{
		const char* footer = "TRUEVISION-XFILE.";
		t.insert(t.end(), footer, footer + 17);
		t.push_back(0);
		PushU32(t, 0);
		PushU32(t, 0);
	}
	return t;
}

// A 2x2 BGRA Targa, bottom-up. Row order in file is [bottom, top]; the decoder
// must return top-row-first.
std::vector<uint8_t> MakeTga2x2Bgra()
{
	const std::vector<uint8_t> body = {
		// bottom row (file row 0): red-ish (B=1,G=2,R=3,A=200), teal (B=4,G=5,R=6,A=255)
		1, 2, 3, 200,  4, 5, 6, 255,
		// top row (file row 1): olive (B=7,G=8,R=9,A=255), navy (B=10,G=11,R=12,A=255)
		7, 8, 9, 255,  10, 11, 12, 255,
	};
	return MakeTgaUncompressed(2, 2, 32, /*topOrigin=*/false, body, /*withFooter=*/true);
}

// ---------------------------------------------------------------------------
// BMP fixtures
// ---------------------------------------------------------------------------

// Builds an uncompressed BI_RGB BMP. `rows` are one packed+stride-padded
// scanline per file row, in file order (bottom row first unless topDown). DIB
// header is BITMAPINFOHEADER (40). `palette` is appended for 8-bit images.
std::vector<uint8_t> MakeBmp(int32_t width, int32_t height, uint16_t bpp, bool topDown,
                              const std::vector<uint8_t>& pixelData,
                              const std::vector<uint8_t>& palette,
                              uint32_t compression = 0)
{
	const uint32_t bytesPerPixel = bpp / 8u;
	const uint32_t rowStride = (((static_cast<uint32_t>(width) * bytesPerPixel) + 3u) & ~3u);
	const uint32_t pixelSize = rowStride * static_cast<uint32_t>(height < 0 ? -height : height);
	const uint32_t headerSize = 40;
	const uint32_t pixelOffset = 14u + headerSize + static_cast<uint32_t>(palette.size());

	std::vector<uint8_t> b;
	b.push_back('B');
	b.push_back('M');
	PushU32(b, 14u + headerSize + static_cast<uint32_t>(palette.size()) + pixelSize);
	PushU16(b, 0);
	PushU16(b, 0);
	PushU32(b, pixelOffset);
	PushU32(b, headerSize);
	PushI32(b, width);
	PushI32(b, topDown ? -height : height);  // negative => top-down
	PushU16(b, 1);                          // planes
	PushU16(b, bpp);
	PushU32(b, compression);
	PushU32(b, 0);                          // image size (may be 0 for BI_RGB)
	PushU32(b, 2835);                       // x pixels per meter
	PushU32(b, 2835);                       // y pixels per meter
	PushU32(b, palette.empty() ? 0u : static_cast<uint32_t>(palette.size()) / 4u);  // colors used
	PushU32(b, 0);                          // colors important
	b.insert(b.end(), palette.begin(), palette.end());
	b.insert(b.end(), pixelData.begin(), pixelData.end());
	return b;
}

// ---------------------------------------------------------------------------
// A fresh temp directory for tree/cache tests
// ---------------------------------------------------------------------------

std::filesystem::path MakeTempDir()
{
	static int counter = 0;
	std::filesystem::path root =
	    std::filesystem::temp_directory_path() / ("ran_texture_" + std::to_string(counter++));
	std::error_code ec;
	std::filesystem::remove_all(root, ec);
	std::filesystem::create_directories(root, ec);
	return root;
}

void WriteBytes(const std::filesystem::path& path, const std::vector<uint8_t>& bytes)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream f(path, std::ios::binary | std::ios::trunc);
	f.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

} // namespace

// ---------------------------------------------------------------------------
// Targa decoder
// ---------------------------------------------------------------------------

MODERN_TEST(TextureTga_TrueColor32BottomUpFlipsVerticalAndKeepsAlpha)
{
	const Result<ImageAsset> decoded = TgaImageDecoder().DecodeImage(ResourceData(MakeTga2x2Bgra()));
	REQUIRE(decoded.IsOk());
	const ImageAsset& image = decoded.GetValue();
	CHECK_EQ(image.GetWidth(), 2u);
	CHECK_EQ(image.GetHeight(), 2u);
	CHECK_EQ(image.GetFormat(), ImageFormat::R8G8B8A8_UNorm);
	// Top row is the file's LAST row (olive, navy); bottom row is the file's
	// first (red, teal). BGR -> RGB and alpha preserved.
	CheckPixel(image, 0, 0, 9, 8, 7, 255);
	CheckPixel(image, 1, 0, 12, 11, 10, 255);
	CheckPixel(image, 0, 1, 3, 2, 1, 200);
	CheckPixel(image, 1, 1, 6, 5, 4, 255);
}

MODERN_TEST(TextureTga_TopOriginKeepsFileOrder)
{
	// Same body but top-origin: file row 0 is now the TOP row.
	const std::vector<uint8_t> body = {
		1, 2, 3, 200, 4, 5, 6, 255,
		7, 8, 9, 255, 10, 11, 12, 255,
	};
	const std::vector<uint8_t> tga = MakeTgaUncompressed(2, 2, 32, true, body, true);
	const Result<ImageAsset> decoded = TgaImageDecoder().DecodeImage(ResourceData(tga));
	REQUIRE(decoded.IsOk());
	CheckPixel(decoded.GetValue(), 0, 0, 3, 2, 1, 200);
	CheckPixel(decoded.GetValue(), 0, 1, 9, 8, 7, 255);
}

MODERN_TEST(TextureTga_TrueColor24GetsOpaqueAlpha)
{
	const std::vector<uint8_t> body = { 30, 60, 90, 12, 24, 48 };  // 2 BGR pixels
	const std::vector<uint8_t> tga = MakeTgaUncompressed(2, 1, 24, true, body, false);
	const Result<ImageAsset> decoded = TgaImageDecoder().DecodeImage(ResourceData(tga));
	REQUIRE(decoded.IsOk());
	CheckPixel(decoded.GetValue(), 0, 0, 90, 60, 30, 255);
	CheckPixel(decoded.GetValue(), 1, 0, 48, 24, 12, 255);
}

MODERN_TEST(TextureTga_RleTrueColorDecodes)
{
	// 1x4, one row. RLE packets: repeat pixel A twice, then two literal pixels.
	const std::vector<uint8_t> body = {
		0x81, 10, 20, 30, 40,   // RLE: count (0x81&0x7F)+1 = 2, pixel BGRA(10,20,30,40)
		0x81, 50, 60, 70, 80,   // RLE: 2, BGRA(50,60,70,80)
	};
	std::vector<uint8_t> t = MakeTgaUncompressed(4, 1, 32, true, /*body=*/{}, true);
	// The helper wrote an uncompressed header; this case is RLE, so set the
	// image type to 10 (RLE truecolour) and supply the RLE stream as the body.
	std::vector<uint8_t> header(t.begin(), t.begin() + 18);
	header[2] = 10;
	std::vector<uint8_t> rle(header);
	rle.insert(rle.end(), body.begin(), body.end());
	const Result<ImageAsset> decoded = TgaImageDecoder().DecodeImage(ResourceData(rle));
	REQUIRE(decoded.IsOk());
	CheckPixel(decoded.GetValue(), 0, 0, 30, 20, 10, 40);
	CheckPixel(decoded.GetValue(), 1, 0, 30, 20, 10, 40);
	CheckPixel(decoded.GetValue(), 2, 0, 70, 60, 50, 80);
	CheckPixel(decoded.GetValue(), 3, 0, 70, 60, 50, 80);
}

MODERN_TEST(TextureTga_GrayscaleExpandsToOpaque)
{
	const std::vector<uint8_t> body = { 100, 200 };  // 2 gray pixels
	const std::vector<uint8_t> tga = MakeTgaUncompressed(2, 1, 8, true, body, false);
	const Result<ImageAsset> decoded = TgaImageDecoder().DecodeImage(ResourceData(tga));
	REQUIRE(decoded.IsOk());
	CheckPixel(decoded.GetValue(), 0, 0, 100, 100, 100, 255);
	CheckPixel(decoded.GetValue(), 1, 0, 200, 200, 200, 255);
}

MODERN_TEST(TextureTga_MalformedIsRefused)
{
	auto badType = MakeTgaUncompressed(2, 1, 32, true, { 1,2,3,4, 5,6,7,8 }, false);
	badType[2] = 1;  // colour-mapped, unsupported
	CHECK(TgaImageDecoder().DecodeImage(ResourceData(badType)).IsError());

	auto badDepth = MakeTgaUncompressed(2, 1, 24, true, { 1,2,3,4,5,6 }, false);
	badDepth[16] = 16;  // 16-bit truecolour, unsupported
	CHECK(TgaImageDecoder().DecodeImage(ResourceData(badDepth)).IsError());

	auto zeroDim = MakeTgaUncompressed(0, 1, 24, true, {}, false);
	CHECK(TgaImageDecoder().DecodeImage(ResourceData(zeroDim)).IsError());

	const std::vector<uint8_t> good = MakeTga2x2Bgra();
	const std::vector<uint8_t> truncated(good.begin(), good.begin() + 20);
	CHECK(TgaImageDecoder().DecodeImage(ResourceData(truncated)).IsError());
}

// ---------------------------------------------------------------------------
// BMP decoder
// ---------------------------------------------------------------------------

// A 1x2 24-bit bottom-up BMP. File row 0 is the BOTTOM (blue), row 1 the TOP.
static std::vector<uint8_t> MakeBmp1x2_Bgr24()
{
	// Each scanline is stride-padded to 4 bytes: 3 used + 1 pad, per row.
	const std::vector<uint8_t> pixels = {
		10, 20, 30, 0,  // file row 0 = bottom (BGR)
		40, 50, 60, 0,  // file row 1 = top
	};
	return MakeBmp(1, 2, 24, /*topDown=*/false, pixels, /*palette=*/{});
}

MODERN_TEST(TextureBmp_TrueColor24BottomUpFlipsVertical)
{
	const Result<ImageAsset> decoded = BmpImageDecoder().DecodeImage(ResourceData(MakeBmp1x2_Bgr24()));
	REQUIRE(decoded.IsOk());
	const ImageAsset& image = decoded.GetValue();
	CHECK_EQ(image.GetWidth(), 1u);
	CHECK_EQ(image.GetHeight(), 2u);
	CHECK_EQ(image.GetFormat(), ImageFormat::R8G8B8A8_UNorm);
	// Top row comes from the file's last row.
	CheckPixel(image, 0, 0, 60, 50, 40, 255);
	CheckPixel(image, 0, 1, 30, 20, 10, 255);
}

MODERN_TEST(TextureBmp_TopDownKeepsFileOrder)
{
	// Same pixels but a negative height marks the file top-down.
	std::vector<uint8_t> pixels = {
		40, 50, 60, 0,  // file row 0 = top
		10, 20, 30, 0,  // file row 1 = bottom
	};
	const std::vector<uint8_t> bmp = MakeBmp(1, 2, 24, true, pixels, {});
	const Result<ImageAsset> decoded = BmpImageDecoder().DecodeImage(ResourceData(bmp));
	REQUIRE(decoded.IsOk());
	CheckPixel(decoded.GetValue(), 0, 0, 60, 50, 40, 255);
	CheckPixel(decoded.GetValue(), 0, 1, 30, 20, 10, 255);
}

MODERN_TEST(TextureBmp_Palette8IndexesColourTable)
{
	// 2x1 8-bit palette bottom-up; palette entry 0 = red, entry 1 = blue.
	const std::vector<uint8_t> palette = {
		0, 0, 255, 0,   // entry 0: B=0, G=0, R=255 (red), reserved 0
		255, 0, 0, 0,   // entry 1: B=255 (blue)
	};
	const std::vector<uint8_t> pixels = { 0, 1, 0, 0 };  // two indices, stride 2 -> pad to 4
	const std::vector<uint8_t> bmp = MakeBmp(2, 1, 8, false, pixels, palette);
	const Result<ImageAsset> decoded = BmpImageDecoder().DecodeImage(ResourceData(bmp));
	REQUIRE(decoded.IsOk());
	CheckPixel(decoded.GetValue(), 0, 0, 255, 0, 0, 255);   // red
	CheckPixel(decoded.GetValue(), 1, 0, 0, 0, 255, 255);   // blue
}

MODERN_TEST(TextureBmp_TrueColor32UnusedAlphaIsOpaque)
{
	// 32-bit BI_RGB: fourth byte is unused, must not be read as transparency.
	const std::vector<uint8_t> pixels = { 5, 6, 7, 0 };  // BGRA with alpha byte 0
	const std::vector<uint8_t> bmp = MakeBmp(1, 1, 32, true, pixels, {});
	const Result<ImageAsset> decoded = BmpImageDecoder().DecodeImage(ResourceData(bmp));
	REQUIRE(decoded.IsOk());
	CheckPixel(decoded.GetValue(), 0, 0, 7, 6, 5, 255);
}

MODERN_TEST(TextureBmp_MalformedIsRefused)
{
	auto badCompression = MakeBmp(1, 1, 24, true, { 1,2,3,0 }, {}, /*compression=*/1);
	CHECK(BmpImageDecoder().DecodeImage(ResourceData(badCompression)).IsError());

	auto badDepth = MakeBmp(1, 1, 16, true, { 1,2 }, {});
	CHECK(BmpImageDecoder().DecodeImage(ResourceData(badDepth)).IsError());

	const std::vector<uint8_t> good = MakeBmp1x2_Bgr24();
	const std::vector<uint8_t> truncated(good.begin(), good.begin() + 30);
	CHECK(BmpImageDecoder().DecodeImage(ResourceData(truncated)).IsError());

	// The 'BM' magic proves BMP; a Targa is not claimed by it.
	CHECK(BmpImageDecoder::LooksLikeBmp(good.data(), good.size()));
	const std::vector<uint8_t> tga = MakeTga2x2Bgra();
	CHECK(!BmpImageDecoder::LooksLikeBmp(tga.data(), tga.size()));
}

// ---------------------------------------------------------------------------
// Content dispatch
// ---------------------------------------------------------------------------

MODERN_TEST(TextureDispatch_RoutesByContentNotExtension)
{
	// TGA bytes and BMP bytes each decode through TextureLoader::Decode, proving
	// the content-magic dispatch routes them rather than guessing.
	CHECK(TextureLoader::Decode(ResourceData(MakeTga2x2Bgra())).IsOk());
	CHECK(TextureLoader::Decode(ResourceData(MakeBmp1x2_Bgr24())).IsOk());

	// Bytes that are neither signed nor a plausible Targa/BMP are refused.
	CHECK_EQ(TextureLoader::Decode(ResourceData(std::vector<uint8_t>(64, 0x42))).GetError(),
	         ErrorCode::InvalidArgument);
	CHECK_EQ(TextureLoader::Decode(ResourceData(std::vector<uint8_t>(2, 0x01))).GetError(),
	         ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// Texture cache: decode-once
// ---------------------------------------------------------------------------

MODERN_TEST(TextureCache_DecodesOncePerResolvedPath)
{
	const std::filesystem::path root = MakeTempDir();
	const std::filesystem::path file = root / "flare.tga";
	WriteBytes(file, MakeTga2x2Bgra());

	TextureCache cache;
	const Result<const ImageAsset*> first = cache.Acquire(file);
	REQUIRE(first.IsOk());
	CHECK_EQ(cache.GetCachedCount(), size_t(1));

	// Second acquire is a cache hit: no re-read, no re-decode, same pointer.
	const Result<const ImageAsset*> second = cache.Acquire(file);
	REQUIRE(second.IsOk());
	CHECK_EQ(first.GetValue(), second.GetValue());
	CHECK_EQ(cache.GetCachedCount(), size_t(1));

	// A lexically-equivalent path (a redundant ".") is one entry.
	CHECK(cache.Acquire(root / "." / "flare.tga").IsOk());
	CHECK_EQ(cache.GetCachedCount(), size_t(1));

	CHECK(cache.Find(file) != nullptr);
	cache.Clear();
	CHECK_EQ(cache.GetCachedCount(), size_t(0));
	CHECK(cache.Find(file) == nullptr);

	std::error_code ec;
	std::filesystem::remove_all(root, ec);
}

MODERN_TEST(TextureCache_ReportsMissingAndUndecipherable)
{
	const std::filesystem::path root = MakeTempDir();
	CHECK_EQ(TextureCache().Acquire(root / "nope.tga").GetError(), ErrorCode::NotFound);

	const std::filesystem::path junk = root / "junk.dat";
	WriteBytes(junk, std::vector<uint8_t>(64, 0x7F));
	CHECK_EQ(TextureCache().Acquire(junk).GetError(), ErrorCode::InvalidArgument);

	std::error_code ec;
	std::filesystem::remove_all(root, ec);
}

// ---------------------------------------------------------------------------
// Loader name resolution over a tree (case-insensitive, first-file-wins)
// ---------------------------------------------------------------------------

MODERN_TEST(TextureLoader_TgaTreeLookup)
{
	const std::filesystem::path root = MakeTempDir();
	WriteBytes(root / "effect" / "Flare_01.TGA", MakeTga2x2Bgra());

	TextureLoader loader;
	REQUIRE(loader.Initialize(root).IsOk());
	// Bare-name, case-insensitive match regardless of the `.TGA` extension.
	CHECK(!loader.Find("flare_01.tga").empty());
	CHECK(loader.Load("flare_01.tga").IsOk());
	CHECK(loader.Find("with/separator").empty());

	std::error_code ec;
	std::filesystem::remove_all(root, ec);
}

// ---------------------------------------------------------------------------
// Real ASURA inventory, when RAN_ASSET_ROOT names a tree
// ---------------------------------------------------------------------------

// Walks the asset root, sniffs every image candidate by content, decodes it and
// tallies per-format success/refusal. This is the authoritative "what actually
// decodes" evidence; it reads only a developer-supplied tree and skips (passes)
// when none is configured.
MODERN_TEST(TextureInventory_RealAsuraTreeDecodes)
{
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
	const char* rootEnv = std::getenv("RAN_ASSET_ROOT");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
	if (rootEnv == nullptr || rootEnv[0] == '\0')
	{
		std::printf("      (real ASURA texture inventory skipped: RAN_ASSET_ROOT not set)\n");
		return;
	}
	const std::filesystem::path root(rootEnv);
	if (!std::filesystem::is_directory(root))
	{
		std::printf("      (real ASURA texture inventory skipped: not a directory)\n");
		return;
	}

	// A bounded, deterministic sample so the suite stays well inside the
	// harness's per-test time budget: visit at most a few thousand entries and
	// decode at most a handful per supported format. This proves representative
	// real files decode; the exhaustive, uncapped inventory is produced by the
	// application's audit mode and is intentionally not part of the CTest run.
	// Textures live under the `textures/` subtree; iterate that (falling back to
	// the root) so the bounded sample is not spent walking non-image data.
	std::error_code dirEc;
	std::filesystem::path scanRoot = root / "textures";
	if (!std::filesystem::is_directory(scanRoot, dirEc)) { scanRoot = root; }

	constexpr size_t kMaxVisited        = 6000;
	constexpr size_t kSamplePerFormat   = 12;
	const char* const kTargetExts[]     = { ".dds", ".mtf", ".png", ".tga", ".bmp" };

	std::unordered_map<std::string, std::vector<std::filesystem::path>> byExt;
	size_t visited = 0;
	std::error_code ec;
	for (std::filesystem::recursive_directory_iterator it(scanRoot, ec), end;
	     !ec && it != end && visited < kMaxVisited; it.increment(ec))
	{
		++visited;
		std::error_code fileEc;
		if (!it->is_regular_file(fileEc)) { continue; }
		std::string ext = it->path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(),
		               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		bool target = false;
		for (const char* t : kTargetExts) { if (ext == t) { target = true; break; } }
		if (!target) { continue; }
		auto& bucket = byExt[ext];
		if (bucket.size() < kSamplePerFormat) { bucket.push_back(it->path()); }
	}

	auto decodeCount = [&](const std::vector<std::filesystem::path>& files) {
		size_t decoded = 0;
		for (const std::filesystem::path& path : files)
		{
			std::ifstream file(path, std::ios::binary);
			if (!file) { continue; }
			const std::vector<uint8_t> bytes(
				(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			if (bytes.empty()) { continue; }
			if (TextureLoader::Decode(ResourceData(bytes)).IsOk()) { ++decoded; }
		}
		return decoded;
	};

	std::printf("      ASURA texture sample (visited %zu entries, content-decoded):\n", visited);
	size_t tgaDecoded = 0, bmpDecoded = 0, ddsDecoded = 0, pngDecoded = 0;
	for (const char* ext : kTargetExts)
	{
		const auto it2 = byExt.find(ext);
		if (it2 == byExt.end() || it2->second.empty()) { continue; }
		const size_t decoded = decodeCount(it2->second);
		std::printf("        %-5s sampled=%zu decoded=%zu\n", ext, it2->second.size(), decoded);
		if (std::string(ext) == ".tga") { tgaDecoded = decoded; }
		if (std::string(ext) == ".bmp") { bmpDecoded = decoded; }
		if (std::string(ext) == ".dds") { ddsDecoded = decoded; }
		if (std::string(ext) == ".png") { pngDecoded = decoded; }
	}

	// Representative real files of the dominant formats decode, and the two
	// formats this milestone added are present in the tree and decode from it.
	CHECK(ddsDecoded > 0u);   // DDS decodes from the real tree
	CHECK(pngDecoded > 0u);   // PNG decodes from the real tree
	CHECK(tgaDecoded > 0u);   // TGA decodes from the real tree
	CHECK(bmpDecoded > 0u);   // BMP decodes from the real tree
}

int main()
{
	std::printf("Modern ASURA-TEXTURE-001 texture decoder / loader / inventory tests\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n", static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases, static_cast<int>(ModernTests::Registry().size()), ModernTests::FailureCount());
	return 1;
}
