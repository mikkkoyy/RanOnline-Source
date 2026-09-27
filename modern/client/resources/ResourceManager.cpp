#include "resources/ResourceManager.h"

namespace Modern::Client
{

const char* ToString(ResourceManagerState state) noexcept
{
	switch (state)
	{
	case ResourceManagerState::Uninitialized: return "Uninitialized";
	case ResourceManagerState::Ready:         return "Ready";
	case ResourceManagerState::ShutdownState: return "Shutdown";
	default:                                  return "Unknown";
	}
}

Status ResourceManager::Initialize()
{
	if (m_state == ResourceManagerState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != ResourceManagerState::Uninitialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	if (m_provider == nullptr)
	{
		return Status(ErrorCode::InvalidArgument);
	}

	m_state = ResourceManagerState::Ready;
	return Ok();
}

Status ResourceManager::Shutdown()
{
	if (m_state == ResourceManagerState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state == ResourceManagerState::Uninitialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_cache.clear();
	m_provider = nullptr;
	m_state = ResourceManagerState::ShutdownState;
	return Ok();
}

Result<ResourceData> ResourceManager::Load(const ResourceId& id)
{
	if (m_state != ResourceManagerState::Ready)
	{
		return Status(ErrorCode::InvalidState);
	}

	if (!id.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	// Check cache first
	auto it = m_cache.find(id);
	if (it != m_cache.end())
	{
		return it->second;
	}

	// Load from provider
	if (m_provider == nullptr)
	{
		return Status(ErrorCode::InvalidState);
	}

	auto result = m_provider->Load(id);
	if (result.IsError())
	{
		return result.GetStatus();
	}

	// Cache loaded data
	m_cache[id] = result.GetValue();
	return result.GetValue();
}

bool ResourceManager::IsCached(const ResourceId& id) const noexcept
{
	if (!id.IsValid() || m_state != ResourceManagerState::Ready)
	{
		return false;
	}

	return m_cache.find(id) != m_cache.end();
}

Status ResourceManager::ClearCache()
{
	if (m_state != ResourceManagerState::Ready)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_cache.clear();
	return Ok();
}

} // namespace Modern::Client
