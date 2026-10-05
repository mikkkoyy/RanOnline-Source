#pragma once

// WORLD-ENTRY-002d: a waypoint list, and the "how far ahead can this actor see"
// step that turns it into a walkable path.
//
// Transcribed from legacy/Lib_Engine/NaviMesh/navigationpath.h.
//
// ---------------------------------------------------------------------------
// WHY GetFurthestVisibleWayPoint IS THE WHOLE OF PATH SMOOTHING
// ---------------------------------------------------------------------------
//
// RAN does not smooth a path. It builds an A* waypoint list through cell wall
// midpoints and then, each time the actor reaches one, asks for the FURTHEST
// waypoint it can see in a straight line from where it stands
// (navigationpath.h:111-150). Everything between those two is skipped.
//
// That matters because `Actor::Update` re-enters itself with the remaining time
// after consuming a waypoint (actor.cpp:405), so the number of waypoint hops
// determines how many mesh resolutions one tick can perform - and therefore the
// frame rate at which a character visibly turns corners. Reproducing the method
// exactly is what makes a modern character walk the same way.
//
// The list is a `std::vector` here rather than RAN's `std::list`, and that IS a
// deviation worth naming: RAN hands out a `std::list::const_iterator` that stays
// valid as the path is walked, while this type hands out an INDEX. Same
// behaviour, but a caller cannot hold a reference across a rebuild - and the
// only caller (`MovementController`) re-reads the index every tick.

#include "navigation/NavigationCell.h"
#include "math/Vector3.h"

#include <cstdint>
#include <vector>

namespace Modern::Navigation
{
	class NavigationMesh;

	struct NavigationWaypoint
	{
		Vector3        position{};
		std::uint32_t  cellId = 0;
	};

	class NavigationPath
	{
	public:
		// Index into the waypoint list. `kEnd` means "past the last waypoint".
		//
		// See the header note on the iterator change.
		using WaypointIndex = std::size_t;

		static constexpr WaypointIndex kEnd = static_cast<WaypointIndex>(-1);

		void Clear() { m_waypoints.clear(); }

		// navigationpath.h:63-74. PUSHES THE START POINT, so the list always
		// begins with where the actor already is.
		void Setup(const NavigationMesh* parent, const Vector3& startPoint, std::uint32_t startId,
		           const Vector3& endPoint, std::uint32_t endId)
		{
			m_waypoints.clear();

			m_parent     = parent;
			m_startPoint = {startPoint, startId};
			m_endPoint   = {endPoint, endId};

			m_waypoints.push_back(m_startPoint);
		}

		void AddWayPoint(const Vector3& point, std::uint32_t cellId)
		{
			m_waypoints.push_back({point, cellId});
		}

		// navigationpath.h:86-89. PUSHES THE END POINT, so the destination is
		// always the last waypoint and arrival is "reached the last index".
		void EndPath() { m_waypoints.push_back(m_endPoint); }

		const NavigationMesh* Parent() const noexcept { return m_parent; }
		const NavigationWaypoint& StartPoint() const noexcept { return m_startPoint; }
		const NavigationWaypoint& EndPoint() const noexcept { return m_endPoint; }

		const std::vector<NavigationWaypoint>& Waypoints() const noexcept { return m_waypoints; }
		std::size_t Size() const noexcept { return m_waypoints.size(); }
		bool        Empty() const noexcept { return m_waypoints.empty(); }

		// The furthest waypoint visible in a straight line from `vantageIndex`.
		//
		// Returns `kEnd` when there is nothing beyond, which is how the caller
		// learns the path is finished.
		WaypointIndex GetFurthestVisibleWayPoint(WaypointIndex vantageIndex) const;

	private:
		const NavigationMesh* m_parent = nullptr;
		NavigationWaypoint m_startPoint{};
		NavigationWaypoint m_endPoint{};
		std::vector<NavigationWaypoint> m_waypoints;
	};
}
