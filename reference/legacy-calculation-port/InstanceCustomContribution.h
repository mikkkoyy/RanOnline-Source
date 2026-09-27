#pragma once

#include <cstdint>

namespace Modern
{
	// Instance custom value contributions from equipped items
	// Corresponds to legacy SITEMCUSTOM computed values:
	// GETADDPA, GETADDSA, GETDAMAGE, GETDEFENSE, GETHITRATE, GETAVOIDRATE
	// and their rate variants
	struct InstanceCustomContribution
	{
		// PA/SA/MA from random options (melee/shooting/energy)
		// Legacy: GETADDPA(), GETADDSA(), GETADDMA()
		int32_t addPA = 0;      // melee random option
		int32_t addSA = 0;      // shooting random option
		int32_t addMA = 0;      // energy random option (also used for MA damage)

		// Damage from instance (base + grade + random option rate/volume)
		// Legacy: GETDAMAGE()
		uint32_t damageLow = 0;
		uint32_t damageHigh = 0;

		// Defense from instance (base + grade)
		// Legacy: GETDEFENSE()
		int32_t defense = 0;

		// Hit/Avoid from instance (base + random option)
		// Legacy: GETHITRATE(), GETAVOIDRATE()
		int32_t hitRate = 0;
		int32_t avoidRate = 0;

		// Hit/Avoid rate percentages from random options
		// Legacy: GETHITRATE_PER(), GETAVOIDRATE_PER()
		float hitRatePer = 0.0f;
		float avoidRatePer = 0.0f;

		void Reset()
		{
			*this = {};
		}
	};
}