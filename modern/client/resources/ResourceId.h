#pragma once

#include "types/Result.h"

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>

namespace Modern::Client
{

// Strong logical resource identifier.
//
// Represents a platform-independent logical resource path or name (e.g.,
// "textures/ui/login_bg", "models/character/body", "data/maps/campus").
// Rejects empty identifiers, whitespace-only identifiers, and backslash
// path separators (enforcing canonical forward-slash portable format).
class ResourceId
{
public:
	ResourceId() = default;

	// Validates and constructs a ResourceId. If invalid, returns
	// Status(ErrorCode::InvalidArgument).
	static Result<ResourceId> Create(std::string_view name);

	// Returns true if this identifier represents a valid, non-empty logical identity.
	bool IsValid() const noexcept { return !m_name.empty(); }

	// Returns the raw logical string identifier.
	const std::string& GetName() const noexcept { return m_name; }
	std::string_view GetView() const noexcept { return m_name; }

	// Comparison operators for deterministic ordering and map lookup.
	friend bool operator==(const ResourceId& lhs, const ResourceId& rhs) noexcept
	{
		return lhs.m_name == rhs.m_name;
	}

	friend bool operator!=(const ResourceId& lhs, const ResourceId& rhs) noexcept
	{
		return lhs.m_name != rhs.m_name;
	}

	friend bool operator<(const ResourceId& lhs, const ResourceId& rhs) noexcept
	{
		return lhs.m_name < rhs.m_name;
	}

private:
	explicit ResourceId(std::string name) : m_name(std::move(name)) {}

	std::string m_name;
};

} // namespace Modern::Client

namespace std
{
	template <>
	struct hash<Modern::Client::ResourceId>
	{
		size_t operator()(const Modern::Client::ResourceId& id) const noexcept
		{
			return std::hash<std::string>{}(id.GetName());
		}
	};
}
