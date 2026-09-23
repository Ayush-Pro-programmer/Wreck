#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>

#include "utils/Math.h"

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

struct Vertex
{
	Vector3 position = {};
	Vector3 normal = {};
	float u, v;

	// to check if two vertices are identical
	bool operator==(const Vertex& other) const
	{
		return
			position.x == other.position.x &&
			position.y == other.position.y &&
			position.z == other.position.z &&
			normal.z == other.normal.x &&
			normal.y == other.normal.y &&
			normal.z == other.normal.z &&
			u == other.u &&
			v == other.v;
	}
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
	size_t totalFaces = 0;
	size_t totalBrushes = 0;
};

bool ParsePlaneLine(const std::string& line, PlaneData& outPlane)
{
	char texBuffer[256] = { 0 };
	int matched = sscanf(line.c_str(),
		"( %f %f %f ) ( %f %f %f ) ( %f %f %f ) %256s [ %f %f %f %f ] [ %f %f %f %f ] %f %f %f",
		&outPlane.p1.x , &outPlane.p1.y, &outPlane.p1.z,
		&outPlane.p2.x , &outPlane.p2.y, &outPlane.p2.z,
		&outPlane.p3.x , &outPlane.p3.y, &outPlane.p3.z,
		texBuffer,
		&outPlane.uAxis.x, &outPlane.uAxis.y, &outPlane.uAxis.z, &outPlane.uShift,
		&outPlane.vAxis.x, &outPlane.vAxis.y, &outPlane.vAxis.z, &outPlane.vShift,
		&outPlane.rotation, &outPlane.scaleX, &outPlane.scaleY
	);

	if (matched >= 19)
	{
		outPlane.textureName = std::string(texBuffer);
		outPlane.mathPlane = Plane::FromPoints(outPlane.p1, outPlane.p2, outPlane.p3);

		return true;
	}

	return false;
}

bool ParseMapFile(const std::string& inputPath, std::vector<BrushData>& outBrushes)
{
	std::cout << "Parsing map file: " << inputPath << "\n";
	std::ifstream file(inputPath);
	if (!file.is_open())
	{
		std::cerr << "Error: Couldn't open input map file!\n";
		return false;
	}

	BrushData currentBrush;
	std::string line;
	bool insideBrush = false;

	while (std::getline(file, line))
	{
		// Trim empty lines / comments
		if (line.empty() || line[0] == '/') continue;

		if (line.find('{') != std::string::npos)
		{
			// Check if this brace opens a brush (opposes entity block)
			// In .map files, enity blocks have key-values, brushes have planes.
			insideBrush = true;
			currentBrush.planes.clear();
			continue;
		}

		if (line.find('}') != std::string::npos)
		{
			if (insideBrush && !currentBrush.planes.empty())
				outBrushes.push_back(currentBrush);

			insideBrush = false;
			continue;
		}

		if (insideBrush)
		{
			PlaneData plane;
			if (ParsePlaneLine(line, plane))
				currentBrush.planes.push_back(plane);
		}
	}

	return true;
}

