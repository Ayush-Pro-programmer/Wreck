#include <fstream>

#include "MapLoader.h"
#include "raymath.h"

namespace Wreck {

	namespace LoaderUtils {

		template <typename T>
		inline void Read(std::ifstream& file, T& outData)
		{
			file.read(reinterpret_cast<char*>(&outData), sizeof(T));
		}

		inline std::string ReadString(std::ifstream& file)
		{
			uint32_t len = 0;
			Read(file, len);
			std::string str;

			if (len > 0)
			{
				str.resize(len);
				file.read(&str[0], len);
			}

			return str;
		}
	}

	bool MapLoader::Load(const std::string& path, float scale)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			TraceLog(LOG_ERROR, "Failed to open WDF map file: %s", path.c_str());
			return false;
		}

		// verify the magic header
		char magic[4];
		file.read(magic, 4);
		if (std::string(magic, 4) != "WDF3")
		{
			TraceLog(LOG_ERROR, "Invalid WDF file header! Expected WDF3.");
			return false;
		}

		uint32_t brushCount = 0;
		uint32_t meshCount = 0;
		LoaderUtils::Read(file, brushCount);
		LoaderUtils::Read(file, meshCount);

		Image defaultImg = GenImageChecked(128, 128, 32, 32, LIGHTGRAY, GRAY);
		Texture2D defaultTexture = LoadTextureFromImage(defaultImg);
		UnloadImage(defaultImg);

		// load mesh
		if (!LoadMeshes(file, scale, meshCount, defaultTexture))
			return false;

		// load BVH tree
		uint32_t nodeCount = 0;
		LoaderUtils::Read(file, nodeCount);
		if (!LoadBVH(file, nodeCount, scale))
			return false;

		// load entities
		uint32_t entityCount = 0;
		LoaderUtils::Read(file, entityCount);
		if (!LoadEntities(file, entityCount))
			return false;

		TraceLog(LOG_INFO, "[Map] Loading map: %s (%d brushes, %d meshes, %d entities)",
			path.c_str(), brushCount, meshCount, entityCount);

		return true;
	}

	bool MapLoader::LoadMeshes(std::ifstream& file, float scale,
		uint32_t meshCount, Texture2D defaultTexture)
	{
		m_Meshes.clear();
		m_Meshes.reserve(meshCount);

		for (uint32_t i = 0; i < meshCount; i++)
		{
			std::string texName = LoaderUtils::ReadString(file);

			uint32_t vertexCount = 0;
			LoaderUtils::Read(file, vertexCount);
			
			Mesh rMesh = { 0 };
			rMesh.vertexCount = vertexCount;
			rMesh.vertices = (float*)MemAlloc(vertexCount * 3 * sizeof(float));
			rMesh.normals = (float*)MemAlloc(vertexCount * 3 * sizeof(float));
			rMesh.texcoords = (float*)MemAlloc(vertexCount * 2 * sizeof(float));

			for (uint32_t v = 0; v < vertexCount; v++)
			{
				struct { float px, py, pz, nx, ny, nz, u, v; } vData;
				LoaderUtils::Read(file, vData);

				rMesh.vertices[v * 3 + 0] = vData.px * scale;
				rMesh.vertices[v * 3 + 1] = vData.pz * scale;
				rMesh.vertices[v * 3 + 2] = vData.py * scale;

				rMesh.normals[v * 3 + 0] = vData.nx;
				rMesh.normals[v * 3 + 1] = vData.nz;
				rMesh.normals[v * 3 + 2] = vData.ny;

				rMesh.texcoords[v * 2 + 0] = vData.u;
				rMesh.texcoords[v * 2 + 1] = vData.v;
			}

			uint32_t indexCount = 0;
			LoaderUtils::Read(file, indexCount);

			rMesh.triangleCount = indexCount / 3;
			rMesh.indices = (unsigned short*)MemAlloc(indexCount * sizeof(unsigned short));

			for (uint32_t idx = 0; idx < indexCount; idx++)
			{
				unsigned short indexValue;
				LoaderUtils::Read(file, indexValue);
				rMesh.indices[idx] = indexValue;
			}

			UploadMesh(&rMesh, false);

			RuntimeMesh compiledMesh;
			compiledMesh.textureName = texName;
			compiledMesh.raylibMesh = rMesh;
			compiledMesh.material = LoadMaterialDefault();

			if (compiledMesh.textureName == "__TB_empty")
			{
				compiledMesh.material.maps[MATERIAL_MAP_DIFFUSE].texture = defaultTexture;
			}
			else
			{
				// TODO: future textures management
			}

			m_Meshes.push_back(compiledMesh);
		}

		return true;
	}

	bool MapLoader::LoadBVH(std::ifstream& file, uint32_t nodeCount, float scale)
	{
		m_BVHNodes.resize(nodeCount);

		for (uint32_t i = 0; i < nodeCount; i++)
		{
			Vector3 rawMin, rawMax;
			LoaderUtils::Read(file, rawMin);
			LoaderUtils::Read(file, rawMax);

			LoaderUtils::Read(file, m_BVHNodes[i].leftChildIndex);
			LoaderUtils::Read(file, m_BVHNodes[i].rightChildIndex);

			Vector3 swizzledMin = { rawMin.x * scale, rawMin.z * scale, rawMin.y * scale };
			Vector3 swizzledMax = { rawMax.x * scale, rawMax.z * scale, rawMax.y * scale };

			m_BVHNodes[i].bounds.min = {
				fminf(swizzledMin.x, swizzledMax.x),
				fminf(swizzledMin.y, swizzledMax.y),
				fminf(swizzledMin.z, swizzledMax.z),
			};
			m_BVHNodes[i].bounds.max = {
				fmaxf(swizzledMin.x, swizzledMax.x),
				fmaxf(swizzledMin.y, swizzledMax.y),
				fmaxf(swizzledMin.z, swizzledMax.z),
			};

			// read collision triangles
			uint32_t triCount = 0;
			LoaderUtils::Read(file, triCount);
			m_BVHNodes[i].triangles.resize(triCount);

			for (uint32_t t = 0; t < triCount; t++)
			{
				Vector3 v0, v1, v2;
				LoaderUtils::Read(file, v0);
				LoaderUtils::Read(file, v1);
				LoaderUtils::Read(file, v2);

				m_BVHNodes[i].triangles[t] = {
					{ v0.x * scale, v0.z * scale, v0.y * scale },
					{ v1.x * scale, v1.z * scale, v1.y * scale },
					{ v2.x * scale, v2.z * scale, v2.y * scale },
				};
			}

			uint32_t brushIdxCount = 0;
			LoaderUtils::Read(file, brushIdxCount);
			m_BVHNodes[i].brushIndices.resize(brushIdxCount);

			for (uint32_t b = 0; b < brushIdxCount; b++)
			{
				uint32_t bIdx;
				LoaderUtils::Read(file, bIdx);
				m_BVHNodes[i].brushIndices[b] = bIdx;
			}
		}

		TraceLog(LOG_INFO, "[Map] Loaded BVH tree with %d nodes.", nodeCount);
		return true;
	}

	bool MapLoader::LoadEntities(std::ifstream& file, uint32_t entityCount)
	{
		m_Entities.resize(entityCount);

		for (uint32_t i = 0; i < entityCount;  i++)
		{
			m_Entities[i].classname = LoaderUtils::ReadString(file);

			uint32_t propCount = 0;
			LoaderUtils::Read(file, propCount);

			for (uint32_t p = 0; p < propCount; p++)
			{
				std::string key = LoaderUtils::ReadString(file);
				std::string value = LoaderUtils::ReadString(file);
				m_Entities[i].properties[key] = value;
			}
		}

		TraceLog(LOG_INFO, "[Map] Loaded %d entities", entityCount);
		return true;
	}

	void MapLoader::RenderMap() const {
		
		for (const auto& mesh : m_Meshes)
			DrawMesh(mesh.raylibMesh, mesh.material, MatrixIdentity());
	}

	void MapLoader::RenderBVH() const
	{
		if (m_BVHNodes.empty()) return;

		// advance animation timer (changes active node every 0.5 seconds)
		m_BVHAnimTimer += GetFrameTime();
		if (m_BVHAnimTimer >= 0.5)
		{
			m_BVHAnimTimer = 0.0f;
			m_CurrentBVHStep++;

			// loop back to root after reaching the final node
			if (m_CurrentBVHStep >= static_cast<int>(m_BVHNodes.size()))
				m_CurrentBVHStep = 0;
		}

		// fetch the current node in the traversal sequence
		int i = m_CurrentBVHStep;
		const auto& node = m_BVHNodes[i];

		// ensure strict min / max ordering
		Vector3 rawMin =
		{
			fminf(node.bounds.min.x, node.bounds.max.x),
			fminf(node.bounds.min.y, node.bounds.max.y),
			fminf(node.bounds.min.z, node.bounds.max.z)
		};

		Vector3 rawMax =
		{
			fmaxf(node.bounds.max.x, node.bounds.min.x),
			fmaxf(node.bounds.max.y, node.bounds.min.y),
			fmaxf(node.bounds.max.z, node.bounds.min.z)
		};

		// calculate dimensions
		Vector3 size =
		{
			rawMax.x - rawMin.x,
			rawMax.y - rawMin.y,
			rawMax.z - rawMin.z
		};

		// skip invalid boxes
		if (size.x <= 0.0f || size.y <= 0.0f || size.z <= 0.0f || size.x > 10000.0f)
			return;

		Vector3 center =
		{
			(rawMin.x + rawMax.x) * 0.5f,
			(rawMin.y + rawMax.y) * 0.5f,
			(rawMin.z + rawMax.z) * 0.5f
		};

		// color code based on node leaf
		Color boxColor = GREEN;
		if (i == 0)
			boxColor = RED;
		else if (!node.IsLeaf())
			boxColor = YELLOW;

		// draw the active node distinctly
		DrawCubeV(center, size, Fade(boxColor, 0.25f));
		DrawCubeWiresV(center, size, boxColor);
	}

	void MapLoader::Unload()
	{
		for (auto& mesh : m_Meshes)
			UnloadMesh(mesh.raylibMesh);

		m_Meshes.clear();
		m_BVHNodes.clear();
		m_Entities.clear();
	}
}