#include <iostream>
#include <string>
#include <vector>

#include "BVH.h"
#include "Brush.h"
#include "Entity.h"
#include "Parser.h"
#include "Serializer.h"
#include "GeometryBuilder.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: Wreck-Compiler.exe <input.map>\n";
		return 1;
	}

	std::string inputPath = argv[1];
	std::string outputPath = argv[2];
	std::vector<BrushData> brushes;
	std::vector<EntityData> entities;

	// parses .map file into raw brushes and entities
	if (!MapParser::ParseFile(inputPath, brushes, entities))
		return 1;

	// compile raw brushes into 3D geometry and meshes
	WorldMap worldMap = GeometryBuilder::Compile(brushes);
	BVHWorldMap bvhMap = BVHBuilder::Build(brushes);

	// serialize map
	SerializeMap(outputPath, worldMap, bvhMap, entities);

	size_t totalVertices = 0;
	size_t totalTriangles = 0;
	for (const auto& mesh : worldMap.meshes)
	{
		totalVertices += mesh.vertices.size();
		totalTriangles += mesh.indices.size() / 3;
	}

	std::cout << "\n=== COMPILATION COMPLETE ===\n";
	std::cout << " - Total Brushes : " << worldMap.totalBrushes << "\n";
	std::cout << " - Total Faces : " << worldMap.totalFaces << "\n";
	std::cout << " - Total Meshes : " << worldMap.meshes.size() << "\n";
	std::cout << " - Total Vertices : " << totalVertices << "\n";
	std::cout << " - Total Triangles : " << totalTriangles << "\n";
	std::cout << " - Total Entities : " << entities.size() << "\n";
	std::cout << " - BVH Tree Nodes : " << bvhMap.nodes.size() << "\n";

	return 0;
}