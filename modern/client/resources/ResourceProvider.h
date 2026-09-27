#pragma once

#include "types/Result.h"
#include "resources/ResourceData.h"
#include "resources/ResourceId.h"

namespace Modern::Client
{

// Abstract provider responsible for resolving a logical ResourceId into raw ResourceData.
//
// Providers have zero knowledge of rendering, GPU handles, or Windows APIs.
// Examples: MemoryResourceProvider (deterministic tests), FileSystemResourceProvider
// (portable disk loader), or future ArchiveResourceProvider (legacy RCC/archive adapter).
class IResourceProvider
{
public:
	virtual ~IResourceProvider() = default;

	// Checks whether the resource exists in this provider.
	virtual bool HasResource(const ResourceId& id) const = 0;

	// Loads raw binary data for the given logical resource identity.
	// Returns ErrorCode::NotFound if the resource does not exist.
	// Returns ErrorCode::InvalidArgument if id is invalid.
	virtual Result<ResourceData> Load(const ResourceId& id) = 0;
};

} // namespace Modern::Client
