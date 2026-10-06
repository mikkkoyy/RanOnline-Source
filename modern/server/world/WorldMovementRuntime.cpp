#include "world/WorldMovementRuntime.h"

#include "MovementStateProtocol.h"

#include <chrono>
#include <utility>

namespace Modern::Server::World
{
	namespace
	{
		// How long the ticker SLEEPS between wake-ups.
		//
		// NOT a movement rate. It bounds how long a character can wait before its next
		// measured slice, and 4 ms keeps a walking character's motion smooth while
		// costing an order of magnitude less than a busy loop. The distance a character
		// travels per second depends on the MEASURED delta between ticks, not on this
		// number - see the header. Changing it changes the smoothness of movement and
		// nothing else.
		constexpr int kTickerSliceMilliseconds = 4;

		// A delta larger than this is not a movement slice, it is a pause - a debugger
		// break, a machine suspended overnight. Feeding such a value to `Update` would
		// teleport a character across the map in one step, because `maxDistance =
		// speed * elapsed` would be enormous. Clamped, so a resumed process does not
		// teleport anybody.
		//
		// The clamp bounds the SPEED of recovery, never the total distance: the next
		// slice simply carries the rest.
		constexpr float kMaximumTickSeconds = 1.0f;

	}

	WorldMovementRuntime::WorldMovementRuntime() = default;

	WorldMovementRuntime::~WorldMovementRuntime()
	{
		StopTicker();
	}

	Status WorldMovementRuntime::StartTicker()
	{
		if (m_tickerRunning.load(std::memory_order_acquire))
		{
			// Refused rather than silently ignored: a second Start would be a second
			// thread walking the same actors, and two threads advancing one Actor is a
			// data race with no correct outcome.
			return Status(ErrorCode::AlreadyExists);
		}

		m_stopTicker.store(false, std::memory_order_release);
		m_tickerRunning.store(true, std::memory_order_release);
		m_ticker = std::thread([this] { TickerLoop(); });
		return Ok();
	}

	void WorldMovementRuntime::StopTicker() noexcept
	{
		if (!m_tickerRunning.exchange(false, std::memory_order_acq_rel))
		{
			return;
		}

		m_stopTicker.store(true, std::memory_order_release);
		if (m_ticker.joinable())
		{
			m_ticker.join();
		}
	}

	void WorldMovementRuntime::TickerLoop()
	{
		auto previous = std::chrono::steady_clock::now();

		while (!m_stopTicker.load(std::memory_order_acquire))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(kTickerSliceMilliseconds));

			const auto now = std::chrono::steady_clock::now();

			// MEASURED, not assumed. This is the whole of the tick design: the delta
			// between two real instants. A character walking at `speed` covers
			// `speed * this` units, and so covers the same distance per second whether
			// this loop runs 20 times or 200.
			float elapsed = std::chrono::duration<float>(now - previous).count();
			previous     = now;

			if (elapsed < 0.0f)
			{
				// Not reachable with a monotonic clock, and handled rather than passed
				// on: a negative delta would move a character BACKWARDS.
				elapsed = 0.0f;
			}
			if (elapsed > kMaximumTickSeconds)
			{
				elapsed = kMaximumTickSeconds;
			}

