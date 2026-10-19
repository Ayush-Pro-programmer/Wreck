#pragma once
#include <string>
#include <vector>
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

		bool Load(const std::string& path, float scale = 0.05f); // loads WDF map file, can be scalled
		void RenderMap() const;	// render actual map geometry
		void RenderBVH() const;	// render bvh tree
		void Unload();

		const std::vector<RuntimeMesh>& GetMeshes() const { return m_Meshes; }
		const std::vector<RuntimeBVHNode>& GetBVHNodes() const { return m_BVHNodes; }
		const std::vector<RuntimeEntity>& GetEntities() const { return m_Entities; }

	private:
		bool LoadMeshes(std::ifstream& file, float scale,
			uint32_t meshCount, Texture2D defaultTexture);
		bool LoadBVH(std::ifstream& file, uint32_t nodeCount, float scale);
		bool LoadEntities(std::ifstream& file, uint32_t entityCount);

	private:
		std::vector<RuntimeMesh> m_Meshes;
		std::vector<RuntimeBVHNode> m_BVHNodes;
		std::vector<RuntimeEntity> m_Entities;

		mutable float m_BVHAnimTimer = 0.0f;
		mutable int m_CurrentBVHStep = 0;
	};

}