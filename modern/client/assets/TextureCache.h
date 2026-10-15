#pragma once

// ASURA-TEXTURE-001: a decode-once texture cache.
//
// Decoding a texture is expensive and deterministic: the same file always
// produces the same `ImageAsset`. The character mesh references one body
// texture from many materials, the character UI references the same frame from
// many widgets, and a whole asset tree is walked more than once (viewer, audit,
// a later material pass), so every decoded image is cached by the *resolved
// path* it came from and reused. A second request for the same file neither
// re-reads it nor re-decodes it.
//
// Deliberately split from GPU upload. Decoding lives in the asset layer and must
// stay free of Direct3D; uploading lives behind the renderer boundary. This
// cache owns only the CPU `ImageAsset` values; the caller (the application)
// keeps its own resolved-path -> handle map so a cached image is uploaded once
// and bound many times. Keeping the two keys the same resolved path is what
// makes "decode once, upload once" fall out of both halves being keyed
// identically.
//
// It does not resolve names or walk a tree -- a caller hands it the concrete
// file a `TextureLoader` already located -- and it does not decide the format:
// the bytes are sniffed through `TextureLoader::Decode` exactly as every other
// load is. Failure to read or decode is reported (`NotFound` / `InvalidArgument`)
// and nothing is cached for it, so a later retry after a fix re-decodes.

#include "assets/ImageAsset.h"
#include "types/Result.h"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace Modern::Client
{

class TextureCache final
{
public:
	// Reads and decode-caches `filePath`. Returns a pointer to the cached asset,
	// valid until the cache is cleared or this entry is evicted. The first call
	// reads+decodes; every later call for the same resolved path returns the
	// same pointer without touching the file.
	//
	//   NotFound        - the file cannot be opened
	//   InvalidArgument - the bytes are not a decodable PNG/DDS/MTF/TGA/BMP image
	Result<const ImageAsset*> Acquire(const std::filesystem::path& filePath);

	// The cached asset for a resolved path, or nullptr when it is not cached.
	const ImageAsset* Find(const std::filesystem::path& filePath) const noexcept;

	// Distinct textures currently cached.
	size_t GetCachedCount() const noexcept { return m_cache.size(); }

	void Clear() noexcept { m_cache.clear(); }

private:
	static std::string CanonicalKey(const std::filesystem::path& filePath);

	std::unordered_map<std::string, std::shared_ptr<ImageAsset>> m_cache;
};

} // namespace Modern::Client
