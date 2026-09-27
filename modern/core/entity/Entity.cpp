#include "Entity.h"

namespace Modern
{
	const char* ToString(EntityState state) noexcept
	{
		switch (state)
		{
			case EntityState::Uninitialized: return "Uninitialized";
			case EntityState::Created:       return "Created";
			case EntityState::Spawned:       return "Spawned";
			case EntityState::Despawned:     return "Despawned";
			case EntityState::Destroyed:     return "Destroyed";
		}

		return "Unknown";
	}
}
