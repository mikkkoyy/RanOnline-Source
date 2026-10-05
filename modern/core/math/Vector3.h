#pragma once

#include <cmath>

namespace Modern
{
	// Minimal 3-component vector for world-space positions and directions.
	//
	// This is deliberately not a general linear-algebra type: the core only
	// needs position and unit-direction arithmetic, and every extra operation
	// is another rule that has to be understood before it can be trusted.
	//
	// WORLD-ENTRY-002d added Dot, Cross and operator[] for the RAN navigation
	// kernel and nothing else; see the note on them below.
	struct Vector3
	{
		float x, y, z;

		// Zero-initialised. A defaulted constructor would leave the members
		// indeterminate, which turns any later read into undefined behaviour.
		constexpr Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f) {}
		constexpr Vector3(float x_, float y_, float z_) noexcept : x(x_), y(y_), z(z_) {}

		Vector3 operator+(const Vector3& o) const { return {x + o.x, y + o.y, z + o.z}; }
		Vector3 operator-(const Vector3& o) const { return {x - o.x, y - o.y, z - o.z}; }
		Vector3 operator-() const { return {-x, -y, -z}; }
		Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }
		Vector3 operator/(float s) const { return {x / s, y / s, z / s}; }

		Vector3& operator+=(const Vector3& o) { x += o.x; y += o.y; z += o.z; return *this; }
		Vector3& operator-=(const Vector3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
		Vector3* operator*=(float s) { x *= s; y *= s; z *= s; return this; }
		Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

		float LengthSq() const { return x * x + y * y + z * z; }
		float Length() const { return std::sqrt(LengthSq()); }
		float DistanceSq(const Vector3& o) const { return (*this - o).LengthSq(); }
		float Distance(const Vector3& o) const { return (*this - o).Length(); }

		bool IsZero() const { return LengthSq() == 0.0f; }

		// Guards against NaN and infinity reaching the simulation. Validating
		// at the boundary is what keeps one bad value from poisoning every
		// downstream rule.
		bool IsFinite() const
		{
			return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
		}

		friend bool operator==(const Vector3& a, const Vector3& b)
		{
			return a.x == b.x && a.y == b.y && a.z == b.z;
		}

		friend bool operator!=(const Vector3& a, const Vector3& b)
		{
			return !(a == b);
		}

		static const Vector3 Zero;
		static const Vector3 Up;
		static const Vector3 Forward;
		static const Vector3 Right;
		static const Vector3 UnitX;
		static const Vector3 UnitY;
		static const Vector3 UnitZ;

		static Vector3 FromArray(const float v[3]) { return {v[0], v[1], v[2]}; }

		// ---- WORLD-ENTRY-002d: the navigation kernel's three additions -------
		//
		// `Dot`, `Cross` and `operator[]` exist for one consumer: the RAN
		// navigation kernel, which was written against `D3DXVECTOR3` and uses
		// exactly these three of the D3DX surface
		// (legacy/Lib_Engine/NaviMesh/plane.h:193, :197;
		// legacy/Lib_Engine/DxCommon/Collision.cpp:261-262).
		//
		// They are here rather than in a navigation-local helper because a vector
		// type that cannot take its own dot product is the wrong shape, and
		// duplicating the arithmetic in the kernel would be two answers to the
		// same question. Nothing else in the codebase depends on them yet.
		float operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }
		float& operator[](int i) { return i == 0 ? x : (i == 1 ? y : z); }
	};

	inline float Dot(const Vector3& a, const Vector3& b) noexcept
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	// The right-handed cross product, in the same order as `D3DXVec3Cross`.
	//
	// Order matters and is not arbitrary: RAN builds every navigation plane as
	// `cross(P1 - P0, P2 - P0)` (legacy/Lib_Engine/NaviMesh/plane.h:193), and the
	// resulting normal is what decides whether `Actor::Update` suppresses Y
	// (legacy/Lib_Engine/NaviMesh/actor.cpp:361). Reversing the arguments would
	// flip every normal in the mesh and silently invert which surfaces count as
	// floors.
	inline Vector3 Cross(const Vector3& a, const Vector3& b) noexcept
	{
		return {a.y * b.z - a.z * b.y,
		        a.z * b.x - a.x * b.z,
		        a.x * b.y - a.y * b.x};
	}

	inline const Vector3 Vector3::Zero    {0.0f, 0.0f, 0.0f};
	inline const Vector3 Vector3::Up      {0.0f, 1.0f, 0.0f};
	inline const Vector3 Vector3::Forward {0.0f, 0.0f, 1.0f};
	inline const Vector3 Vector3::Right   {1.0f, 0.0f, 0.0f};
	inline const Vector3 Vector3::UnitX   {1.0f, 0.0f, 0.0f};
	inline const Vector3 Vector3::UnitY   {0.0f, 1.0f, 0.0f};
	inline const Vector3 Vector3::UnitZ   {0.0f, 0.0f, 1.0f};

	// Returns the unit vector, or Zero when the input has no direction. Callers
	// that require a real direction must reject Zero themselves rather than
	// silently propagating it.
	inline Vector3 Normalize(const Vector3& v)
	{
		if (!v.IsFinite())
		{
			return Vector3::Zero;
		}

		const float len = v.Length();
		if (len <= 0.0f)
		{
			return Vector3::Zero;
		}

		return v / len;
	}
}
