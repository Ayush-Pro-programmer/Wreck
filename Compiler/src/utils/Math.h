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

struct Plane
{
	Vector3 normal = {};
	float dist = 0.0f;

	// Construct a plane from 3 points
	static Plane FromPoints(const Vector3& p1, const Vector3& p2, const Vector3& p3)
	{
		Vector3 edge1 = p2 - p1;
		Vector3 edge2 = p3 - p1;
		Vector3 normal = Vector3::Normalize(Vector3::Cross(edge1, edge2));
		float dist = Vector3::Dot(normal, p1);
		return { normal, dist };
	}

	std::vector<Vector3> CreateLargePolygon(float size = 16384.0f) const
	{
		// find two orthogonal vectors tangent to the plane normal
		Vector3 majorAxis;
		if (fabsf(normal.x) > 0.9f)
			majorAxis = Vector3(0.0f, 1.0f, 0.0f);
		else
			majorAxis = Vector3(1.0f, 0.0f, 0.0f);

		Vector3 u = Vector3::Normalize(Vector3::Cross(normal, majorAxis));
		Vector3 v = Vector3::Normalize(Vector3::Cross(normal, u));

		// center point of the giant polygon on the plane
		Vector3 center = normal * dist;

		// 4 corners of the giant quad
		std::vector<Vector3> poly(4);
		poly[0] = center + (u * -size) + (v * -size);
		poly[1] = center + (u *  size) + (v * -size);
		poly[2] = center + (u *  size) + (v *  size);
		poly[3] = center + (u * -size) + (v *  size);

		return poly;
	}
};