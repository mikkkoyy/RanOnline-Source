#pragma once

#include "resources/ResourceData.h"
#include "resources/ResourceId.h"
#include "resources/ResourceProvider.h"
#include "types/Result.h"

#include <cstdint>
#include <unordered_map>

namespace Modern::Client
{

// Lifecycle states of the ResourceManager.
enum class ResourceManagerState : uint8_t
{
	Uninitialized = 0,
	Ready,
	ShutdownState,
};

const char* ToString(ResourceManagerState state) noexcept;

// Small, deterministic resource manager coordinating resource requests.
//
// Operates without global state or singletons. Receives an injected IResourceProvider.
// Provides basic in-memory caching of loaded ResourceData to ensure repeated
// lookups are deterministic and fast.
//
// Zero dependency on rendering, DirectX, Vulkan, Windows handles, or legacy headers.
class ResourceManager
{
public:
	ResourceManager() = default;
	~ResourceManager() = default;

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;
	ResourceManager(ResourceManager&&) = default;
	ResourceManager& operator=(ResourceManager&&) = default;

	// Injects or replaces the resource provider. Allowed in any state except Shutdown.
	// Can be called prior to Initialize().
	void SetProvider(IResourceProvider* provider) noexcept { m_provider = provider; }
	IResourceProvider* GetProvider() const noexcept { return m_provider; }

	// Uninitialized -> Ready. Requires a valid provider; returns InvalidArgument if null.
	// Returns InvalidState if already Ready, or NotAllowed if ShutdownState.
	Status Initialize();

	// Explicit shutdown: Ready -> ShutdownState. Clears cache and detaches provider.
	// Returns NotAllowed if already ShutdownState, InvalidState if Uninitialized.
	Status Shutdown();

	// Looks up / loads a resource by ResourceId.
	// Returns cached data if available.
	// Otherwise loads from provider and caches the result.
	// Returns InvalidState if not in Ready state.
	// Returns InvalidArgument if id is invalid.
	// Returns NotFound if not found in provider.
	Result<ResourceData> Load(const ResourceId& id);

	// Queries if a resource is currently cached in memory.
	bool IsCached(const ResourceId& id) const noexcept;

	// Clears the internal cache without shutting down the manager.
	// Valid only when in Ready state.
	Status ClearCache();

	// Returns the number of cached resources.
	size_t GetCachedCount() const noexcept { return m_cache.size(); }

	// Current state
	ResourceManagerState GetState() const noexcept { return m_state; }

private:
	ResourceManagerState m_state = ResourceManagerState::Uninitialized;
	IResourceProvider* m_provider = nullptr;
	std::unordered_map<ResourceId, ResourceData> m_cache;
};

} // namespace Modern::Client
