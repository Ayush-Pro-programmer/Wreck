#pragma once

#include <cmath>
#include <vector>

struct Vector3
{
	float x, y, z;

	// Basic operatoors
	Vector3 operator+(const Vector3& o) const { return { x + o.x, y + o.y, z + o.z }; }
	Vector3 operator-(const Vector3& o) const { return { x - o.x, y - o.y, z - o.z }; }
	Vector3 operator*(float s) const { return { x * s, y * s, z * s }; }
	Vector3 operator/(float s) const { return { x / s, y / s, z / s }; }

	Vector3& operator+=(const Vector3& o) { x += o.x; y += o.y; z += o.z; return *this; }
	Vector3& operator-=(const Vector3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }

	// ===== Vector maths =====

	// returns scalar product of vector
	static float Dot(const Vector3& a, const Vector3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	// returns vector product of vectors
	static Vector3 Cross(const Vector3& a, const Vector3& b)
	{
		return
		{
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		};
	}

	// returns length of the vector
	float Length() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	// returns normalized copy of the vector
	Vector3 Normalized() const
	{
		float len = Length();
		if (len == 0.0f) return { 0.0f, 0.0f, 0.0f };
		return *this / len;
	}

	static Vector3 Normalize(const Vector3& v)
	{
		return v.Normalized();
	}
};