// clips a polygon against a plane, returning the resultting clipped polygon 
std::vector<Vector3> ClipPolygonToPlane(
	const std::vector<Vector3>& poly,
	const Plane& plane, float epsilon = 0.01f)
{
	std::vector<Vector3> outputList;
	if (poly.empty()) return outputList;

	auto ClassifyPoint = [&](const Vector3& p) {
		float dist = Vector3::Dot(plane.normal, p) - plane.dist;
		if (dist >  epsilon) return 1;  // in front
		if (dist < -epsilon) return -1; // behind
		return 0; // on plane
	};

	Vector3 s = poly.back();
	int sSide = ClassifyPoint(s);

	for (const Vector3& p : poly)
	{
		int pSide = ClassifyPoint(p);

		// point is in front or on plane
		if (pSide >= 0)
		{
			// previous point was behind
			if (sSide < 0)
			{
				Vector3 dir = p - s;
				float denom = Vector3::Dot(plane.normal, dir);
				if (fabsf(denom) > 0.00001f)
				{
					float t = (plane.dist - Vector3::Dot(plane.normal, s)) / denom;
					outputList.push_back(s + dir * t);
				}
			}
			outputList.push_back(p);
		}
		// previous was in front, current is behind
		else if (sSide >= 0)
		{
			Vector3 dir = p - s;
			float denom = Vector3::Dot(plane.normal, dir);
			if (fabsf(denom) > 0.00001f)
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

WorldMap CompileMap(const std::vector<BrushData>& brushes)
{
	WorldMap worldMap;
	worldMap.totalBrushes = brushes.size();

	std::unordered_map<std::string, Mesh> globalTexturedMeshes;
	for (const auto& brush : brushes)
	{
		for (size_t i = 0; i < brush.planes.size(); i++)
		{
			// start with a massive polygon for this plane
			std::vector<Vector3> poly = brush.planes[i].mathPlane.CreateLargePolygon();

			// clips this polygon against all OTHER planes ins the same brush
			for (size_t j = 0; j < brush.planes.size(); j++)
			{
				if (i == j) continue;
				poly = ClipPolygonToPlane(poly, brush.planes[j].mathPlane);
				if (poly.empty()) break;
			}

			// if any valid polygon remains, it's a face for our brush!
			if (!poly.empty())
			{
				worldMap.totalFaces++;

				const auto& planeData = brush.planes[i];
				std::string texName = planeData.textureName;

				if (globalTexturedMeshes.find(texName) == globalTexturedMeshes.end())
					globalTexturedMeshes[texName].textureName = texName;

				Mesh& mesh = globalTexturedMeshes[texName];

				// standard retro texture assumptions (64x64) 
				// - need actual texture width and height to compasate high res textures
				float texWidth = 64.0f;
				float texHeight = 64.0f;

				auto FindOrAddVertex = [&](const Vertex& vert) -> unsigned short {
					for (size_t i = 0; i < mesh.vertices.size(); i++)
					{
						if (mesh.vertices[i] == vert)
							return static_cast<unsigned short>(i);
					}

					mesh.vertices.push_back(vert);
					return static_cast<unsigned short>(mesh.vertices.size() - 1);
				};

				// copy position into our final vertex format
				Vector3 faceNormal = planeData.mathPlane.normal;
				std::vector<unsigned short> polyIndices;
				for (const auto& pos : poly)
				{
					float uCoord = (Vector3::Dot(pos, planeData.uAxis) + planeData.uShift) / (texWidth * planeData.scaleX);
					float vCoord = (Vector3::Dot(pos, planeData.vAxis) + planeData.vShift) / (texHeight * planeData.scaleY);

					Vertex v{ pos, faceNormal, uCoord, vCoord };
					unsigned short idx = FindOrAddVertex(v);
					polyIndices.push_back(idx);
				}

				// triangle fan geneation:
				// anchor is vertex 0, paired with i and i + 1
				for (size_t i = 1; i < polyIndices.size() - 1; i++)
				{
					mesh.indices.push_back(polyIndices[0]);		// anchor
					mesh.indices.push_back(polyIndices[i]);	    // current
					mesh.indices.push_back(polyIndices[i + 1]); // next
				}
			}
		}
	}

	for (const auto& [tex, mesh] : globalTexturedMeshes)
		worldMap.meshes.push_back(mesh);

	return worldMap;
}

bool SerializedCompiledMap(const std::string& outputPath, const WorldMap& worldMap)
{
	std::ofstream file(outputPath, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Error: Couldn't open output file for serialization: " << outputPath << "\n";
		return false;
	}

	// magic header to identify the file format
	file.write("WDF1", 4);

	// write total brushes
	uint32_t brushCount = static_cast<uint32_t>(worldMap.totalBrushes);
	file.write(reinterpret_cast<const char*>(&brushCount), sizeof(brushCount));

	uint32_t meshCount = static_cast<uint32_t>(worldMap.meshes.size());
	file.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));

	for (const auto& mesh : worldMap.meshes)
	{
		// write texture name
		uint32_t texNameLen = static_cast<uint32_t>(mesh.textureName.size());
		file.write(reinterpret_cast<const char*>(&texNameLen), sizeof(texNameLen));
		file.write(mesh.textureName.data(), texNameLen);

		// write vertices 
		uint32_t vertexCount = static_cast<uint32_t>(mesh.vertices.size());
		file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
		file.write(reinterpret_cast<const char*>(mesh.vertices.data()), vertexCount * sizeof(Vertex));

		// write indices 
		uint32_t indexCount = static_cast<uint32_t>(mesh.indices.size());
		file.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
		file.write(reinterpret_cast<const char*>(mesh.indices.data()), indexCount * sizeof(unsigned short));
	}

	std::cout << "Successfully serialized map to: " << outputPath << "\n";
	return true;
}

void PrintCompilationStatus(const WorldMap& worldMap)
{
	size_t totalTriangles = 0;
	size_t totalVertices = 0;

	for (const auto& meshes : worldMap.meshes)
	{
		totalVertices += meshes.vertices.size();
		totalTriangles += meshes.indices.size() / 3;
	}

	std::cout << "\n-:COMPILATION STATUS:-\n";
	std::cout << " - Total Brushes : " << worldMap.totalBrushes << "\n";
	std::cout << " - Total Meshes : " << worldMap.meshes.size() << "\n";
	std::cout << " - Total Faces : " << worldMap.totalFaces << "\n";
	std::cout << " - Total Vertices : " << totalVertices << "\n";
	std::cout << " - Total Triangles : " << totalTriangles << "\n\n";
}

// --- MAIN ENTRY ---
int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: Wreck-Compiler.exe <input.map> <output.wmf> \n";
		return 1;
	}

	std::string inputPath = argv[1];
	std::string outputPath = argv[2];

	std::vector<BrushData> brushes;
	if (!ParseMapFile(inputPath, brushes))
		return 1;

	WorldMap worldMap = CompileMap(brushes);
	PrintCompilationStatus(worldMap);

	if (!SerializedCompiledMap(outputPath, worldMap))
		return 1;

	return 0;
}

/**TODO:
* Next step -> geometry generation (cliping planes into polygons) - done
* Serializing into binary .wmf format - done
* Optimize code to reduce irrevelant vertex
*/
