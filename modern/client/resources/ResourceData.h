#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace Modern::Client
{

// Immutable/value-oriented representation of loaded binary resource bytes.
//
// Provides safe, zero-dependency access to binary data in memory without
// exposing OS file handles, DirectX textures/buffers, or legacy streams.
class ResourceData
{
public:
	ResourceData() = default;

	explicit ResourceData(std::vector<uint8_t> bytes)
		: m_bytes(std::move(bytes))
	{
	}

	ResourceData(const uint8_t* data, size_t size)
		: m_bytes(data, data + size)
	{
	}

	explicit ResourceData(std::string_view text)
		: m_bytes(reinterpret_cast<const uint8_t*>(text.data()),
		          reinterpret_cast<const uint8_t*>(text.data() + text.size()))
	{
	}

	bool IsEmpty() const noexcept { return m_bytes.empty(); }
	size_t GetSize() const noexcept { return m_bytes.size(); }
	const uint8_t* GetData() const noexcept { return m_bytes.data(); }

	const std::vector<uint8_t>& GetBytes() const noexcept { return m_bytes; }

	// View binary data as string_view (for text resources or testing)
	std::string_view AsStringView() const noexcept
	{
		return std::string_view(reinterpret_cast<const char*>(m_bytes.data()), m_bytes.size());
	}

	friend bool operator==(const ResourceData& lhs, const ResourceData& rhs) noexcept
	{
		return lhs.m_bytes == rhs.m_bytes;
	}

	friend bool operator!=(const ResourceData& lhs, const ResourceData& rhs) noexcept
	{
		return lhs.m_bytes != rhs.m_bytes;
	}

private:
	std::vector<uint8_t> m_bytes;
};

} // namespace Modern::Client
