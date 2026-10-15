#include "assets/TextureCache.h"

#include "assets/TextureLoader.h"

#include <fstream>
#include <iterator>
#include <utility>
#include <vector>

namespace Modern::Client
{

std::string TextureCache::CanonicalKey(const std::filesystem::path& filePath)
{
	// The identity of a texture is the file it resolved to, normalised so a
	// lexically-equivalent path (a redundant "." or "..", a trailing slash)
	// shares one entry rather than two.
	return filePath.lexically_normal().string();
}

Result<const ImageAsset*> TextureCache::Acquire(const std::filesystem::path& filePath)
{
	const std::string key = CanonicalKey(filePath);

	const auto existing = m_cache.find(key);
	if (existing != m_cache.end())
	{
		return existing->second.get();
	}

	std::ifstream file(filePath, std::ios::binary);
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

	// Content dispatch, not extension -- the same sniff TextureLoader::Load uses.
	Result<ImageAsset> decoded = TextureLoader::Decode(ResourceData(std::move(bytes)));
	if (decoded.IsError())
	{
		return decoded.GetStatus();
	}

	auto stored = std::make_shared<ImageAsset>(std::move(decoded.GetValue()));
	const ImageAsset* pointer = stored.get();
	m_cache.emplace(std::move(key), std::move(stored));
	return pointer;
}

const ImageAsset* TextureCache::Find(const std::filesystem::path& filePath) const noexcept
{
	const auto existing = m_cache.find(CanonicalKey(filePath));
	return existing != m_cache.end() ? existing->second.get() : nullptr;
}

} // namespace Modern::Client
