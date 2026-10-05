#include "navigation/NavigationPath.h"

#include "navigation/NavigationMesh.h"

namespace Modern::Navigation
{
	NavigationPath::WaypointIndex NavigationPath::GetFurthestVisibleWayPoint(
	    WaypointIndex vantageIndex) const
	{
		if (vantageIndex >= m_waypoints.size())
		{
			return kEnd;
		}

		// Already the last waypoint.
		const NavigationWaypoint& vantage = m_waypoints[vantageIndex];

		WaypointIndex test = vantageIndex + 1;
		if (test >= m_waypoints.size())
		{
			return test;
		}

		WaypointIndex visible = test;
		++test;

		// Walk forward while each successive waypoint is still visible from the
		// vantage point, and stop at the last one that was. The comparison is
		// against the VANTAGE, never against the previous waypoint: that is what
		// makes this a string-pull rather than a chain of pairwise checks
		// (navigationpath.h:142).
		while (test < m_waypoints.size())
		{
			const NavigationWaypoint& candidate = m_waypoints[test];
			if (m_parent == nullptr ||
			    !m_parent->LineOfSightTest(vantage.cellId, vantage.position,
			                              candidate.cellId, candidate.position))
			{
				return visible;
			}
			visible = test;
			++test;
		}

		return visible;
	}
}
