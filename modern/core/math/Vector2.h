#pragma once

// WORLD-ENTRY-002d: the 2-component vector the navigation kernel needs.

#include <cmath>

//
// ---------------------------------------------------------------------------
// WHY THIS EXISTS AND WHY IT IS NOT A GENERAL MATH LIBRARY
// ---------------------------------------------------------------------------
//
// RAN's navigation code is written against `D3DXVECTOR2`, used in exactly one
// place: `Line2D` (legacy/Lib_Engine/NaviMesh/line2d.h:69-73) holds the two
// endpoints of a 2D line segment, and every navigation query projects a world
// position onto the XZ plane before testing it. That is the whole use.
//
// So this is two floats and the operations `Line2D` performs: addition,
// subtraction, scaling, normalisation, and the dot product it uses as a signed
// distance. Adding a matrix type, a quaternion or a general-purpose vector
// library would be several thousand lines whose every rule would have to be
// understood before it could be trusted - which is the opposite of what a
// movement milestone needs.
//
// ---------------------------------------------------------------------------
// WHY THERE IS NO MATRIX ANYWHERE IN THIS KERNEL
// ---------------------------------------------------------------------------
//
// WORLD-ENTRY-002d §11 asked for this explicitly, and the answer is worth
// recording because it is a measured finding rather than a style choice.
//
// Legacy threads a `D3DXMATRIX& matComb` through the whole AABB-tree builder
// and applies `D3DXVec3TransformCoord` to every vertex it examines
// (legacy/Lib_Engine/NaviMesh/navagationtree.cpp:11, :28, :90, :127). That
// looks like a transform dependency. It is not: `MakeAABBTree` builds the
// matrix with `D3DXMatrixIdentity` at navagationtree.cpp:503 and then only ever
// PASSES IT ON - every one of the 13 matrix mentions in the kernel either
// creates that identity or forwards it.
//
// `D3DXVec3TransformCoord` against the identity is a copy. So the matrix
// parameter has no observable effect on the tree that comes out, it is deleted
// here, and the kernel builds the same tree. This is the whole of the D3DX9
// dependency the navigation path has, and it is zero.
//
// ---------------------------------------------------------------------------
// NAMING
// ---------------------------------------------------------------------------
//
// `Line2D`'s two-dimensional endpoints are (x, z) of the world, not (x, y).
// That mapping is NOT implicit in this type - it is applied at the call sites in
// NavigationCell, exactly where legacy applies it
// (legacy/Lib_Engine/NaviMesh/navigationcell.h:205-207). A `Vector2` that
// silently meant "x and y" would be a trap; this one means "two numbers" and
// the projection is visible where it happens.

namespace Modern
{
	struct Vector2
	{
		float x = 0.0f;
		float y = 0.0f;

		constexpr Vector2() noexcept = default;
		constexpr Vector2(float x_, float y_) noexcept : x(x_), y(y_) {}

		Vector2 operator+(const Vector2& o) const { return {x + o.x, y + o.y}; }
		Vector2 operator-(const Vector2& o) const { return {x - o.x, y - o.y}; }
		Vector2 operator-() const { return {-x, -y}; }
		Vector2 operator*(float s) const { return {x * s, y * s}; }
		Vector2 operator/(float s) const { return {x / s, y / s}; }

		Vector2& operator+=(const Vector2& o) { x += o.x; y += o.y; return *this; }
		Vector2& operator-=(const Vector2& o) { x -= o.x; y -= o.y; return *this; }
		Vector2& operator*=(float s) { x *= s; y *= s; return *this; }

		float LengthSq() const { return x * x + y * y; }
		float Length() const { return std::sqrt(LengthSq()); }
		float Distance(const Vector2& o) const { return (*this - o).Length(); }

		bool IsZero() const { return LengthSq() == 0.0f; }

		bool IsFinite() const { return std::isfinite(x) && std::isfinite(y); }

		float operator[](int i) const { return i == 0 ? x : y; }
		float& operator[](int i) { return i == 0 ? x : y; }

		friend bool operator==(const Vector2& a, const Vector2& b)
		{
			return a.x == b.x && a.y == b.y;
		}

		friend bool operator!=(const Vector2& a, const Vector2& b) { return !(a == b); }
	};

	// The dot product of two 2D vectors.
	//
	// Free function rather than a member because it is symmetric in its
	// arguments and because `Line2D::SignedDistance` (line2d.h:207) and
	// `NavigationCell::ProjectPathOnCellWall` (navigationcell.cpp:121) both use
	// it as a relation between two vectors, not as an operation on one.
	inline float Dot(const Vector2& a, const Vector2& b) noexcept
	{
		return a.x * b.x + a.y * b.y;
	}

	// The unit vector, or (0,0) when the input has no direction.
	//
	// Legacy calls `D3DXVec2Normalize`, which for a zero-length input leaves the
	// destination at whatever it already held. Here a zero-length input yields
	// (0,0), so a caller that forgets to guard cannot read an uninitialised
	// direction - and the only two call sites both operate on a difference of two
	// distinct points.
	inline Vector2 Normalize(const Vector2& v) noexcept
	{
		if (!v.IsFinite())
		{
			return Vector2{};
		}

		const float len = v.Length();
		if (len <= 0.0f)
		{
			return Vector2{};
		}

		return v / len;
	}
}
