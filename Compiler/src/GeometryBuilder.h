#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#include "Brush.h"
#include "utils/Math.h"

struct Vertex
{
	Vector3 position = {};
	Vector3 normal = {};
	float u = 0.0f;
	float v = 0.0f;
};

struct Triangle
{
	Vector3 v0, v1, v2;
};

struct Mesh
{
	std::string textureName;
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;
};

struct WorldMap
{
	std::vector<Mesh> meshes;
	std::vector<std::vector<Triangle>> brushTriangles;
	size_t totalFaces = 0;
	size_t totalBrushes = 0;
};

namespace GeometryBuilder
{

	struct GeneratedFace
	{
		Plane plane;
		std::vector<Vector3> poly;
		PlaneData planeData;
		size_t brushIndex;
	};

	inline Vector3 ConvertToRaylib(const Vector3& v)
	{
		return { -v.x, v.z, v.y };
	}

	// clips a polygon against a plane, returning the resulting clipped polygon
	inline std::vector<Vector3> ClipPolygonToPlane(
		const std::vector<Vector3>& poly,
		const Plane& plane, float epsilon = 0.01f )
	{
		std::vector<Vector3> outputList;
		if (poly.empty()) return outputList;

		auto ClassifyPoint = [&](const Vector3& p) {
			float dist = Vector3::Dot(plane.normal, p) - plane.dist;
			if (dist > epsilon) return 1;	// in front
			if (dist < -epsilon) return -1; // behind
			return 0;						// on plane
		};

		Vector3 s = poly.back();
		int sSide = ClassifyPoint(s);

		for (const Vector3& p : poly)
		{
			int pSide = ClassifyPoint(p);

			if (pSide >= 0) 
			{
				if (sSide < 0)
				{
					Vector3 dir = p - s;
					float denom = Vector3::Dot(plane.normal, dir);
					if (fabsf(denom) > 0.00001)
					{
						float t = (plane.dist - Vector3::Dot(plane.normal, s)) / denom;
						outputList.push_back(s + dir * t);
					}
				}
				outputList.push_back(p);
			}
			else if (sSide >= 0)
			{
				Vector3 dir = p - s;
				float denom = Vector3::Dot(plane.normal, dir);
				if (fabsf(denom) > 0.00001)
				{
					float t = (plane.dist - Vector3::Dot(plane.normal, s)) / denom;
					outputList.push_back(s + dir * t);
				}
			}

			s = p;
			sSide = pSide;
		}

		return outputList;
	}

	// check if two 3D polygons overlap using 2D half-plane tests
	inline bool DoPolygonsOverlap(
		const std::vector<Vector3>& innerPoly ,
		const std::vector<Vector3>& outerPoly,
		const Plane& plane, float tol = 0.05f)
	{
		if (innerPoly.empty() || outerPoly.empty()) return false;

		// build a local 2D basis (tangent vectors u and v) on the plane
		Vector3 majorAxis = (fabsf(plane.normal.x) > 0.9f) ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
		Vector3 u = Vector3::Normalize(Vector3::Cross(plane.normal, majorAxis));
		Vector3 v = Vector3::Normalize(Vector3::Cross(plane.normal, u));

		// projects 3D polygon to 2D
		auto to2D = [&](const std::vector<Vector3>& poly)
			{
				std::vector<Vector2> res;
				res.reserve(poly.size());
				for (const auto& p : poly)
					res.push_back({ Vector3::Dot(p, u), Vector3::Dot(p, v) });

				return res;
			};

		std::vector<Vector2> outer2D = to2D(outerPoly);
		std::vector<Vector2> inner2D = to2D(innerPoly);

		auto IsPointInConvexPolygon = [](const Vector2& pt, const std::vector<Vector2>& poly, float tolerance)
			{
				bool hasPositive = false;
				bool hasNegative = false;
				size_t n = poly.size();

				for (size_t i = 0; i < n; i++)
				{
					const Vector2& a = poly[i];
					const Vector2& b = poly[(i + 1) % n];

					float cross = (b.x - a.x) * (pt.y - a.y) - (b.y - a.y) * (pt.x - a.x);

					if (cross > tolerance) hasPositive = true;
					if (cross < -tolerance) hasNegative = true;

					if (hasPositive && hasNegative) return false;
				}

				return true;
			};

		// check if ALL vertices of innerPoly are inside outerPoly
		for (const auto& pt : inner2D)
		{
			if (!IsPointInConvexPolygon(pt, outer2D, tol))
				return false;
		}

		return true;
	}

	// checks if an entire brush plane is completely buried inside a brush
	inline bool IsFaceInsideBrush(
		const GeneratedFace& face,
		const BrushData& brush, float tol = 0.05)
	{
		if (face.poly.empty()) return false;

		auto IsPointInsideBrush = [](const Vector3& p, const BrushData& brush, float tol)
			{
				for (const auto& planeData : brush.planes)
				{
					Plane pEq = Plane::FromPoints(planeData.p1, planeData.p2, planeData.p3);
					float dist = Vector3::Dot(pEq.normal, p) - pEq.dist;

					if (dist > tol) return false;
				}

				return true;
			};

		for (const auto& v : face.poly)
		{
			if (!IsPointInsideBrush(v, brush, tol))
				return false;
		}

		return true;
	}


