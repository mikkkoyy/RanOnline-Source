#include "movement/Actor.h"

#include <cfloat>
#include <cmath>

namespace Modern::Movement
{
	Vector3 NoNextPosition() noexcept
	{
		// RAN's sentinel, actor.cpp:19 and :400. FLT_MAX in ALL THREE components, and
		// compared component-wise rather than as a vector - GLChar.cpp:6104 tests
		// `x != FLT_MAX && y != FLT_MAX && z != FLT_MAX`, because that code compares
		// plain floats rather than Modern::Vector3 and has no operator>.
		return Vector3{ FLT_MAX, FLT_MAX, FLT_MAX };
	}

	bool HasNextPosition(const Vector3& value) noexcept
	{
		// The exact three-component test of GLChar.cpp:6104.
		return value.x != FLT_MAX && value.y != FLT_MAX && value.z != FLT_MAX;
	}

	Actor::~Actor()
	{
		// Nothing to release: the mesh is a shared_ptr and the session a unique_ptr,
		// both destroyed with the actor. Declared explicitly because an implicit
		// destructor hides whether the class owns anything, and this one deliberately
		// shares the mesh while owning the search session.
	}

	Status Actor::Create(std::shared_ptr<const Navigation::NavigationMesh> mesh,
	                     const Vector3& position, std::uint32_t cellId)
	{
		// A null or unbuilt mesh is refused here rather than accepted and then failing
		// every Update. Legacy allowed a null `m_Parent` and reported E_FAIL from
		// `Update` forever after, which is a failure discovered at the worst possible
		// moment instead of at the call that caused it.
		if (mesh == nullptr || !mesh->Built())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		m_mesh = std::move(mesh);

		if (!EnsureSession())
		{
			m_mesh.reset();
			return Status(ErrorCode::InvalidState);
		}

		m_position      = position;
		m_currentCellId = cellId;
		m_movement      = Vector3{ 0.0f, 0.0f, 0.0f };

		// actor.cpp:73-86: the ±5 spawn probe runs FIRST, and only a miss falls back to
		// the nearest cell plus a snap. A hit adopts the hit point and its cell. The
		// `cellId` argument is therefore ADVISORY and is overwritten either way, which
		// is what legacy does and which is why a caller passing a stale cell id is not
		// stranded.
		Vector3        hitPoint{};
		std::uint32_t hitCell = 0;

		const Vector3 from{ position.x, position.y + kSpawnProbeHalfHeight, position.z };
		const Vector3 to{ position.x, position.y - kSpawnProbeHalfHeight, position.z };

		if (m_mesh->IsCollision(from, to, hitPoint, &hitCell))
		{
			m_currentCellId = hitCell;
			m_position      = hitPoint;
		}
		else
		{
			m_currentCellId = m_mesh->FindClosestCell(position);
			m_position      = m_mesh->SnapPointToCell(m_currentCellId, position);
		}

		// A mesh can report a cell id no cell has. Caught here so the actor is never
		// created already broken, which would otherwise surface as an Update failure on
		// some later tick with no obvious cause.
		if (m_mesh->GetCell(m_currentCellId) == nullptr)
		{
			m_mesh.reset();
			m_session.reset();
			return Status(ErrorCode::InvalidArgument);
		}

		return Ok();
	}

	void Actor::Detach()
	{
		Stop();
		m_mesh.reset();
		m_session.reset();
		m_currentCellId = kNoCell;
		m_position      = Vector3{ 0.0f, 0.0f, 0.0f };
		m_movement      = Vector3{ 0.0f, 0.0f, 0.0f };
		m_nextPosition  = NoNextPosition();
		m_correctY      = 0.0f;
		m_firstPathDist = 0.0f;
	}

	void Actor::Stop()
	{
		// actor.cpp:36-49's `Stop`. `m_NextPosition` is deliberately NOT touched:
		// legacy's Stop does not clear it either - the sentinel is set by the arrival
		// branch (actor.cpp:400), by the A* branch of GotoCell (:462) and by Release.
		// Preserved, and the reason `Detach` exists as a separate, clearer operation.
		m_pathActive = false;
		m_path.Clear();
	}