			if (elapsed > 0.0f)
			{
				(void)Tick(elapsed);
			}
		}
	}

	WorldMovementRuntime::SlotPtr WorldMovementRuntime::FindLocked(
	    Network::WireU64 sessionId) const
	{
		for (const SlotPtr& slot : m_slots)
		{
			if (slot->sessionId == sessionId)
			{
				return slot;
			}
		}
		return nullptr;
	}

	Status WorldMovementRuntime::Attach(Network::WireU64 sessionId,
	                                    const WorldCharacter& character)
	{
		if (sessionId == 0 || character.id.value == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		SlotPtr slot = std::make_shared<Slot>();
		slot->sessionId   = sessionId;
		slot->gaeaId      = character.gaeaId;
		slot->mapId       = character.saveMapId.value;
		slot->identity    = character;

		// The mesh is resolved BEFORE the slot is published, so no reader ever sees a
		// half-built slot. A null source or an unavailable map leaves `hasMesh` false
		// and the character still attached - see the header on why world entry is not
		// refused for a missing asset.
		std::shared_ptr<const Navigation::NavigationMesh> mesh;
		if (m_maps != nullptr)
		{
			mesh = m_maps->MeshForPackedMapId(slot->mapId);
		}

		if (mesh != nullptr)
		{
			const Vector3 spawn{ character.savePosition.x, character.savePosition.y,
				                character.savePosition.z };

			// `Actor::Create` runs legacy's ±5 spawn probe, so the actor lands on the
			// mesh rather than merely being told where it is. A character whose DB
			// position is off the walkable surface therefore spawns at the nearest
			// walkable point - which is what RAN does.
			if (const Status status = slot->actor.Create(mesh, spawn, Movement::Actor::kNoCell);
			    status.IsOk())
			{
				slot->hasMesh = true;
			}
		}

		const std::lock_guard<std::mutex> lock(m_slotsMutex);

		// A second Attach for one session replaces the first. Reachable only if a Field
		// role were to reuse a session id, and replacing is the safer of the two
		// answers: keeping both would leave an orphaned actor walking a map nobody is
		// watching.
		for (SlotPtr& existing : m_slots)
		{
			if (existing->sessionId == sessionId)
			{
				existing = slot;
				return Ok();
			}
		}

		m_slots.push_back(std::move(slot));
		return Ok();
	}

	void WorldMovementRuntime::Detach(Network::WireU64 sessionId)
	{
		SlotPtr detached;
		{
			const std::lock_guard<std::mutex> lock(m_slotsMutex);
			for (auto it = m_slots.begin(); it != m_slots.end(); ++it)
			{
				if ((*it)->sessionId == sessionId)
				{
					detached = *it;
					m_slots.erase(it);
					break;
				}
			}
		}

		if (detached == nullptr)
		{
			return;
		}

		// The actor is torn down AFTER the slot leaves the list, so no ticker iteration
		// can be looking at it. `Detach` on the actor is what drops the mesh reference.
		const std::lock_guard<std::mutex> lock(detached->mutex);
		detached->actor.Detach();
	}

	Status WorldMovementRuntime::SetActState(Network::WireU64 sessionId,
	                                          Network::WireU32 actState)
	{
		const std::lock_guard<std::mutex> listLock(m_slotsMutex);
		const SlotPtr                      slot = FindLocked(sessionId);
		if (slot == nullptr)
		{
			return Status(ErrorCode::NotFound);
		}

		const std::lock_guard<std::mutex> lock(slot->mutex);
		slot->identity.actState = actState;
		return Ok();
	}

	Status WorldMovementRuntime::ApplyGoto(Network::WireU64 sessionId,
	                                       WorldCharacter&                character,
	                                       const Network::Goto::GotoRequest& request,
	                                       GotoResult& result)
	{
		result = GotoResult{};

		if (m_goto == nullptr)
		{
			result.outcome = GotoOutcome::RejectedNoMesh;
			result.detail  = "no GOTO rule is configured on this Field role";
			return Status(ErrorCode::InvalidState);
		}

		const std::lock_guard<std::mutex> listLock(m_slotsMutex);
		const SlotPtr                      slot = FindLocked(sessionId);
		if (slot == nullptr)
		{
			result.outcome = GotoOutcome::RejectedDestination;
			result.detail  = "the session has no movement state; was it spawned?";
			return Status(ErrorCode::NotFound);
		}

		// One slot lock for the whole sequence. Held across an A* search and a mesh
		// probe, which is a longer critical section than the walker's - and correct,
		// because the destination must not be set while the walker is mid-step on the
		// same actor. The list lock is released as soon as the slot is found, so two
		// different characters never contend on it.
		const std::lock_guard<std::mutex> lock(slot->mutex);

		GotoRequest serviceRequest;
		serviceRequest.requestedActState = request.actState;
		serviceRequest.claimedCurrent    = Vector3{ request.currentPosition.x,
			                                        request.currentPosition.y,
			                                        request.currentPosition.z };
		serviceRequest.requestedTarget   = Vector3{ request.targetPosition.x,
			                                        request.targetPosition.y,
			                                        request.targetPosition.z };

		// The rule mutates `slot->identity`, and the authoritative record is written
		// immediately afterwards from the same word. Both writes happen under this one
		// lock, so the snapshot the ticker reads can never disagree with the record the
		// Field session holds.
		result = m_goto->Apply(slot->actor, slot->identity, serviceRequest);

		character.actState = slot->identity.actState;

		if (result.accepted)
		{
			m_gotoAccepted.fetch_add(1, std::memory_order_relaxed);
		}
		else
		{
			m_gotoRejected.fetch_add(1, std::memory_order_relaxed);
			if (result.outcome == GotoOutcome::RejectedNoMesh && m_maps != nullptr)
			{
				// Replace the generic message with the map source's, which names the map
				// and the registry's own reason. This is the difference an operator
				// debugging a missing asset needs, and it costs one call on a path that
				// is already failing.
				result.detail = m_maps->Describe(slot->mapId);
			}
		}

		return Ok();
	}

	std::size_t WorldMovementRuntime::Tick(float elapsedSeconds)
	{
		if (elapsedSeconds <= 0.0f)
		{
			return 0;
		}

		// The list is COPIED under the lock and then walked without it, so an Attach or
		// a Detach on a Field worker is never blocked by a walk in progress - and, more
		// importantly, a walk is never blocked by a slow Field worker.
		std::vector<SlotPtr> targets;
		{
			const std::lock_guard<std::mutex> lock(m_slotsMutex);
			targets = m_slots;
		}

		std::size_t moved = 0;

		for (const SlotPtr& slot : targets)
		{
			const std::lock_guard<std::mutex> lock(slot->mutex);

			if (!slot->hasMesh || !slot->actor.PathIsActive())
			{
				continue;
			}

			const Vector3 before = slot->actor.Position();

			// GLChar.cpp:6089: the speed is refreshed EVERY tick, not once per GOTO,
			// because `GetMoveVelo()` reads the state word and that word can change
			// between ticks. Reproduced: a 3032 that arrives mid-walk changes the speed
			// on the very next slice.
			if (m_movement != nullptr && m_movement->HasSpeedProvider())
			{
				slot->actor.SetMaxSpeed(m_movement->SpeedFor(slot->identity));
			}

			const bool wasActive = true;
			(void)slot->actor.Update(elapsedSeconds);
			const bool nowActive = slot->actor.PathIsActive();

			if (before != slot->actor.Position())
			{
				++moved;
			}

			if (wasActive && !nowActive)
			{
				// GLChar.cpp:6091-6095: when the path ends, stop and turn idle. The
				// modern equivalent of TurnAction(GLAT_IDLE) is "no longer walking",
				// which is what `pathActive == false` already means to every caller.
				m_arrivals.fetch_add(1, std::memory_order_relaxed);
			}
		}

		m_ticks.fetch_add(1, std::memory_order_relaxed);
		return moved;
	}

	bool WorldMovementRuntime::Snapshot(Network::WireU64 sessionId,
	                                    ActorSnapshot& out) const
	{
		const std::lock_guard<std::mutex> listLock(m_slotsMutex);
		const SlotPtr                      slot = FindLocked(sessionId);
		if (slot == nullptr)
		{
			return false;
		}

		const std::lock_guard<std::mutex> lock(slot->mutex);
		out.sessionId          = slot->sessionId;
		out.gaeaId             = slot->gaeaId;
		out.mapId              = slot->mapId;
		out.position           = slot->actor.Position();
		out.target             = slot->actor.TargetPosition();
		out.currentCellId      = slot->actor.CurrentCellId();
		out.pathActive         = slot->actor.PathIsActive();
		out.hasMesh            = slot->hasMesh;
		out.maxSpeed           = slot->actor.MaxSpeed();
		out.movedDist          = slot->actor.MovedDist();
		out.movedTime          = slot->actor.MovedTime();
		out.waypointsRemaining = slot->actor.WaypointsRemaining();
		out.waypointFaults     = slot->actor.WaypointFaults();
		out.actState           = slot->identity.actState;
		return true;
	}

	std::size_t WorldMovementRuntime::MeshlessActorCount() const
	{
		const std::lock_guard<std::mutex> lock(m_slotsMutex);

		std::size_t count = 0;
		for (const SlotPtr& slot : m_slots)
		{
			if (!slot->hasMesh)
			{
				++count;
			}
		}
		return count;
	}

}
