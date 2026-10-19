#pragma once

#include <string>
#include <vector>
#include "utils/Math.h"

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
		poly[1] = center + (u * size) + (v * -size);
		poly[2] = center + (u * size) + (v * size);
		poly[3] = center + (u * -size) + (v * size);

		return poly;
	}

	// check if another plane is coplanar and facing the opposite direction
	bool IsSharedBoundary(const Plane& other, float normalTol = 0.001f, float disTol = 0.01f) const 
	{
		bool oppositeNormals =
			fabsf(normal.x + other.normal.x) < normalTol &&
			fabsf(normal.y + other.normal.y) < normalTol &&
			fabsf(normal.z + other.normal.z) < normalTol;

		bool matchingDist = fabsf(dist + other.dist) < disTol;

		return oppositeNormals && matchingDist;
	}
};

struct PlaneData
{
	Vector3 p1 = {};
	Vector3 p2 = {};
	Vector3 p3 = {};
	Plane mathPlane = {};

	std::string textureName = "";

	Vector3 uAxis = { 1, 0, 0 };
	float uShift = 0.0f;
	Vector3 vAxis = { 0, 1, 0 };
	float vShift = 0.0f;
	float rotation = 0.0f;
	float scaleX = 1.0f;
	float scaleY = 1.0f;
};

struct BrushData
{
	std::vector<PlaneData> planes;
};