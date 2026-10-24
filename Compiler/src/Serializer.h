#pragma once

#include <fstream>
#include <iostream>

#include "utils/SerializerUtils.h"
#include "GeometryBuilder.h"
#include "Entity.h"
#include "BVH.h"

namespace Serializer {

	inline void WriteMeshes(std::ofstream& file, const std::vector<Mesh>& meshes)
	{
		uint32_t meshCount = static_cast<uint32_t>(meshes.size());
		SerializerUtils::Write(file, meshCount);

		for (const auto& mesh : meshes)
		{
			SerializerUtils::WriteString(file, mesh.textureName);
			SerializerUtils::WriteVector(file, mesh.vertices);
			SerializerUtils::WriteVector(file, mesh.indices);
		}
	}

	inline void WriteBVHNode(
		std::ofstream& file,
		const BVHNode& node, 
		const WorldMap& worldMap)
	{
		// write bounds and child indices
		SerializerUtils::Write(file, node.bounds.min);
		SerializerUtils::Write(file, node.bounds.max);
		SerializerUtils::Write(file, node.leftChildIndex);
		SerializerUtils::Write(file, node.rightChildIndex);

		// gather and write leaf collision triangles
		std::vector<Triangle> leafTriangles;
		if (node.IsLeaf())
		{
			for (size_t brushIdx : node.brushIndices)
			{
				if (brushIdx < worldMap.brushTriangles.size())
				{
					const auto& bTris = worldMap.brushTriangles[brushIdx];
					leafTriangles.insert(leafTriangles.end(), bTris.begin(), bTris.end());
				}
			}
		}

		uint32_t triCount = static_cast<uint32_t>(leafTriangles.size());
		SerializerUtils::Write(file, triCount);
		for (const auto& tri : leafTriangles)
		{
			SerializerUtils::Write(file, tri.v0);
			SerializerUtils::Write(file, tri.v1);
			SerializerUtils::Write(file, tri.v2);
		}

		// write leaf/node brush indices
		uint32_t brushIndexCount = static_cast<uint32_t>(node.brushIndices.size());
		SerializerUtils::Write(file, brushIndexCount);

		for (size_t brushIdx : node.brushIndices)
		{
			uint32_t safeIndex = static_cast<uint32_t>(brushIdx);
			SerializerUtils::Write(file, safeIndex);
		}
	}

	inline void WriteBVH(
		std::ofstream& file,
		const BVHWorldMap& bvhMap, 
		const WorldMap& worldMap)
	{
		uint32_t nodeCount = static_cast<uint32_t>(bvhMap.nodes.size());
		SerializerUtils::Write(file, nodeCount);

		for (const auto& node : bvhMap.nodes)
			WriteBVHNode(file, node, worldMap);
	}

	inline void WriteEntities(
		std::ofstream& file,
		const std::vector<EntityData>& entities,
		const std::unordered_map<size_t, WorldMap>& brushEntity)
	{
		uint32_t entityCount = static_cast<uint32_t>(entities.size());
		SerializerUtils::Write(file, entityCount);

		for (size_t i = 0; i < entities.size(); i++)
		{
			const auto& entity = entities[i];

			SerializerUtils::WriteString(file, entity.classname);

			uint32_t propCount = static_cast<uint32_t>(entity.properties.size());
			SerializerUtils::Write(file, propCount);

			for (const auto& [key, value] : entity.properties)
			{
				SerializerUtils::WriteString(file, key);
				SerializerUtils::WriteString(file, value);
			}

			// check if this entity has compiled brush geometry
			auto it = brushEntity.find(i);
			bool hasBrushGeometry = (it != brushEntity.end());
			SerializerUtils::Write(file, hasBrushGeometry);

			if (hasBrushGeometry)
			{
				const WorldMap& brushMap = it->second;
				WriteMeshes(file, brushMap.meshes);

				size_t totalTriangles = 0;
				for (const auto& bTris : brushMap.brushTriangles)
					totalTriangles += bTris.size();

				SerializerUtils::Write(file, static_cast<uint32_t>(totalTriangles));
				for (const auto& bTris : brushMap.brushTriangles)
				{
					for (const auto& tri : bTris)
					{
						SerializerUtils::Write(file, tri.v0);
						SerializerUtils::Write(file, tri.v1);
						SerializerUtils::Write(file, tri.v2);
					}
				}
			}
		}
	}
}

inline bool SerializeMap(
	const std::string& outputPath,
	const WorldMap& worldMap,
	const BVHWorldMap& bvhMap,
	const std::vector<EntityData>& entities,
	const std::unordered_map<size_t, WorldMap>& brushEntity)
{
	std::ofstream file(outputPath, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Error: Could not open output file for serialization: " << outputPath << "\n";
		return false;
	}

	// magic header to identify the file
	file.write("WDF3", 4);

	// write global metadata
	uint32_t brushCount = static_cast<uint32_t>(worldMap.totalBrushes);
	SerializerUtils::Write(file, brushCount);

	// write meshes
	Serializer::WriteMeshes(file, worldMap.meshes);

	// write bvh tree & collision data
	Serializer::WriteBVH(file, bvhMap, worldMap);

	// write entities
	Serializer::WriteEntities(file, entities, brushEntity);

	return true;
}