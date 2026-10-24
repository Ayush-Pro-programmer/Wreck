#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

#include "raylib.h"

namespace Wreck {
	
	struct RuntimeMesh
	{
		std::string textureName;
		Mesh raylibMesh;
		Material material;
	};

	struct RuntimeAABB
	{
		Vector3 min;
		Vector3 max;
	};

	struct RuntimeTriangle
	{
		Vector3 v0, v1, v2;
	};

	struct RuntimeBVHNode
	{
		RuntimeAABB bounds;
		int leftChildIndex;
		int rightChildIndex;
		std::vector<uint32_t> brushIndices;
		std::vector<RuntimeTriangle> triangles;

		bool IsLeaf() const { return leftChildIndex == -1 && rightChildIndex == -1; }
	};

	struct RuntimeEntity
	{
		std::string classname;
		std::unordered_map<std::string, std::string> properties;

		bool hasBrushEntity = false;
		std::vector<RuntimeMesh> meshes;
		std::vector<RuntimeTriangle> collisionTriangles;

		std::string GetProperty(
			const std::string& key,
			const std::string& defaultValue = "") const
		{
			auto it = properties.find(key);
			if (it != properties.end()) return it->second;
			return defaultValue;
		}
	};

	class MapLoader
	{
	public:
		MapLoader() = default;
		~MapLoader() { Unload(); }

		// prevent dangerous shallow copies of GPU/CPU resources
		MapLoader(const MapLoader&) = delete;
		MapLoader& operator=(const MapLoader&) = delete;
		MapLoader(MapLoader&&) noexcept = default;
		MapLoader& operator=(MapLoader&&) noexcept = default;

		bool Load(const std::string& path, float scale);
		void RenderMap() const;	
		void Unload();

		const std::vector<RuntimeMesh>& GetMeshes() const { return m_Meshes; }
		const std::vector<RuntimeBVHNode>& GetBVHNodes() const { return m_BVHNodes; }
		const std::vector<RuntimeEntity>& GetEntities() const { return m_Entities; }

	private:
		bool LoadMeshes(std::ifstream& file, uint32_t meshCount, std::vector<RuntimeMesh>& outMeshes);
		bool LoadBVH(std::ifstream& file, uint32_t nodeCount);
		bool LoadEntities(std::ifstream& file, uint32_t entityCount);

	private:
		std::vector<RuntimeMesh> m_Meshes;
		std::vector<RuntimeBVHNode> m_BVHNodes;
		std::vector<RuntimeEntity> m_Entities;

		float m_Scale = 1.0f;
		Texture2D m_DefaultTexture = { 0 };
	};

}