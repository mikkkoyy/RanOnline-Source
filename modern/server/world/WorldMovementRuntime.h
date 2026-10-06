#pragma once

// WORLD-ENTRY-002f: the Field role's world state - one Actor per character, and
// something that advances them.
//
// `GLChar::FrameMove` (GLChar.cpp:5238) is what makes a RAN character walk: for
// every character whose action is GLAT_MOVE, it calls
// `m_actorMove.SetMaxSpeed(GetMoveVelo())`, then `m_actorMove.Update(fElapsedTime)`,
// then `m_vPos = m_actorMove.Position()`. None of that is reachable from a message
// handler, because none of it is triggered BY a message - the message only sets the
// destination.
//
// So the destination and the walking have to be separate things, and this is the
// separation:
//
//     FieldSession / FieldRoleRuntime   the GOTO handler. Runs on the connection's
//                                       worker thread. Sets a destination.
//     WorldMovementRuntime::Tick        the walk. Runs on ONE thread for the whole
//                                       role. Advances every active actor.
//
// ---------------------------------------------------------------------------
// WHY ONE TICKER THREAD AND NOT ONE PER ACTOR
// ---------------------------------------------------------------------------
//
// Per-actor threads would be a thread per connected client, which is how a server
// runs out of handles at a few dozen players. One thread for the role walks every
// actor, and each walk is microseconds of arithmetic plus at most one
// `ResolveMotionOnMesh`. The cost of a thread per actor exceeds the cost of
// serialising the walks by orders of magnitude.
//
// ---------------------------------------------------------------------------
// WHY THERE IS NO TICK RATE
// ---------------------------------------------------------------------------
//
// The ticker SLEEPS in slices, and the slice length is a scheduling detail - not a
// movement rate. What moves the characters is `Tick(elapsed)`, and `elapsed` is
// MEASURED between ticks:
//
//     elapsed = steady_clock::now() - previousTick
//
// The consequence, and it is the property 002c §9 requires: total travel is a
// function of elapsed WALL TIME, not of how many ticks happened. A server running
// 200 ticks a second and one running 20 move a character the same distance per
// second, because each tick moves `speed * measuredDelta`.
//
// Introducing a fixed rate - 20 Hz, 60 Hz - would make a character's movement rate a
// function of the server's frame rate, which is precisely the mistake 002c §3.1
// warns about. RAN has no fixed rate either: `GLChar::FrameMove` receives
// `fElapsedTime`, and the `0.020f` comparison at GLChar.cpp:294 is followed by a
// COMMENTED-OUT `return S_FALSE`, so the frame timer only ever `Sleep(0)`s.
//
// The ticker is therefore NOT the test interface. `Tick(elapsed)` is: a unit test
// calls it with 0.5f and 0.25f and proves two quarters move as far as one half.
// ---------------------------------------------------------------------------
// THE MESH IS SHARED, THE ACTOR IS NOT
// ---------------------------------------------------------------------------
//
// Each slot holds an `Actor` (with its own A* `NavigationSearchSession`) and a
// `shared_ptr<const NavigationMesh>` obtained from the map source. 002e measured
// that 16 of the 56 distinct `.wld` files behind the 99 registered maps are named by
// MORE THAN ONE map identity, so several slots routinely hold the SAME mesh pointer -
// and that is correct. The mesh is immutable, so sharing it is free, and copying it
// would build 99 copies of 56 objects.
//
// Mutable state is per slot and behind that slot's mutex. Nothing mutable is shared,
// so the ONLY concurrency in this class is which thread holds which slot's lock.
//
// ---------------------------------------------------------------------------
// THE ACTOR STATE IS A SNAPSHOT, AND WHY
// ---------------------------------------------------------------------------
//
// A slot keeps a copy of the fields movement needs - `characterClass`,
// `characterGender`, `actState`, `accountLevel` - because `IMovementSpeedProvider`
// takes a `WorldCharacter` and the ticker cannot touch the authoritative record: that
// lives on the Field session's own worker thread.
//
// So the snapshot exists to keep the ticker OFF the authoritative record, not to
// replace it. It is refreshed - never independently edited - by `Attach`,
// `SetActState` and `ApplyGoto`, all of which run on the connection's worker. If the
// snapshot and the authoritative record ever disagreed, the character would walk at a
// speed its state does not imply; the two are written in the same call, immediately
// after each other, and `ActorSnapshot()` is exposed so a test can assert they agree.

