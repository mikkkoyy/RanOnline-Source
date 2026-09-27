#pragma once

#include "../types/Types.h"
#include "../math/Vector3.h"

namespace Modern
{
	struct Entity
	{
		EntityId id;
		Vector3 position;
		Vector3 direction;

		Entity() = default;
		Entity(EntityId i, const Vector3& p, const Vector3& d)
			: id(i), position(p), direction(Normalize(d)) {}
	};
}
