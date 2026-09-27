#pragma once

#include <cmath>

namespace Modern
{
	struct Vector3
	{
		float x, y, z;

		Vector3() = default;
		constexpr Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

		Vector3 operator+(const Vector3& o) const { return {x + o.x, y + o.y, z + o.z}; }
		Vector3 operator-(const Vector3& o) const { return {x - o.x, y - o.y, z - o.z}; }
		Vector3 operator-() const { return {-x, -y, -z}; }
		Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }
		Vector3 operator/(float s) const { return {x / s, y / s, z / s}; }

		Vector3& operator+=(const Vector3& o) { x += o.x; y += o.y; z += o.z; return *this; }
		Vector3& operator-=(const Vector3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
		Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
		Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

		float LengthSq() const { return x * x + y * y + z * z; }
		float Length() const { return std::sqrt(LengthSq()); }
		float DistanceSq(const Vector3& o) const { return (*this - o).LengthSq(); }
		float Distance(const Vector3& o) const { return (*this - o).Length(); }

		bool IsZero() const { return LengthSq() == 0.0f; }

		static const Vector3 Zero;
		static const Vector3 Up;
		static const Vector3 Forward;
		static const Vector3 Right;
		static const Vector3 UnitX;
		static const Vector3 UnitY;
		static const Vector3 UnitZ;

		static Vector3 FromArray(const float v[3]) { return {v[0], v[1], v[2]}; }
	};

	inline const Vector3 Vector3::Zero    {0.0f, 0.0f, 0.0f};
	inline const Vector3 Vector3::Up      {0.0f, 1.0f, 0.0f};
	inline const Vector3 Vector3::Forward {0.0f, 0.0f, 1.0f};
	inline const Vector3 Vector3::Right   {1.0f, 0.0f, 0.0f};
	inline const Vector3 Vector3::UnitX   {1.0f, 0.0f, 0.0f};
	inline const Vector3 Vector3::UnitY   {0.0f, 1.0f, 0.0f};
	inline const Vector3 Vector3::UnitZ   {0.0f, 0.0f, 1.0f};

	inline Vector3 Normalize(const Vector3& v)
	{
		const float len = v.Length();
		if (len <= 0.0f) return Vector3::Zero;
		return v / len;
	}
}
