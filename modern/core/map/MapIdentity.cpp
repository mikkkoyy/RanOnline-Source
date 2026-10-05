#include "map/MapIdentity.h"

namespace Modern::Map
{
	std::string ToString(MapIdentity id)
	{
		return std::to_string(id.main) + "/" + std::to_string(id.sub);
	}
}
