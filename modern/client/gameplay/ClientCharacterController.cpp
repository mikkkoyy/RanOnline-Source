#include "gameplay/ClientCharacterController.h"

#include <algorithm>
#include <cmath>

namespace Modern::Client::Gameplay
{

	void ClientCharacterController::SetMoveDirection(Vector3 direction) noexcept
	{
		// Normalise so diagonal movement isn't faster than axial.
		if (!direction.IsZero())
		{
			direction = Normalize(direction);
		}
		moveDirection = direction;
	}

	void ClientCharacterController::Update(
	    float dt,
	    std::function<std::optional<float>(float, float)> groundQuery,
	    const Vector3& fieldMin,
	    const Vector3& fieldMax) noexcept
	{
		if (dt <= 0.0f) { return; }

		// 1. Horizontal movement on the XZ plane, framerate-independent.
		const Vector3 moveDelta = moveDirection * (moveSpeed * dt);
		position.x += moveDelta.x;
		position.z += moveDelta.z;

		// Clamp XZ to the field bounding box so the character can't walk off.
		position.x = std::clamp(position.x, fieldMin.x, fieldMax.x);
		position.z = std::clamp(position.z, fieldMin.z, fieldMax.z);

		// 2. Ground query at the new XZ position.
		const std::optional<float> groundY = groundQuery ? groundQuery(position.x, position.z)
		                                                 : std::nullopt;

		if (groundY.has_value())
		{
			// Snap to ground: target Y is ground + offset.
			const float targetY = *groundY + groundOffset;

			if (position.y <= targetY + 0.01f)
			{
				// On or just above ground: snap down, kill vertical velocity.
				position.y = targetY;
				verticalVelocity = 0.0f;
				onGround = true;
			}
			else
			{
				// Above ground: fall toward it under gravity.
				verticalVelocity = std::max(verticalVelocity - gravity * dt, maxFallSpeed);
				position.y += verticalVelocity * dt;

				// Don't fall through the ground in one big step.
				if (position.y < targetY)
				{
					position.y = targetY;
					verticalVelocity = 0.0f;
					onGround = true;
				}
				else
				{
					onGround = false;
				}
			}
		}
		else
		{
			// No ground beneath us: fall under gravity.
			verticalVelocity = std::max(verticalVelocity - gravity * dt, maxFallSpeed);
			position.y += verticalVelocity * dt;
			onGround = false;
		}

		// Clamp Y to the field's valid range so the character never ends up
		// below the world floor.
		position.y = std::clamp(position.y, fieldMin.y, fieldMax.y);
	}

} // namespace Modern::Client::Gameplay