#include "GotoService.h"
#include "math/Vector3.h"
#include "movement/Actor.h"
#include "movement/NavigationMapSource.h"
#include "types/Result.h"
#include "world/MovementStateService.h"
#include "world/WorldCharacter.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace Modern::Server::World
{
	// What one character's movement looks like from outside, for tests and logs.
	//
	// A COPY under the slot's lock. Handing out a reference to the live actor would let
	// a reader race the ticker, which is exactly the bug the slot mutex exists to
	// prevent.
	struct ActorSnapshot
	{
		Network::WireU64 sessionId = 0;
		Network::WireU32 gaeaId    = 0;
		Network::WireU32 mapId     = 0;

		Vector3 position{};
		Vector3 target{};

		std::uint32_t currentCellId = Movement::Actor::kNoCell;

		bool  pathActive  = false;
		bool  hasMesh     = false;
		float maxSpeed    = 0.0f;
		float movedDist   = 0.0f;
		float movedTime   = 0.0f;
		std::size_t waypointsRemaining = 0;
		std::size_t waypointFaults     = 0;

		// The movement-identity snapshot's authoritative word, so a caller can compare
		// it with the Field session's own.
		Network::WireU32 actState = 0;
	};

	// The runtime. One per Field role.
	class WorldMovementRuntime
	{
	public:
		WorldMovementRuntime();
		~WorldMovementRuntime();

		WorldMovementRuntime(const WorldMovementRuntime&)            = delete;
		WorldMovementRuntime& operator=(const WorldMovementRuntime&) = delete;

		// ---- wiring, all before Start() ------------------------------------

		// Where meshes come from. Borrowed; may be null, in which case every character
		// spawns with no mesh and every GOTO is `RejectedNoMesh`.
		void SetMapSource(const Movement::INavigationMapSource* source) noexcept
		{
			m_maps = source;
		}

		// The movement-state rule, which owns the speed seam. Borrowed; may be null.
		void SetMovementStateService(const MovementStateService* service) noexcept
		{
			m_movement = service;
		}

		// The GOTO rule. Built from the movement-state service at Set time, because a
		// service that changes under a built GotoService would be a dangling borrow.
		void SetGotoService(const GotoService* service) noexcept { m_goto = service; }

		// ---- lifecycle ------------------------------------------------------

		// Starts the ticker thread. Idempotent-safe: a second Start is refused rather
		// than starting a second thread.
		//
		// Calling Start is OPTIONAL. `Tick` is public and is the test interface; a
		// caller that drives movement itself should not start a ticker.
		Status StartTicker();
		void   StopTicker() noexcept;
		bool   TickerRunning() const noexcept { return m_tickerRunning.load(); }

		// ---- per-character state --------------------------------------------

		// Binds a character to an actor on `character`'s map.
		//
		// Returns Ok even when the map has no navigation mesh. That is the point: RAN's
		// field servers always have their meshes, but a modern server started without
		// an asset root must still accept a world entry and still spawn, and the
		// character must be told "no mesh" rather than refused a login it is entitled
		// to. A GOTO for such a character is `RejectedNoMesh`, which is a different
		// answer from "you may not be here".
		//
		// `nowMs` is unused by the movement rule and exists only so a future
		// age-based rule has the clock it would need; movement is driven by injected
		// elapsed time, never by a timestamp.
		Status Attach(Network::WireU64 sessionId, const WorldCharacter& character);

		// Forgets a character. Safe to call for one that was never attached.
		void Detach(Network::WireU64 sessionId);

		// The authoritative movement word changed elsewhere - a 3032, most often.
		//
		// Called by the Field role immediately after it writes the authoritative
		// character, so the snapshot the ticker reads is never a tick behind a state
		// change the client has already been told about.
		Status SetActState(Network::WireU64 sessionId, Network::WireU32 actState);

		// Applies one 3034.
		//
		// Runs the whole legacy sequence under the slot's lock: the die check, the run
		// bit, the 60-unit comparison, the ±10 probe, the speed, and - when accepted -
		// writing the new authoritative state back into `character`.
		//
		// `character` is the Field session's own authoritative record, mutated in place
		// so the session and the snapshot cannot drift. `result` carries what the
		// caller needs to build the 3035 and to log why nothing was sent.
		Status ApplyGoto(Network::WireU64 sessionId, WorldCharacter& character,
		                 const Network::Goto::GotoRequest& request, GotoResult& result);

		// ---- the walk --------------------------------------------------------

		// Advances every attached actor by `elapsedSeconds`.
		//
		// This is the ONLY way movement happens, and `elapsedSeconds` is injected. The
		// ticker thread calls it with a measured delta; a unit test calls it with
		// whatever it wants.
		//
		// Returns the number of actors whose position CHANGED. That is the honest
		// measure of "movement happened" for a server that has no per-tick position
		// packet to count.
		std::size_t Tick(float elapsedSeconds);

		// ---- observation ------------------------------------------------------

		// A copy of one character's movement state, or false when there is no slot.
		bool Snapshot(Network::WireU64 sessionId, ActorSnapshot& out) const;

		std::size_t ActorCount() const noexcept { return m_slots.size(); }

		// Slots with no mesh - a deployment problem rather than a player's.
		std::size_t MeshlessActorCount() const;

		// Monotonic counters, for the operator log and for tests. Written under the
		// list lock, read after Stop or with the lock held by `Snapshot`.
		std::size_t GotoAcceptedCount() const noexcept { return m_gotoAccepted.load(); }
		std::size_t GotoRejectedCount() const noexcept { return m_gotoRejected.load(); }
		std::size_t ArrivalCount() const noexcept { return m_arrivals.load(); }
		std::size_t TickCount() const noexcept { return m_ticks.load(); }

	private:
		// One character in the world.
		//
		// The mutex is per slot, so the ticker can advance one actor while a Field
		// worker sets another's destination. A single runtime-wide lock would serialise
		// every character in the world behind the slowest one, for no benefit: the
		// actors share nothing mutable.
		struct Slot
		{
			Network::WireU64 sessionId = 0;
			Network::WireU32 gaeaId    = 0;
			Network::WireU32 mapId     = 0;

			Movement::Actor actor{};

			// The movement-identity snapshot. The fields `IMovementSpeedProvider` may
			// read, copied so the ticker never touches the authoritative record. Never
			// edited directly - refreshed by Attach, SetActState and ApplyGoto.
			WorldCharacter identity{};

			bool hasMesh = false;

			mutable std::mutex mutex{};
		};

		using SlotPtr = std::shared_ptr<Slot>;

		SlotPtr FindLocked(Network::WireU64 sessionId) const;

		// The ticker thread's body. Named so `StartTicker` is a two-line function and
		// the loop's structure is readable on its own.
		void TickerLoop();

		const Movement::INavigationMapSource* m_maps      = nullptr;
		const MovementStateService*            m_movement  = nullptr;
		const GotoService*                      m_goto      = nullptr;

		// Guards `m_slots` only - copied or mutated, never held across a mesh
		// resolution, an A* search or a send.
		mutable std::mutex m_slotsMutex{};
		std::vector<SlotPtr> m_slots{};

		std::thread m_ticker{};
		std::atomic<bool> m_stopTicker{false};
		std::atomic<bool> m_tickerRunning{false};

		std::atomic<std::size_t> m_gotoAccepted{0};
		std::atomic<std::size_t> m_gotoRejected{0};
		std::atomic<std::size_t> m_arrivals{0};
		std::atomic<std::size_t> m_ticks{0};
	};
}
