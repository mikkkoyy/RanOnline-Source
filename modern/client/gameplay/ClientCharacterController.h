#pragma once

// WORLD-PLAYABLE-001C: client-side character controller.
//
// Holds position, velocity and movement state. Movement is framerate-independent
// (metres per second) and ground height is supplied by an injected query
// callable, so the controller itself never touches terrain data, Direct3D or
// any platform type. A future Vulkan or Metal backend can reuse it unchanged.
//
// Grounding: the caller provides a ground-height query (typically backed by the
// WORLD-PLAYABLE-001B terrain parser output). The controller snaps to the
// returned height each frame. When no ground is found, the character falls at a
// fixed gravity until ground is found again; it never falls indefinitely below
// the field's minimum Y because the query is bounded by the field's XZ extent
// and the caller clamps the position to the field bounding box first.

#include "math/Vector3.h"

#include <cstddef>
#include <functional>
#include <optional>

namespace Modern::Client::Gameplay
{

	// One character's movement state. Renderer-independent: only Vector3 and
	// plain numbers, no graphics API, no platform type.
	struct ClientCharacterController
	{
		// World-space position. Defaults to origin; callers set the spawn point
		// from the field bounding box before the first update.
		Vector3 position = Vector3::Zero;

		// Movement speed in world units per second.
		float moveSpeed = 250.0f;

		// Gravity in world units per second squared, applied when no ground is
		// found beneath the character.
		float gravity = 1200.0f;

		// Maximum fall speed, world units per second (negative).
		float maxFallSpeed = -600.0f;

		// How far above the ground the character's feet sit when snapped. Zero
		// means feet exactly on the triangle.
		float groundOffset = 8.0f;

		// Current vertical velocity (positive = up). Zero when standing on
		// ground; negative when falling.
		float verticalVelocity = 0.0f;

		// True when the controller snapped to ground this frame.
		bool onGround = false;

		// Desired movement direction in XZ, already normalised. Callers set this
		// from input each frame; the controller applies it during Update.
		Vector3 moveDirection = Vector3::Zero;

		// Advances the character by `dt` seconds.
		//
		// `groundQuery(x, z)` returns terrain height at (x, z) or nullopt when
		// there is no ground. `fieldMin` / `fieldMax` clamp the XZ position to
		// the field's valid area so the character cannot walk off the edge.
		void Update(float dt,
		            std::function<std::optional<float>(float, float)> groundQuery,
		            const Vector3& fieldMin,
		            const Vector3& fieldMax) noexcept;

		// Sets the desired movement direction from input. The vector is
		// normalised internally; zero means "no input".
		void SetMoveDirection(Vector3 direction) noexcept;
	};

} // namespace Modern::Client::Gameplay