	bool Actor::EnsureSession()
	{
		if (m_mesh == nullptr)
		{
			return false;
		}

		const std::size_t cells = m_mesh->CellCount();
		if (cells == 0)
		{
			return false;
		}

		// Built fresh rather than reused, because the session sizes its per-cell arrays
		// from the cell count and there is no way to resize them. In practice this runs
		// once per Create: a character's mesh does not change while it is in a map.
		m_session = std::make_unique<Navigation::NavigationSearchSession>(cells);
		return m_session != nullptr;
	}

	std::uint32_t Actor::CurrentCellId() const noexcept
	{
		return m_mesh == nullptr ? kNoCell : m_currentCellId;
	}

	Vector3 Actor::TargetPosition() const
	{
		// actor.cpp:505-511. The LAST waypoint, not the next one, and the sentinel
		// when the path is empty.
		if (m_path.Empty())
		{
			return NoNextPosition();
		}
		return m_path.Waypoints().back().position;
	}

	std::size_t Actor::WaypointsRemaining() const noexcept
	{
		if (m_nextWaypoint >= m_path.Size())
		{
			return 0;
		}
		return m_path.Size() - m_nextWaypoint;
	}

	bool Actor::GotoDestination(const Vector3& from, const Vector3& to)
	{
		if (m_mesh == nullptr)
		{
			return false;
		}

		// actor.cpp:476-495. The whole function is a probe and a delegation.
		Vector3        hitPoint{};
		std::uint32_t hitCell = 0;

		// Legacy's return value is the PROBE's, not the pathfind's. Preserved: see the
		// header. A destination whose probe hit but whose A* failed still broadcasts
		// 3035, because that is what MsgGoto does with it.
		if (!m_mesh->IsCollision(from, to, hitPoint, &hitCell))
		{
			return false;
		}

		GotoCell(hitPoint, hitCell);
		return true;
	}

	bool Actor::GotoCell(const Vector3& position, std::uint32_t cellId)
	{
		// actor.cpp:422-425. These run BEFORE every early return below, because legacy
		// puts them first.
		m_movedDist     = 0.0f;
		m_movedTime     = 0.0f;
		m_movement      = Vector3{ 0.0f, 0.0f, 0.0f };
		m_firstPathDist = 0.0f;

		if (m_mesh == nullptr)
		{
			return false;
		}

		// actor.cpp:429-435: the actor's own cell is re-anchored if its position has
		// drifted outside that cell's column. This self-heal runs on EVERY GOTO and is
		// the only reason a character nudged by a wall slide can still path.
		const Navigation::NavigationCell* startCell = m_mesh->GetCell(m_currentCellId);
		if (startCell == nullptr)
		{
			return false;
		}

		if (!startCell->IsPointInCellCollumn(m_position))
		{
			m_currentCellId = m_mesh->FindClosestCell(m_position);
		}

		// The line-of-sight fast path, actor.cpp:437-453. Two waypoints, no A* - and
		// this is the quirk worth naming: `m_NextPosition` is NOT set to the sentinel
		// here, so it keeps whatever the previous path left. Reproduced.
		if (m_mesh->LineOfSightTest(m_currentCellId, m_position, cellId, position))
		{
			m_path.Setup(m_mesh.get(), m_position, m_currentCellId, position, cellId);
			m_pathActive = true;
			m_path.AddWayPoint(m_position, m_currentCellId);
			m_path.AddWayPoint(position, cellId);

			m_nextWaypoint      = 0;
			m_firstCalc         = false;
			m_firstWaypointPass = false;

			// actor.cpp:450-451 measures the FIRST LEG in XZ with Y zeroed, even though
			// this branch paths all the way to `position`. Reproduced.
			const Vector3 movePosition{ position.x - m_position.x, 0.0f,
				                        position.z - m_position.z };
			m_firstPathDist = movePosition.Length();
			return true;
		}

		// actor.cpp:456-472: the full A*.
		m_pathActive =
		    m_mesh->BuildNavigationPath(m_path, *m_session, m_currentCellId, m_position,
		                                cellId, position);

		if (!m_pathActive)
		{
			return false;
		}

		m_nextWaypoint = 0;
		m_nextPosition = NoNextPosition();

		m_firstCalc         = false;
		m_firstWaypointPass = false;

		// actor.cpp:468-471: the first-leg distance is measured to the FURTHEST VISIBLE
		// waypoint, not to the destination. So the number is a property of this path and
		// this vantage point, and it changes if the smoothing result does.
		const Navigation::NavigationPath::WaypointIndex furthest =
		    m_path.GetFurthestVisibleWayPoint(m_nextWaypoint);

		if (furthest < m_path.Size())
		{
			const Vector3& next  = m_path.Waypoints()[furthest].position;
			const Vector3  movePosition{ next.x - m_position.x, 0.0f,
				                             next.z - m_position.z };
			m_firstPathDist = movePosition.Length();
		}

		return true;
	}

