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
		std::cout << "Usage: Wreck-Compiler.exe <input.map> <output.wdf>\n";
		return 1;
	}

	std::string inputPath = argv[1];
	std::string outputPath = argv[2];
	std::vector<EntityData> entities;

	// parses .map file into raw brushes and entities
	if (!MapParser::ParseFile(inputPath, entities))
		return 1;

	// compile raw world brushes into 3D geometry and meshes
	std::vector<BrushData> worldBrushes;
	for (const auto& entity : entities)
	{
		if (entity.classname == "worldspawn")
		{
			worldBrushes = entity.brushes;
			break;
		}
	}

	WorldMap worldMap = GeometryBuilder::Compile(worldBrushes, true);
	BVHWorldMap bvhMap = BVHBuilder::Build(worldBrushes);

	// compile brush entities independently without culling
	std::unordered_map<size_t, WorldMap> brushEntity;
	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i].classname != "worldspawn" && !entities[i].brushes.empty())
			brushEntity[i] = GeometryBuilder::Compile(entities[i].brushes, false);
	}

	// serialize map
	SerializeMap(outputPath, worldMap, bvhMap, entities, brushEntity);

	size_t totalVertices = 0;
	size_t totalTriangles = 0;
	for (const auto& mesh : worldMap.meshes)
	{
		totalVertices += mesh.vertices.size();
		totalTriangles += mesh.indices.size() / 3;
	}

	std::cout << "\n=== COMPILATION COMPLETE ===\n";
	std::cout << " - Total World Brushes : " << worldMap.totalBrushes << "\n";
	std::cout << " - Total Entities : " << entities.size() << "\n";
	std::cout << " - Total Brush Entities : " << brushEntity.size() << "\n";
	std::cout << " - Total Faces : " << worldMap.totalFaces << "\n";
	std::cout << " - Total Meshes : " << worldMap.meshes.size() << "\n";
	std::cout << " - Total Vertices : " << totalVertices << "\n";
	std::cout << " - Total Triangles : " << totalTriangles << "\n";
	std::cout << " - BVH Tree Nodes : " << bvhMap.nodes.size() << "\n";

	return 0;
}