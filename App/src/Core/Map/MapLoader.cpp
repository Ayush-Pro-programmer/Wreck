#include <iostream>
#include <fstream>
#include <cstdint>

#include "MapLoader.h"
#include "raymath.h";

namespace Wreck {

	struct CompilerVertexBinary
	{
		Vector3 position;
		Vector3 normal;
		float u, v;
	};

	bool MapLoader::LoadWDF(const std::string& filePath, std::vector<Mesh>& outMeshes, float worldScale)
	{
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open())
		{
			TraceLog(LOG_ERROR, "Couldn't open WDF file for loading: %s ", filePath.c_str());
			return false;
		}

		// verify magic header
		char magic[4];
		file.read(magic, 4);
		if (std::string(magic, 4) != "WDF1")
		{
			TraceLog(LOG_ERROR, "Invaild WDF file format or version");
			return false;
		}

		uint32_t brushCount = 0;
		uint32_t meshCount = 0;
		file.read(reinterpret_cast<char*>(&brushCount), sizeof(brushCount));
		file.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));

		outMeshes.clear();
		outMeshes.resize(meshCount);

		for (uint32_t m = 0; m < meshCount; m++)
		{
			// Read texture name
			uint32_t texNameLen = 0;
			file.read(reinterpret_cast<char*>(&texNameLen), sizeof(texNameLen));
			std::string texName(texNameLen, '\0');
			file.read(&texName[0], texNameLen);
			outMeshes[m].TextureName = texName;

			// Read vertices
			uint32_t vertexCount = 0;
			file.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));

			std::vector<CompilerVertexBinary> binaryVertices(vertexCount);
			file.read(reinterpret_cast<char*>(binaryVertices.data()), vertexCount * sizeof(CompilerVertexBinary));

			for (const auto& bv : binaryVertices)
			{
				Vertex v;

				// Convert Trenchbroom Z-up to Raylib Y-up and scale down from inches to meters
				v.position.x = bv.position.x * worldScale;
				v.position.y = bv.position.z * worldScale;
				v.position.z = bv.position.y * worldScale;

				v.normal.x = bv.normal.x;
				v.normal.y = bv.normal.z;
				v.normal.z = bv.normal.y;

				v.texCoords = { bv.u, bv.v };
				outMeshes[m].AddVertex(v);
			}

			// Read indices
			uint32_t indexCount = 0;
			file.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));

			std::vector<unsigned short> indices(indexCount);
			file.read(reinterpret_cast<char*>(indices.data()), indexCount * sizeof(unsigned short));

			for (unsigned short idx : indices)
				outMeshes[m].AddIndex(idx);
		}

		TraceLog(LOG_ERROR, "Successfully loaded WDF map with %u meshes", meshCount);
		return true;
	}
}