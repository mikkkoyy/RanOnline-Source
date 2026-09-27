#pragma once

#include "character/Character.h"

namespace Modern
{
	namespace Legacy { class GLChar; }

	class CharacterAdapter
	{
	public:
		static Character FromLegacy(Legacy::GLChar& src);
		static void ToLegacy(const Character& src, Legacy::GLChar& dst);
	};
}
