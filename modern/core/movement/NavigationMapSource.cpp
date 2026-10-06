#include "movement/NavigationMapSource.h"

namespace Modern::Movement
{
	std::shared_ptr<const Navigation::NavigationMesh>
	MapRegistryMeshSource::MeshForPackedMapId(std::uint32_t packedMapId) const
	{
		if (m_registry == nullptr || !m_registry->Loaded())
		{
			return nullptr;
		}

		// A `SNATIVEID` union's `wMainID` is the LOW half, because it is declared first
		// in the struct the union overlays (GLDefine.h:102-121), which is exactly the
		// rule `MapIdentity::Packed` inverts. This is the only place that conversion
		// happens.
		return m_registry->NavigationMesh(Map::MapIdentity::FromPacked(packedMapId));
	}

	std::string MapRegistryMeshSource::Describe(std::uint32_t packedMapId) const
	{
		if (m_registry == nullptr)
		{
			return "no map registry was configured";
		}

		if (!m_registry->Loaded())
		{
			return "the map registry did not load: " + m_registry->LoadError();
		}

		const Map::MapIdentity id = Map::MapIdentity::FromPacked(packedMapId);

		const Map::MapEntry* entry = m_registry->Find(id);
		if (entry == nullptr)
		{
			return "map " + ToString(id) + " is not registered";
		}

		// Every status except NoNavigation is a failure to walk, and the entry already
		// carries a one-line reason naming the stage. NoNavigation gets its own
		// sentence because it is NOT a failure: it is a valid map RAN never walks, and
		// calling it an error would turn nine good login maps into nine errors.
		if (entry->status == Map::MapResolveStatus::NoNavigation)
		{
			return "map " + ToString(id) + " (" + entry->wldFileName +
			       ") has no navigation data; RAN never walks it";
		}

		return "map " + ToString(id) + ": " + Map::ToString(entry->status) +
		       (entry->detail.empty() ? std::string() : " - " + entry->detail);
	}

	bool MapRegistryMeshSource::RegistryLoaded() const noexcept
	{
		return m_registry != nullptr && m_registry->Loaded();
	}
}
