#pragma once

// WORLD-PLAYABLE-001B: camera and projection maths for the client.
//
// These live in the rendering *library* but deliberately carry no graphics API
// at all -- only `Vector3` from core and plain row-major float buffers. That is
// what makes them testable without a device (see `ClientRenderingTests`) and
// what keeps a future Vulkan or Metal backend free to reuse them: a camera is a
// set of numbers, not a GPU object.
//
// The matrix convention is fixed by what `D3D11Renderer`'s inline shader already
// expects: `#pragma pack_matrix(row_major)`, row-vector convention
// (`mul(float4(pos,1), world) * view * projection`), and a left-handed view and
// projection. Every current backend therefore has to produce exactly this
// layout, and the tests pin it.

#include "math/Vector3.h"

namespace Modern::Client
{
	// A scene camera in plain numbers: where it is, what it looks at, which way
	// is up, and how the projection is cut. Renderer-independent by construction.
	struct SceneCamera
	{
		Vector3 eye     = Vector3::Zero;
		Vector3 target  = Vector3::Forward;
		Vector3 up      = Vector3::Up;
		float   fovYRadians = 1.0471975512f;  // 60 degrees
		float   nearPlane   = 0.1f;
		float   farPlane    = 1000.0f;
	};

	// Row-major, left-handed look-at view matrix.
	//
	// Writes exactly 16 floats. `out` is not validated beyond being non-null:
	// callers in this client pass fixed-size arrays, and a null here would be a
	// programming error rather than a runtime condition worth branching on.
	void BuildLookAtRowMajor(const Vector3& eye, const Vector3& target, const Vector3& up, float* out);

	// Row-major, left-handed perspective projection.
	//
	// A non-positive aspect, field of view or plane distance yields a zero
	// matrix rather than a divide by zero or a matrix full of infinities; a
	// caller that ignored that would draw nothing, which is easier to diagnose
	// than a crashed process.
	void BuildPerspectiveRowMajor(float fovYRadians, float aspect, float nearPlane, float farPlane, float* out);

	// Identity world matrix (translation + scale only), row-major.
	void BuildIdentityRowMajor(float* out);

	// A camera that frames an axis-aligned box.
	//
	// This is the one piece of "where should the camera start" logic, and it is
	// here rather than in the application so it can be tested without a window.
	// The camera sits back along `viewDirection` from the box centre, far enough
	// that the whole bounding sphere fits the vertical field of view with margin,
	// and looks at the centre. The near/far planes are pushed outside the
	// bounding radius so the box is never clipped.
	SceneCamera FrameBox(const Vector3& boxMin, const Vector3& boxMax, const Vector3& viewDirection);

	// Half the vertical field of view in radians, as the tangent of it. Exposed so
	// `FrameBox` can be checked in tests without re-deriving the formula.
	float TanHalfFovY(float fovYRadians) noexcept;
} // namespace Modern::Client