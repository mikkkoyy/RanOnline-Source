#pragma once

// WORLD-ENTRY-002f: the headless `Actor` - authoritative movement over one mesh.
//
// Transliterated from legacy/Lib_Engine/NaviMesh/actor.cpp and actor.h, with no
// D3DX, no device, no frame graph and no map engine. The 002d navigation kernel
// already exists and is used AS IS: `NavigationMesh`, `NavigationCell`,
// `NavigationPath`, `NavigationSearchSession`, `ResolveMotionOnMesh`,
// `IsCollision`, `LineOfSightTest`, `BuildNavigationPath`. None of it is
// reimplemented, wrapped or second-guessed here.
//
// ---------------------------------------------------------------------------
// THE THREE LITERALS, NAMED
// ---------------------------------------------------------------------------
//
// Legacy hides three thresholds in inline arithmetic. They are the whole
// behaviour of `Update` and are named here so a caller can assert on them:
//
//     actor.cpp:342-344   |component| < 0.001f  ->  component = 0
//     actor.cpp:347       distance  > 0.01f     ->  move, else ARRIVE
//     actor.cpp:361,382   Normal().y <= 0.0001f  ->  pin Y to the last good height
//
// The third is not a detail. It is a STICKY LATCH: `m_CorrectY` remembers the last
// Y that came off a cell with a usable normal, and a step that lands on a
// degenerate-normal cell reuses it instead. That is what stops a character popping
// through a floor at a wall seam. actor.h:22-25 says exactly this, and the latch is
// per-member rather than `static` precisely so two actors cannot tangle over it.
//
// ---------------------------------------------------------------------------
// THE 10 AND THE 5 ARE DIFFERENT NUMBERS, AND THE DIFFERENCE MATTERS
// ---------------------------------------------------------------------------
//
//   * GOTO probes ±10 about the destination (actor.cpp:295-296). That is
//     `NavigationCell::kDestinationProbeHalfHeight`, and `GotoDestination` uses it.
//   * Create/SetPosition probe ±5 (actor.cpp:74, :121). Spawn settle, not GOTO.
//
// Using 10 for spawn or 5 for GOTO would be a plausible-looking change that moves
// characters, so both are named here rather than being passed in.
//
// ---------------------------------------------------------------------------
// ONE SHARED MESH, MANY ACTORS, NO LOCK
// ---------------------------------------------------------------------------
//
// The mesh is a `shared_ptr<const NavigationMesh>`. WORLD-ENTRY-002e established
// that 16 of the 56 distinct `.wld` files behind the 99 registered maps are named by
// MORE THAN ONE map identity, so "one mesh per character" is the wrong shape: it
// would build 99 copies of 56 objects and give each character its own copy of state
// that is immutable anyway.
//
// The per-cell A* state lives in this class's own `NavigationSearchSession`, sized
// to the mesh at Create time. That is the 002d design and it is what lets several
// Field workers walk ONE mesh at once. Nothing mutable is written into the mesh or
// into a cell.
//
// ---------------------------------------------------------------------------
// AN ACTOR IS NOT THREAD-SAFE BY ITSELF; THE OWNER MAKES IT SO
// ---------------------------------------------------------------------------
//
// One Actor belongs to one character's movement state, and that state is mutated by
// the GOTO handler and by the movement tick. Those are two threads, so the OWNER
// (WorldMovementRuntime) holds a mutex per actor. This class deliberately has no
// lock of its own: a lock here would imply the object is safe to share, and it is
// not. Documented rather than silently assumed.
//
// ---------------------------------------------------------------------------
// WHAT IS FAITHFUL AND WHAT IS NOT - THE FOUR DEPARTURES
// ---------------------------------------------------------------------------
//
// Faithful, and asserted by tests:
//
//   * every constant above,
//   * `max_distance = m_MaxSpeed * elapsedTime`, with elapsed time INJECTED. There
//     is no fixed rate anywhere in this file, and 002c §9 measured that RAN has none
//     either: `GLChar::FrameMove` receives `fElapsedTime` and the `0.020f` compare at
//     :294 is followed by a COMMENTED-OUT `return S_FALSE`. Converting elapsed time
//     into a fixed per-tick constant would let the movement rate depend on the
//     server's frame rate, which is the one thing 002c §3.1 forbids.
//   * the wall slide is `NavigationMesh::ResolveMotionOnMesh` with its `0.98f`,
//     untouched. The comment in legacy says 10 %; the code says 2 %. 002d already
//     kept the code and this milestone keeps 002d.
//   * `m_NextPosition = (FLT_MAX, FLT_MAX, FLT_MAX)` as "no valid next position",
//     set on the A* branch (actor.cpp:462) and on arrival (actor.cpp:400) but NOT on
//     the line-of-sight branch (:441-448), which leaves it stale.
//   * `GotoDestination` returns whether the ±10 PROBE hit, not whether the pathfind
//     succeeded. So a GOTO whose probe hit but whose A* failed still broadcasts
//     3035, and the actor stops on the next tick. See the .cpp.
//
// One deliberate departure, named because it is the only one:
//
//   * A waypoint whose cell does not exist (actor.cpp:378) leaves `m_PathActive`
//     true in legacy, because the recursive frame's E_FAIL is discarded by the
//     outer frame (:405, :415) - which strands the actor on a stale waypoint
//     forever. This implementation instead clears the path and increments an
//     observable fault counter, so the actor stops instead of spinning. Both
//     versions report success to the caller, which is the part any real client can
//     observe; only the stall differs, and a stall inside a server tick is a hang.

