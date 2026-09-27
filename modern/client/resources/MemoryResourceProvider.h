#pragma once

#include "resources/ResourceData.h"
#include "resources/ResourceId.h"
#include "resources/ResourceProvider.h"
#include "types/Result.h"

#include <unordered_map>

namespace Modern::Client
{

// Deterministic in-memory resource provider for headless tests and mock setups.
//
// Allows tests and headless harnesses to register byte data for logical resource
// identities without touching the filesystem, OS handles, or any graphics APIs.
class MemoryResourceProvider final : public IResourceProvider
{
public:
	MemoryResourceProvider() = default;

	// Registers or overwrites resource data for an identifier.
	// Returns ErrorCode::InvalidArgument if id is invalid.
	Status RegisterResource(const ResourceId& id, ResourceData data);

	// Unregisters a resource. Returns ErrorCode::NotFound if not present.
	Status UnregisterResource(const ResourceId& id);

	// Removes all registered resources.
	void Clear();

	// Number of registered resources.
	size_t GetCount() const noexcept { return m_storage.size(); }

	// IResourceProvider implementation
	bool HasResource(const ResourceId& id) const override;
	Result<ResourceData> Load(const ResourceId& id) override;

private:
	std::unordered_map<ResourceId, ResourceData> m_storage;
};

} // namespace Modern::Client
