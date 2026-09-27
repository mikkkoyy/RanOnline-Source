#pragma once

#include "../math/Vector3.h"
#include "../types/Ids.h"

namespace Modern
{
	// Where an entity sits in its lifecycle.
	//
	// Transitions are one-way and checked; Destroyed is terminal. Keeping the
	// state on the entity (rather than as a bool per subclass) means a caller
	// can ask any object "is this live?" and get the same answer.
	enum class EntityState : uint8_t
	{
		// Default-constructed. No identity, cannot be spawned.
		Uninitialized = 0,

		// Identity assigned, not present in a world.
		Created = 1,

		// Present in a world.
		Spawned = 2,

		// Left the world, identity retained. Can be spawned again.
		Despawned = 3,

		// Terminal. Identity released; the slot may be reused after Reset().
		Destroyed = 4,
	};

	const char* ToString(EntityState state) noexcept;

	// Base for anything that can be placed in a world.
	//
	// Templated on the identity type so each entity carries exactly one id of
	// the correct type: a Character holds a CharacterId, not a generic
	// EntityId that has to be narrowed back at every use. It owns its
	// position, direction and lifecycle state, and nothing else.
	template <typename IdType>
	class Entity
	{
	public:
		Entity() = default;
		explicit Entity(const IdType& id) : m_id(id) {}

		const IdType& GetId() const { return m_id; }

		const Vector3& GetPosition() const { return m_position; }
		const Vector3& GetDirection() const { return m_direction; }

		EntityState GetState() const { return m_state; }

		// "Active" and "alive" are the same question asked in two domains: the
		// simulation asks whether it is active, gameplay asks whether it is
		// alive. Both mean "currently spawned".
		bool IsActive() const { return m_state == EntityState::Spawned; }
		bool IsAlive() const { return IsActive(); }

		bool IsDestroyed() const { return m_state == EntityState::Destroyed; }

		// Position and direction are only meaningful once an entity exists, so
		// moving a default-constructed entity is a no-op rather than a silent
		// write to state nothing will ever read.
		void SetPosition(const Vector3& position)
		{
			if (m_state == EntityState::Uninitialized || m_state == EntityState::Destroyed)
			{
				return;
			}

			if (!position.IsFinite())
			{
				return;
			}

			m_position = position;
		}

		// Direction is stored normalised, so consumers never have to ask
		// whether they were handed a unit vector.
		void SetDirection(const Vector3& direction)
		{
			if (m_state == EntityState::Uninitialized || m_state == EntityState::Destroyed)
			{
				return;
			}

			const Vector3 normalized = Normalize(direction);
			if (normalized.IsZero())
			{
				return;
			}

			m_direction = normalized;
		}

	protected:
		void SetId(const IdType& id) { m_id = id; }
		void SetState(EntityState state) { m_state = state; }

		void PlaceAt(const Vector3& position, const Vector3& direction)
		{
			m_position   = position;
			m_direction  = direction;
		}

		// Returns the object to its pre-Create condition, releasing identity.
		void ClearCore()
		{
			m_id        = IdType();
			m_position  = Vector3::Zero;
			m_direction = Vector3::Forward;
			m_state     = EntityState::Uninitialized;
		}

	private:
		IdType      m_id{};
		Vector3     m_position   = Vector3::Zero;
		Vector3     m_direction  = Vector3::Forward;
		EntityState m_state      = EntityState::Uninitialized;
	};
}