	inline std::vector<GeneratedFace>
		GenerateFaces(const std::vector<BrushData>& brushes)
	{
		std::vector<GeneratedFace> faces;

		for (size_t bIdx = 0; bIdx < brushes.size(); bIdx++)
		{
			const auto& brush = brushes[bIdx];
			for (size_t i = 0; i < brush.planes.size(); i++)
			{
				Plane mathPlane = Plane::FromPoints(
					brush.planes[i].p1,
					brush.planes[i].p2,
					brush.planes[i].p3
				);
				std::vector<Vector3> poly = mathPlane.CreateLargePolygon();

				for (size_t j = 0; j < brush.planes.size(); j++)
				{
					if (i == j) continue;
					Plane otherPlane = Plane::FromPoints(
						brush.planes[j].p1,
						brush.planes[j].p2,
						brush.planes[j].p3
					);
					poly = ClipPolygonToPlane(poly, otherPlane);
					if (poly.empty()) break;
				}

				if (!poly.empty())
				{
					faces.push_back({ 
						mathPlane, poly,
						brush.planes[i], bIdx 
					});
				}
			}
		}

		return faces;
	}

	inline void CullHiddenFaces(
		const std::vector<GeneratedFace>& faces,
		const std::vector<BrushData>& brushes,
		std::vector<bool>& skipFace)
	{
		for (size_t i = 0; i < faces.size(); i++)
		{
			if (skipFace[i]) continue;

			for (size_t bIdx = 0; bIdx < brushes.size(); bIdx++)
			{
				if (faces[i].brushIndex == bIdx) continue;

				if (IsFaceInsideBrush(faces[i], brushes[bIdx]))
				{
					skipFace[i] = true;
					break;
				}
			}

			if (skipFace[i]) continue;

			for (size_t j = i + 1; j < faces.size(); j++)
			{
				if (skipFace[j]) continue;

				if (faces[i].brushIndex == faces[j].brushIndex) continue;

				if (faces[i].plane.IsSharedBoundary(faces[j].plane))
				{
					bool jInsideI = DoPolygonsOverlap(faces[j].poly, faces[i].poly, faces[i].plane);
					bool iInsideJ = DoPolygonsOverlap(faces[i].poly, faces[j].poly, faces[i].plane);

					// check if Face J is complete match of Face I
					if (jInsideI && iInsideJ)
					{
						skipFace[i] = true;
						skipFace[j] = true;
						break;
					}

					// check if Face J is completely inside Face I
					if (jInsideI)
					{
						skipFace[j] = true;
					}
					// check if Face I is completely inside Face J
					else if (iInsideJ)
					{
						skipFace[i] = true;
						break;
					}
				}
			}
		}
	}

	inline void BuildMeshesAndCollision(
		const std::vector<GeneratedFace>& faces,
		const std::vector<bool>& skipFace,
		WorldMap& worldMap)
	{
		// groups meshes by texture name
		std::unordered_map<std::string, Mesh> textureMeshes;

		// standard retro texture assumption (64x64)
		float texWidth = 64;
		float texHeight = 64;

		for (size_t i = 0; i < faces.size(); i++)
		{
			if (skipFace[i]) continue;

			worldMap.totalFaces++;

			const auto& face = faces[i];
			const auto& planeData = face.planeData;
			std::string texName = planeData.textureName;

			if (textureMeshes.find(texName) == textureMeshes.end())
				textureMeshes[texName].textureName = texName;

			Mesh& mesh = textureMeshes[texName];
			Vector3 faceNormal = planeData.mathPlane.normal;
			unsigned short startIndex = static_cast<unsigned short>(mesh.vertices.size());

			std::vector<Vector3> triVerts;

			for (const auto& pos : face.poly)
			{
				Vector3 raylibPos = ConvertToRaylib(pos);
				Vector3 raylibNormal = ConvertToRaylib(faceNormal);

				float uCoord = (Vector3::Dot(pos, planeData.uAxis) + planeData.uShift) / (texWidth * planeData.scaleX);
				float vCoord = (Vector3::Dot(pos, planeData.vAxis) + planeData.vShift) / (texWidth * planeData.scaleY);

				mesh.vertices.push_back({ raylibPos, raylibNormal, uCoord, vCoord });
				triVerts.push_back(raylibPos);
			}

			// triangulte the face polygon for rendering
			for (size_t k = 1; k < face.poly.size() - 1; k++)
			{
				mesh.indices.push_back(startIndex);
				mesh.indices.push_back(startIndex + static_cast<unsigned short>(k + 1));
				mesh.indices.push_back(startIndex + static_cast<unsigned short>(k));
			}

			// triangulate for collision triangles
			for (size_t k = 1; k < triVerts.size() - 1; k++)
			{
				worldMap.brushTriangles[face.brushIndex].push_back({
					triVerts[0],
					triVerts[k + 1],
					triVerts[k]
				});
			}
		}

		// flaten the hash map into final world map mesh list
		for (const auto& [tex, mesh] : textureMeshes)
			worldMap.meshes.push_back(mesh);
	}

	inline WorldMap Compile(const std::vector<BrushData>& brushes, bool enableCulling = true)
	{
		WorldMap worldMap;
		worldMap.totalBrushes = brushes.size();
		worldMap.brushTriangles.resize(brushes.size());

		auto faces = GenerateFaces(brushes);

		std::vector<bool> skipFace(faces.size(), false);

		if (enableCulling)
			CullHiddenFaces(faces, brushes, skipFace);

		BuildMeshesAndCollision(faces, skipFace, worldMap);

		return worldMap;
	}
}