#include "math/Vector3.h"
#include "navigation/NavigationMesh.h"
#include "navigation/NavigationPath.h"
#include "navigation/NavigationSearchSession.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace Modern::Movement
{
	// actor.cpp:342-344. A per-axis deadzone: a component smaller than this is zeroed
	// before the step, which is also what makes a pure-vertical step (x and z both
	// zeroed) not advance at all.
	inline constexpr float kAxisDeadZone = 0.001f;

	// actor.cpp:347. `distance > kArrivalThreshold` selects the move branch; anything
	// at or below it takes the arrival branch, which SNAPS to the waypoint exactly.
	inline constexpr float kArrivalThreshold = 0.01f;

	// actor.cpp:361 and :382. `Normal().y <= kCellNormalFloor` means "this cell's
	// plane is not usable as a floor" - a wall or a degenerate triangle - so the new
	// Y reuses the last good one instead of being taken from the plane.
	inline constexpr float kCellNormalFloor = 0.0001f;

	// `m_MaxSpeed`'s constructor value, actor.cpp:106.
	//
	// NOT a gameplay value. It is the number an Actor has before anything sets a
	// speed, and the GOTO path always sets one before walking. Exposed so the
	// "unset" case is a named state rather than an accident.
	inline constexpr float kUnsetMaxSpeed = 5.0f;

	// actor.cpp:74 and :121 - spawn settle.
	inline constexpr float kSpawnProbeHalfHeight = 5.0f;

	// actor.cpp:295-296 - the GOTO destination probe. Equal to
	// `NavigationCell::kDestinationProbeHalfHeight`; asserted below rather than
	// restated, because two names for one number is how they drift.
	inline constexpr float kGotoProbeHalfHeight = 10.0f;

	static_assert(kGotoProbeHalfHeight == Navigation::NavigationCell::kDestinationProbeHalfHeight,
	              "the GOTO probe and NavigationCell's destination probe are the same 10 units");

	// RAN's "no valid next position" sentinel, actor.cpp:19 and :400.
	//
	// FLT_MAX in all three components, and compared COMPONENT-WISE at GLChar.cpp:6104
	// rather than as a vector. Preserved exactly: `Vector3` has no operator> so a
	// caller must check per component, and doing so is what legacy does.
	Vector3 NoNextPosition() noexcept;

	// True when `value` is NOT the sentinel, i.e. there is a next position.
	bool HasNextPosition(const Vector3& value) noexcept;

	class Actor
	{
	public:
		Actor() = default;
		~Actor();

		Actor(const Actor&)            = delete;
		Actor& operator=(const Actor&) = delete;

		// actor.cpp:58-104. Binds the mesh and settles onto it.
		//
		// The ±5 probe runs FIRST: a hit adopts the hit point and its cell verbatim,
		// and only a miss falls back to `FindClosestCell` + `SnapPointToCell`. The
		// `cellId` argument is therefore ADVISORY and is overwritten either way - which
		// is what legacy does, and which is why a caller passing a stale cell id does
		// not get stranded.
		//
		// Refuses a null mesh or a mesh that is not built. Legacy would happily keep a
		// null `m_Parent` and then fail every `Update`, so the failure is reported at
		// the point where it can still be acted on.
		Status Create(std::shared_ptr<const Navigation::NavigationMesh> mesh,
		              const Vector3& position, std::uint32_t cellId);

		// Detaches the mesh and clears the path. Not `Actor::Release`: that legacy
		// function also resets `m_NextPosition` to `(0,0,0)` (actor.cpp:41) instead of
		// the sentinel, which 002c §9 identifies as a hazard - a stale next position
		// feeds the facing computation at GLChar.cpp:6104. Nothing in this milestone
		// calls Release, so carrying a function whose only effect is a hazard would be
		// carrying a bug, not a behaviour.
		void Detach();

		// actor.cpp:476-495. The ±10 destination probe, and the entry point MsgGoto
		// uses.
		//
		// Returns whether the PROBE HIT. That is legacy's return value and it is not
		// the same question as "will the actor walk there":
		//
		//   * probe miss -> false. MsgGoto's `bSucceed` is FALSE, so NO 3035 goes out,
		//     nothing moves, and the caller is told nothing happened.
		//   * probe hit, pathfind OK -> true, and the actor walks.
		//   * probe hit, pathfind FAILED -> still TRUE. MsgGoto broadcasts 3035 and then
		//     `FrameMove` sees `!PathIsActive()` on the very next tick and stops the
		//     actor (GLChar.cpp:6091-6095).
		//
		// Reproduced exactly, including the third case. Making the return value mean
		// "the walk will happen" would change which clients get a 3035 for a
		// destination their own copy of the world cannot path to.
		//
		// `from` and `to` are the destination ± `kGotoProbeHalfHeight` in Y; the caller
		// computes them, because legacy's caller is the only thing that knows where the
		// destination is.
		bool GotoDestination(const Vector3& from, const Vector3& to);

		// actor.cpp:418-474. Paths to a resolved point and cell.
		//
		// `return` value is whether the path is ACTIVE afterwards, which is what makes
		// it testable independently of the probe.
		//
		// Returns false - having cleared `ResetMovedData` and zeroed the movement
		// vector first, exactly as legacy does - when the actor is not on a cell at
		// all, or when the A* found no route. Legacy returns void here and leaves the
		// caller's only signal being `PathIsActive()`; returning it makes the
		// distinction reportable instead of inferred.
		bool GotoCell(const Vector3& position, std::uint32_t cellId);

		// actor.cpp:54. Stops walking and clears the path.
		//
		// Legacy does NOT clear `m_NextPosition` here either (actor.cpp:36-49 sets it to
		// the sentinel; that function is `Release`, not `Stop` - `Stop` only clears
		// `m_PathActive`, `m_Path` and `m_NextWaypoint`). Preserved.
		void Stop();

		// actor.cpp:302-416. Advances the actor by `elapsedSeconds`.
		//
		// `elapsedSeconds` is INJECTED. The caller measures it; this class never reads a
		// clock, so a unit test can prove "two half-steps travel as far as one whole
		// step" without sleeping, and a server can drive any tick rate it likes.
		//
		// Returns InvalidState for an actor with no mesh (legacy's `E_FAIL` at :304),
		// Ok otherwise - including the "no path active" case, which legacy reports as
		// `S_OK` and which is the overwhelmingly common one.
		Status Update(float elapsedSeconds);

		// ---- state ------------------------------------------------------------

		void  SetMaxSpeed(float speed) noexcept { m_maxSpeed = speed; }
		float MaxSpeed() const noexcept { return m_maxSpeed; }

		const Vector3& Position() const noexcept { return m_position; }
		const Vector3& Movement() const noexcept { return m_movement; }
		const Vector3& NextPosition() const noexcept { return m_nextPosition; }

		// Legacy's `CurrentCellID()`, actor.h:84. With no mesh there is no cell, so 0
		// rather than a silent -1: `kNoCell` exists so the difference between "cell 0"
		// and "no cell" is readable.
		static constexpr std::uint32_t kNoCell = 0xFFFFFFFFu;
		std::uint32_t CurrentCellId() const noexcept;

		bool PathIsActive() const noexcept { return m_pathActive; }

		// actor.cpp:505-511. The LAST waypoint - the goal - or the sentinel when the
		// path is empty. Not the next waypoint.
		Vector3 TargetPosition() const;

		// actor.cpp:60-64. `MovedTime` is the accumulated injected time, which is NOT
		// wall clock: legacy's arrival branch re-enters `Update`, and each re-entry adds
		// `elapsedTime` again (actor.cpp:308). That inflation is reproduced, so this
		// counter measures "frames visited", not seconds.
		float MovedDist() const noexcept { return m_movedDist; }
		float MovedTime() const noexcept { return m_movedTime; }

		// actor.cpp:58 and :64.
		float FirstPathDist() const noexcept { return m_firstPathDist; }
		bool  FirstWaypointPassed() const noexcept { return m_firstWaypointPass; }

		// The waypoints still ahead of the actor, `kEnd` included. For tests and logs.
		std::size_t WaypointsRemaining() const noexcept;

		// Waypoints a waypoint's cell could not be resolved for. Non-zero means the
		// actor met the one departure described in the header.
		std::size_t WaypointFaults() const noexcept { return m_waypointFaults; }

		const Navigation::NavigationPath& Path() const noexcept { return m_path; }

		const std::shared_ptr<const Navigation::NavigationMesh>& Mesh() const noexcept
		{
			return m_mesh;
		}

	private:
		// Creates the per-actor A* session if the mesh has changed size. Returns false
		// only if a session of that size cannot be made, which cannot happen for a
		// built mesh - but a mesh that is not built reports zero cells, and a
		// zero-cell session would make every search trivially empty.
		bool EnsureSession();

		std::shared_ptr<const Navigation::NavigationMesh> m_mesh{};
		std::unique_ptr<Navigation::NavigationSearchSession> m_session{};

		std::uint32_t m_currentCellId = kNoCell;
		Vector3       m_position{};
		Vector3       m_movement{};
		Vector3       m_nextPosition = NoNextPosition();
		float         m_maxSpeed     = kUnsetMaxSpeed;

		// The sticky last-good-height latch. actor.h:22-25, and the reason it is a
		// member rather than a static.
		float m_correctY = 0.0f;

		bool                      m_pathActive = false;
		Navigation::NavigationPath m_path{};
		Navigation::NavigationPath::WaypointIndex m_nextWaypoint = 0;

		bool m_firstCalc        = false;
		bool m_firstWaypointPass = false;
		float m_firstPathDist   = 0.0f;

		float m_movedDist = 0.0f;
		float m_movedTime = 0.0f;

		std::size_t m_waypointFaults = 0;
	};
}
