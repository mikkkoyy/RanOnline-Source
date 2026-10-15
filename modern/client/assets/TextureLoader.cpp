#include "assets/TextureLoader.h"

#include "assets/DdsImageDecoder.h"
#include "assets/BmpImageDecoder.h"
#include "assets/MtfTextureTransform.h"
#include "assets/PngImageDecoder.h"
#include "assets/TgaImageDecoder.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>

namespace Modern::Client
{
namespace
{

// The PNG signature, compared as one little-endian uint64 so the sniff is a
// single load rather than eight byte tests. The value is the eight spec bytes
// 137 80 78 71 13 10 26 10 in file order.
constexpr uint64_t kPngSignature = 0x0A1A0A0D474E5089ull;

// The DDS magic as the first four bytes 'D' 'D' 'S' ' ' read little-endian.
constexpr uint32_t kDdsMagic = 0x20534444u;

// The MTF version field (first four bytes, little-endian). The full MTF
// header check -- payload size and file type -- belongs to
// MtfTextureTransform, which owns the format; this sniff only has to be
// specific enough that no other magic reaches it first.
constexpr uint32_t kMtfVersion = 0x100u;

// ASCII lowercase, defined for byte values only. std::tolower's behaviour
// outside 0..127 is locale-dependent, and texture names are looked up
// case-insensitively the same way the legacy `_strlwr` path did.
char ToLowerAscii(char c) noexcept
{
	return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

std::string Lowercase(std::string name) noexcept
{
	for (char& c : name)
	{
		c = ToLowerAscii(c);
	}
	return name;
}

uint32_t ReadU32Le(const uint8_t* p) noexcept
{
	return static_cast<uint32_t>(p[0]) |
	       (static_cast<uint32_t>(p[1]) << 8) |
	       (static_cast<uint32_t>(p[2]) << 16) |
	       (static_cast<uint32_t>(p[3]) << 24);
}

} // namespace

Status TextureLoader::Initialize(const std::filesystem::path& texturesRoot)
{
	m_index.clear();
	m_initialized = false;

	if (texturesRoot.empty())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	std::error_code error;
	if (!std::filesystem::is_directory(texturesRoot, error))
	{
		return Status(ErrorCode::NotFound);
	}

	// The iteration order of a directory iterator is unspecified, so every
	// path is collected first and sorted: the same tree always produces the
	// same index, and "first file wins" is first in sorted order rather than
	// first on disk.
	std::vector<std::filesystem::path> files;
	for (std::filesystem::recursive_directory_iterator it(texturesRoot, error), end;
	     !error && it != end; it.increment(error))
	{
		if (it->is_regular_file(error))
		{
			files.push_back(it->path());
		}
	}
	if (error)
	{
		return Status(ErrorCode::NotFound);
	}
	std::sort(files.begin(), files.end());

	for (const std::filesystem::path& path : files)
	{
		// Lowercase filename as the key, exactly as legacy lowercased the
		// requested name and its own tree to make the match case-blind.
		const std::string key = Lowercase(path.filename().string());
		if (!key.empty())
		{
			m_index.emplace(key, path);  // first file wins on collision
		}
	}

	m_initialized = true;
	return Ok();
}

std::filesystem::path TextureLoader::Find(const std::string& bareName) const
{
	// Only bare filenames are addresses. A separator in the name would let a
	// material string point anywhere under (or above) the textures root, and
	// the RAN `.X` files never carry one.
	if (bareName.empty() || bareName.find('/') != std::string::npos ||
	    bareName.find('\\') != std::string::npos)
	{
		return {};
	}

	const std::string key = Lowercase(bareName);

	// Legacy order (TextureManager::LoadImageData): the encrypted sibling is
	// preferred -- the requested name with its extension replaced by ".mtf" --
	// before the exact name is tried.
	const size_t dot = key.rfind('.');
	const std::string stem = (dot == std::string::npos) ? key : key.substr(0, dot);
	if (!stem.empty())
	{
		const auto encrypted = m_index.find(stem + ".mtf");
		if (encrypted != m_index.end())
		{
			return encrypted->second;
		}
	}

	const auto exact = m_index.find(key);
	if (exact != m_index.end())
	{
		return exact->second;
	}

	return {};
}

Result<ImageAsset> TextureLoader::Load(const std::string& bareName)
{
	if (!m_initialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	const std::filesystem::path path = Find(bareName);
	if (path.empty())
	{
		return Status(ErrorCode::NotFound);
	}

	std::ifstream file(path, std::ios::binary);
	if (!file)
	{
		return Status(ErrorCode::NotFound);
	}

	const std::vector<uint8_t> bytes(
		(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	if (bytes.empty())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	return Decode(ResourceData(bytes));
}

Result<ImageAsset> TextureLoader::Decode(const ResourceData& data)
{
	const size_t size = data.GetSize();
	const uint8_t* bytes = data.GetData();
	if (bytes == nullptr || size < 4)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Magic dispatch, most-specific signature first. The extension of the file
	// is never consulted: a `.png` that is really a DDS, or an `.mtf` that is
	// really a PNG, decodes for what it is, and a file that claims nothing is
	// refused rather than guessed at.
	if (size >= 8)
	{
		uint64_t signature = 0;
		for (int i = 0; i < 8; ++i)
		{
			signature |= static_cast<uint64_t>(bytes[i]) << (i * 8);
		}
		if (signature == kPngSignature)
		{
			PngImageDecoder decoder;
			return decoder.DecodeImage(data);
		}
	}

	if (ReadU32Le(bytes) == kDdsMagic)
	{
		DdsImageDecoder decoder;
		return decoder.DecodeImage(data);
	}

	if (size >= 12 && ReadU32Le(bytes) == kMtfVersion)
	{
		const Result<ResourceData> plain = MtfTextureTransform::Transform(data);
		if (plain.IsError())
		{
			return plain.GetStatus();
		}
		DdsImageDecoder decoder;
		return decoder.DecodeImage(plain.GetValue());
	}

	// BMP carries a two-byte `BM` magic, so it sniffs as reliably as DDS.
	if (BmpImageDecoder::LooksLikeBmp(bytes, size))
	{
		BmpImageDecoder decoder;
		return decoder.DecodeImage(data);
	}

	// TGA has no leading magic. Its detection is the footer signature or a
	// strict header check, tried last so no signed format above is ever
	// mis-attributed to it.
	if (TgaImageDecoder::LooksLikeTga(bytes, size))
	{
		TgaImageDecoder decoder;
		return decoder.DecodeImage(data);
	}

	return Status(ErrorCode::InvalidArgument);
}

} // namespace Modern::Client