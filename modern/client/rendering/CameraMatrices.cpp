#include "rendering/CameraMatrices.h"

#include <algorithm>
#include <cmath>

namespace Modern::Client
{
	namespace
	{
		void Zero(float* out) noexcept
		{
			for (int i = 0; i < 16; ++i) { out[i] = 0.0f; }
		}
	} // namespace

	float TanHalfFovY(float fovYRadians) noexcept
	{
		if (!(fovYRadians > 0.0f) || !(fovYRadians < 3.14159265f)) { return 0.0f; }
		return std::tan(fovYRadians * 0.5f);
	}

	void BuildLookAtRowMajor(const Vector3& eye, const Vector3& target, const Vector3& up, float* out)
	{
		if (out == nullptr) { return; }

		// A degenerate view direction or a `up` parallel to it leaves the basis
		// undefined. Fall back to a straight-ahead view rather than dividing by
		// zero and filling the matrix with NaNs.
		Vector3 forward = target - eye;
		if (forward.IsZero())
		{
			forward = Vector3::Forward;
		}
		const Vector3 zaxis = Normalize(forward);

		Vector3 xaxis = Cross(up, zaxis);
		if (xaxis.IsZero())
		{
			// `up` is parallel to the view direction; pick any perpendicular.
			xaxis = Cross(Vector3::Right, zaxis);
			if (xaxis.IsZero()) { xaxis = Vector3::UnitX; }
		}
		xaxis = Normalize(xaxis);
		const Vector3 yaxis = Cross(zaxis, xaxis);

		out[0]  = xaxis.x; out[1]  = yaxis.x; out[2]  = zaxis.x; out[3]  = 0.0f;
		out[4]  = xaxis.y; out[5]  = yaxis.y; out[6]  = zaxis.y; out[7]  = 0.0f;
		out[8]  = xaxis.z; out[9]  = yaxis.z; out[10] = zaxis.z; out[11] = 0.0f;
		out[12] = -Dot(xaxis, eye);
		out[13] = -Dot(yaxis, eye);
		out[14] = -Dot(zaxis, eye);
		out[15] = 1.0f;
	}

	void BuildPerspectiveRowMajor(float fovYRadians, float aspect, float nearPlane, float farPlane, float* out)
	{
		if (out == nullptr) { return; }

		const float tanHalf = TanHalfFovY(fovYRadians);
		const bool  usable   = tanHalf > 0.0f &&
		                       aspect > 0.0f &&
		                       nearPlane > 0.0f &&
		                       farPlane > nearPlane;
		if (!usable)
		{
			Zero(out);
			return;
		}

		const float cotHalfFov = 1.0f / tanHalf;
		const float w = cotHalfFov / aspect;
		const float h = cotHalfFov;
		const float q = farPlane / (farPlane - nearPlane);

		out[0]  = w;      out[1]  = 0.0f; out[2]  = 0.0f; out[3]  = 0.0f;
		out[4]  = 0.0f;   out[5]  = h;      out[6]  = 0.0f; out[7]  = 0.0f;
		out[8]  = 0.0f;   out[9]  = 0.0f; out[10] = q;      out[11] = 1.0f;
		out[12] = 0.0f;   out[13] = 0.0f;
		out[14] = -nearPlane * farPlane / (farPlane - nearPlane);
		out[15] = 0.0f;
	}

	void BuildIdentityRowMajor(float* out)
	{
		if (out == nullptr) { return; }
		Zero(out);
		out[0]  = 1.0f;
		out[5]  = 1.0f;
		out[10] = 1.0f;
		out[15] = 1.0f;
	}

	SceneCamera FrameBox(const Vector3& boxMin, const Vector3& boxMax, const Vector3& viewDirection)
	{
		SceneCamera camera;

		const Vector3 centre = (boxMin + boxMax) * 0.5f;
		const Vector3 extent = boxMax - boxMin;

		// Bounding-sphere radius, so "does the whole thing fit" does not depend on
		// which axis happens to be longest.
		const float radius = std::max(0.001f, extent.Length() * 0.5f);

		camera.fovYRadians = 1.0471975512f;  // 60 degrees
		const float tanHalf = TanHalfFovY(camera.fovYRadians);

		Vector3 direction = viewDirection;
		if (Normalize(direction).IsZero()) { direction = Vector3::Forward; }
		direction = Normalize(direction);

		// Pull back far enough that a sphere of `radius` fills the vertical field of
		// view, with a little margin so the edge of the field is not exactly the
		// edge of the terrain.
		float distance = radius / std::max(0.05f, tanHalf);
		distance = std::max(distance * 1.25f, radius + 1.0f);

		camera.eye = centre + direction * distance;
		camera.target = centre;

		// Keep "up" perpendicular to the view direction, otherwise the look-at basis
		// degenerates for a camera looking straight down.
		const Vector3 up = Cross(direction, Cross(Vector3::Up, direction));
		camera.up = up.IsZero() ? Vector3::Forward : Normalize(up);

		// Push both planes outside the bounding sphere so nothing clips.
		camera.nearPlane = std::max(0.01f, distance - radius * 1.5f);
		camera.farPlane  = distance + radius * 2.5f;

		return camera;
	}
} // namespace Modern::Client