	Status Actor::Update(float elapsedSeconds)
	{
		// actor.cpp:304-305.
		if (m_mesh == nullptr)
		{
			return Status(ErrorCode::InvalidState);
		}
		if (!m_pathActive)
		{
			return Ok();
		}

		// One logical frame.
		//
		// Legacy RECURSES from the arrival branch (actor.cpp:405) and re-runs the whole
		// body, including `m_fMovedTime += elapsedTime` at :308, once per waypoint
		// consumed. Reproduced here as an explicit LOOP over the same frames: identical
		// arithmetic, and a stack depth that cannot depend on how many waypoints one
		// tick happened to eat.
		//
		// The loop is bounded by the waypoint count plus one, because every frame either
		// advances the waypoint index or leaves the loop, and the index is strictly
		// increasing while the path is non-empty. The bound is therefore a formality,
		// and it is written down so a future edit cannot turn a "few waypoints"
		// assumption into an unbounded loop inside a server tick.
		const std::size_t frameLimit = m_path.Size() + 1;

		for (std::size_t frame = 0; frame <= frameLimit; ++frame)
		{
			// actor.cpp:308. Accumulated per frame, so a frame that consumed three
			// waypoints adds 3 * elapsedSeconds. Preserved, and `MovedTime`'s
			// documentation says so.
			m_movedTime += elapsedSeconds;

			// ---- actor.cpp:312-323, steer toward the next waypoint -------------
			if (m_nextWaypoint < m_path.Size())
			{
				m_movement = m_path.Waypoints()[m_nextWaypoint].position - m_position;
			}
			else
			{
				// Path exhausted. Legacy sets m_PathActive false and CONTINUES - it does
				// not return - so the distance and arrival work below still runs with a
				// zero movement vector. Reproduced.
				m_pathActive = false;
				m_movement   = Vector3{ 0.0f, 0.0f, 0.0f };
			}

			// ---- actor.cpp:327-333, clamp to the speed budget ------------------
			//
			// `max_distance = m_MaxSpeed * elapsedTime`. Injected time and nothing
			// else, which is what makes "two half steps travel as far as one whole
			// step" true by construction rather than by a rate constant.
			const float maxDistance = m_maxSpeed * elapsedSeconds;
			float       distance    = m_movement.Length();

			if (distance > maxDistance)
			{
				// `D3DXVec3Normalize` then scale. `Vector3::Normalize` returns Zero for a
				// non-finite or zero-length input where D3DX would produce NaN, which is
				// the one difference in this expression and a strictly safer one: the
				// branch is unreachable for a zero-length vector anyway, since
				// `distance > maxDistance` cannot hold at zero with two non-negatives.
				m_movement = Normalize(m_movement) * maxDistance;
			}

			// ---- actor.cpp:337-339, distance accounting ------------------------
			//
			// `fMoveDist` is the 3D length, NOT the XZ length: the y-zeroed `vMoveDist`
			// built immediately above it is never read (actor.cpp:337 is a dead local)
			// and the comment claiming the distance is XZ is wrong. Only the 3D length is
			// reproduced; the dead local is not, because reproducing a dead computation
			// is not reproducing behaviour.
			m_movedDist += m_movement.Length();

			// ---- actor.cpp:342-344, per-axis deadzone --------------------------
			if (std::fabs(m_movement.x) < kAxisDeadZone) { m_movement.x = 0.0f; }
			if (std::fabs(m_movement.y) < kAxisDeadZone) { m_movement.y = 0.0f; }
			if (std::fabs(m_movement.z) < kAxisDeadZone) { m_movement.z = 0.0f; }

			// ---- actor.cpp:347, the branch -------------------------------------
			//
			// On `distance`, the length BEFORE the clamp - not the movement vector's
			// length after it. Preserved. The two differ when the step was clamped, and
			// using the clamped length here would change which branch a slow frame takes.
			if (distance > kArrivalThreshold)
			{
				// ---- move, actor.cpp:348-372 -----------------------------------
				Vector3        nextPosition = m_position + m_movement;
				std::uint32_t nextCellId   = m_currentCellId;

				// 002d's resolver, untouched - including its `0.98f` wall slide and its
				// truncated-integer progress test. `nextPosition` is passed BY REFERENCE
				// and is corrected in place, exactly as legacy does.
				m_mesh->ResolveMotionOnMesh(m_position, m_currentCellId, nextPosition,
				                           &nextCellId);

				const Navigation::NavigationCell* nextCell = m_mesh->GetCell(nextCellId);
				if (nextCell == nullptr)
				{
					return Status(ErrorCode::InvalidState);
				}

				// The sticky height latch, taking the new height.
				if (nextCell->Normal().y <= kCellNormalFloor)
				{
					nextPosition.y = m_correctY;
				}
				else
				{
					m_correctY = nextPosition.y;
				}

				m_position      = nextPosition;
				m_currentCellId = nextCellId;
				return Ok();
			}

			// ---- arrive, actor.cpp:374-413 ------------------------------------
			//
			// The waypoint is taken EXACTLY, bypassing the movement vector. This is the
			// `0.01` arrival snap: below the threshold the actor is not walked there, it
			// is PLACED there.
			const Navigation::NavigationWaypoint& waypoint = m_path.Waypoints()[m_nextWaypoint];
			m_position = waypoint.position;

			const Navigation::NavigationCell* cell = m_mesh->GetCell(waypoint.cellId);
			if (cell == nullptr)
			{
				// THE ONE DEPARTURE. actor.cpp:378 returns E_FAIL from the recursive
				// frame, and the outer frame discards that return value (:405, :415), so
				// the caller saw S_OK while `m_PathActive` stayed TRUE and
				// `m_NextWaypoint` stayed on the unreachable waypoint. Every later
				// `Update` would repeat it, forever, from inside a server tick.
				//
				// Here the path is cleared and the fault counted, so the actor stops -
				// which is exactly what `!PathIsActive()` makes the caller do. The caller
				// still sees Ok, which is the only part a real client can observe.
				++m_waypointFaults;
				m_pathActive   = false;
				m_path.Clear();
				m_movement     = Vector3{ 0.0f, 0.0f, 0.0f };
				m_nextPosition = NoNextPosition();
				return Ok();
			}

			// The sticky height latch, on the arrival path too.
			if (cell->Normal().y <= kCellNormalFloor)
			{
				m_position.y = m_correctY;
			}
			else
			{
				m_correctY = m_position.y;
			}

			m_movement = Vector3{ 0.0f, 0.0f, 0.0f };

			// actor.cpp:394. The string pull: skip every waypoint visible in a straight
			// line from where the actor now stands. This is the whole of RAN's path
			// smoothing - there is no other.
			m_nextWaypoint = m_path.GetFurthestVisibleWayPoint(m_nextWaypoint);

			if (m_nextWaypoint >= m_path.Size())
			{
				m_pathActive   = false;
				m_movement     = Vector3{ 0.0f, 0.0f, 0.0f };
				m_nextPosition = NoNextPosition();
			}
			else
			{
				m_nextPosition = m_path.Waypoints()[m_nextWaypoint].position;
				// actor.cpp:405: recurse. `continue` below is that recursion.
			}

			// actor.cpp:408-412, OUTSIDE the end-vs-recurse branch, as legacy has it.
			if (m_firstCalc)
			{
				m_firstWaypointPass = true;
			}
			m_firstCalc = true;

			// A finished path leaves immediately. A consumed waypoint loops, which is
			// the recursive re-entry.
			if (!m_pathActive)
			{
				return Ok();
			}
		}

		return Ok();
	}
}
