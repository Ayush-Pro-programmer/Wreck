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

	// check if two 3D polygons overlap
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

		auto getBounds2D = [&](const std::vector<Vector3>& poly, float& minX, float& maxX, float& minY, float& maxY)
			{
				minX = minY = 1e30f;
				maxX = maxY = -1e30f;

				for (const auto& p : poly)
				{
					float px = Vector3::Dot(p, u);
					float py = Vector3::Dot(p, v);
					minX = fminf(minX, px); maxX = fmaxf(maxX, px);
					minY = fminf(minY, py); maxY = fmaxf(maxY, py);
				}
			};

		float minAX, maxAX, minAY, maxAY; // outer 
		float minBX, maxBX, minBY, maxBY; // inner
		getBounds2D(outerPoly, minAX, maxAX, minAY, maxAY);
		getBounds2D(innerPoly, minBX, maxBX, minBY, maxBY);

		// check if B's bound fit completely inside A's bound
		bool insideX = (minBX >= minAX - tol) && (maxBX <= maxAX + tol);
		bool insideY = (minBY >= minAY - tol) && (maxBY <= maxAY + tol);

		return insideX && insideY;
	}

	struct GeneratedFace
	{
		Plane plane;
		std::vector<Vector3> poly;
		PlaneData planeData;
		size_t brushIndex;
	};

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
		std::vector<bool>& skipFace)
	{
		for (size_t i = 0; i < faces.size(); i++)
		{
			if (skipFace[i]) continue;

			for (size_t j = i + 1; j < faces.size(); j++)
			{
				if (skipFace[j]) continue;

				if (faces[i].brushIndex == faces[j].brushIndex) continue;

				if (faces[i].plane.IsSharedBoundary(faces[j].plane))
				{
					bool iInsideJ = DoPolygonsOverlap(faces[j].poly, faces[i].poly, faces[i].plane);
					bool jInsideI = DoPolygonsOverlap(faces[i].poly, faces[j].poly, faces[i].plane);

					// check if Face J is complete match of Face I
					if (iInsideJ && jInsideI)
					{
						skipFace[i] = true;
						skipFace[j] = true;
						break;
					}

					// check if Face J is completely inside Face I
					if (iInsideJ)
					{
						skipFace[j] = true;
						break;
					}
					// check if Face I is completely inside Face J
					else if (jInsideI)
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
				float uCoord = (Vector3::Dot(pos, planeData.uAxis) + planeData.uShift) / (texWidth * planeData.scaleX);
				float vCoord = (Vector3::Dot(pos, planeData.vAxis) + planeData.vShift) / (texWidth * planeData.scaleY);

				mesh.vertices.push_back({ pos, faceNormal, uCoord, vCoord });
				triVerts.push_back(pos);
			}

			// triangulte the face polygon for rendering
			for (size_t k = 1; k < face.poly.size() - 1; k++)
			{
				mesh.indices.push_back(startIndex);
				mesh.indices.push_back(startIndex + static_cast<unsigned short>(k));
				mesh.indices.push_back(startIndex + static_cast<unsigned short>(k + 1));
			}

			// triangulate for collision triangles
			for (size_t k = 1; k < triVerts.size() - 1; k++)
			{
				worldMap.brushTriangles[face.brushIndex].push_back({
					triVerts[0],
					triVerts[k],
					triVerts[k + 1]
				});
			}
		}

		// flaten the hash map into final world map mesh list
		for (const auto& [tex, mesh] : textureMeshes)
			worldMap.meshes.push_back(mesh);
	}

	inline WorldMap Compile(const std::vector<BrushData>& brushes)
	{
		WorldMap worldMap;
		worldMap.totalBrushes = brushes.size();
		worldMap.brushTriangles.resize(brushes.size());

		auto faces = GenerateFaces(brushes);

		std::vector<bool> skipFace(faces.size(), false);
		CullHiddenFaces(faces, skipFace);

		BuildMeshesAndCollision(faces, skipFace, worldMap);

		return worldMap;
	}
}