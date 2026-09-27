#include "resources/MemoryResourceProvider.h"

namespace Modern::Client
{

Status MemoryResourceProvider::RegisterResource(const ResourceId& id, ResourceData data)
{
	if (!id.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	m_storage[id] = std::move(data);
	return Ok();
}

Status MemoryResourceProvider::UnregisterResource(const ResourceId& id)
{
	if (!id.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	auto it = m_storage.find(id);
	if (it == m_storage.end())
	{
		return Status(ErrorCode::NotFound);
	}

	m_storage.erase(it);
	return Ok();
}

void MemoryResourceProvider::Clear()
{
	m_storage.clear();
}

bool MemoryResourceProvider::HasResource(const ResourceId& id) const
{
	if (!id.IsValid())
	{
		return false;
	}

	return m_storage.find(id) != m_storage.end();
}

Result<ResourceData> MemoryResourceProvider::Load(const ResourceId& id)
{
	if (!id.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	auto it = m_storage.find(id);
	if (it == m_storage.end())
	{
		return Status(ErrorCode::NotFound);
	}

	return it->second;
}

} // namespace Modern::Client
