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
		Unload();

		std::ifstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			TraceLog(LOG_ERROR, "[ActiveMap] Failed to open WDF map file: %s", path.c_str());
			return false;
		}

		// verify the magic header
		char magic[4];
		file.read(magic, 4);
		if (std::string(magic, 4) != "WDF3")
		{
			TraceLog(LOG_ERROR, "[ActiveMap] Invalid WDF file header! Expected WDF3.");
			return false;
		}

		uint32_t brushCount = 0;
		uint32_t meshCount = 0;
		LoaderUtils::Read(file, brushCount);
		LoaderUtils::Read(file, meshCount);

		Image defaultImg = GenImageChecked(128, 128, 32, 32, LIGHTGRAY, GRAY);
		m_DefaultTexture = LoadTextureFromImage(defaultImg);
		UnloadImage(defaultImg);

		m_Scale = scale;

		// load world meshes
		if (!LoadMeshes(file, meshCount, m_Meshes))
			return false;

		// load BVH tree
		uint32_t nodeCount = 0;
		LoaderUtils::Read(file, nodeCount);
		if (!LoadBVH(file, nodeCount))
			return false;

		// load entities
		uint32_t entityCount = 0;
		LoaderUtils::Read(file, entityCount);
		if (!LoadEntities(file, entityCount))
			return false;

		TraceLog(LOG_INFO, "[ActiveMap] Loading map: %s (%d brushes, %d meshes, %d entities)",
			path.c_str(), brushCount, meshCount, entityCount);

		return true;
	}

	bool MapLoader::LoadMeshes(
		std::ifstream& file,
		uint32_t meshCount, 
		std::vector<RuntimeMesh>& outMeshes)
	{
		outMeshes.clear();
		outMeshes.reserve(meshCount);

		for (uint32_t i = 0; i < meshCount; i++)
		{
			std::string texName = LoaderUtils::ReadString(file);

			uint32_t vertexCount = 0;
			LoaderUtils::Read(file, vertexCount);
			
			Mesh rMesh = { 0 };
			rMesh.vertexCount = vertexCount;
			rMesh.vertices  =	static_cast<float*>(MemAlloc(vertexCount * 3 * sizeof(float)));
			rMesh.normals	=	static_cast<float*>(MemAlloc(vertexCount * 3 * sizeof(float)));
			rMesh.texcoords =	static_cast<float*>(MemAlloc(vertexCount * 2 * sizeof(float)));

			for (uint32_t v = 0; v < vertexCount; v++)
			{
				struct { float px, py, pz, nx, ny, nz, u, v; } vData;
				LoaderUtils::Read(file, vData);

				rMesh.vertices[v * 3 + 0] = vData.px * m_Scale;
				rMesh.vertices[v * 3 + 1] = vData.py * m_Scale;
				rMesh.vertices[v * 3 + 2] = vData.pz * m_Scale;

				rMesh.normals[v * 3 + 0] = vData.nx;
				rMesh.normals[v * 3 + 1] = vData.ny;
				rMesh.normals[v * 3 + 2] = vData.nz;

				rMesh.texcoords[v * 2 + 0] = vData.u;
				rMesh.texcoords[v * 2 + 1] = vData.v;
			}

			uint32_t indexCount = 0;
			LoaderUtils::Read(file, indexCount);

			rMesh.triangleCount = indexCount / 3;
			rMesh.indices = static_cast<unsigned short*>(MemAlloc(indexCount * sizeof(unsigned short)));

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
				compiledMesh.material.maps[MATERIAL_MAP_DIFFUSE].texture = m_DefaultTexture;
			}
			else
			{
				// TODO: future textures management
			}

			outMeshes.push_back(compiledMesh);
		}

		return true;
	}

	bool MapLoader::LoadBVH(std::ifstream& file, uint32_t nodeCount)
	{
		m_BVHNodes.resize(nodeCount);

		for (uint32_t i = 0; i < nodeCount; i++)
		{
			Vector3 rawMin, rawMax;
			LoaderUtils::Read(file, rawMin);
			LoaderUtils::Read(file, rawMax);

			LoaderUtils::Read(file, m_BVHNodes[i].leftChildIndex);
			LoaderUtils::Read(file, m_BVHNodes[i].rightChildIndex);

			m_BVHNodes[i].bounds.min = { rawMin.x * m_Scale, rawMin.y * m_Scale, rawMin.z * m_Scale };
			m_BVHNodes[i].bounds.max = { rawMax.x * m_Scale, rawMax.y * m_Scale, rawMax.z * m_Scale };

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
					{ v0.x * m_Scale, v0.y * m_Scale, v0.z * m_Scale },
					{ v1.x * m_Scale, v1.y * m_Scale, v1.z * m_Scale },
					{ v2.x * m_Scale, v2.y * m_Scale, v2.z * m_Scale },
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
			m_Entities[i].properties.reserve(propCount);

			for (uint32_t p = 0; p < propCount; p++)
			{
				std::string key = LoaderUtils::ReadString(file);
				std::string value = LoaderUtils::ReadString(file);
				m_Entities[i].properties[std::move(key)] = std::move(value);
			}

			LoaderUtils::Read(file, m_Entities[i].hasBrushEntity);

			if (m_Entities[i].hasBrushEntity)
			{
				uint32_t entityMeshCount = 0;
				LoaderUtils::Read(file, entityMeshCount);
				LoadMeshes(file, entityMeshCount, m_Entities[i].meshes);

				uint32_t triCount = 0;
				LoaderUtils::Read(file, triCount);
				m_Entities[i].collisionTriangles.resize(triCount);

				for (uint32_t t = 0; t < triCount; t++)
				{
					Vector3 v0, v1, v2;
					LoaderUtils::Read(file, v0);
					LoaderUtils::Read(file, v1);
					LoaderUtils::Read(file, v2);

					m_Entities[i].collisionTriangles[t] = {
						{ v0.x * m_Scale, v0.y * m_Scale, v0.z * m_Scale },
						{ v1.x * m_Scale, v1.y * m_Scale, v1.z * m_Scale },
						{ v2.x * m_Scale, v2.y * m_Scale, v2.z * m_Scale }
					};
				}
			}
		}

		TraceLog(LOG_INFO, "[Map] Loaded %d entities", entityCount);
		return true;
	}

	void MapLoader::RenderMap() const {
		
		for (const auto& mesh : m_Meshes)
			DrawMesh(mesh.raylibMesh, mesh.material, MatrixIdentity());
	}

	void MapLoader::Unload()
	{
		auto UnloadRuntimeMeshes = [](std::vector<RuntimeMesh>& meshes)
			{
				for (auto& mesh : meshes)
				{
					UnloadMesh(mesh.raylibMesh);
					mesh.raylibMesh = {};
				}

				meshes.clear();
			};

		UnloadRuntimeMeshes(m_Meshes);

		for (auto& entity : m_Entities)
		{
			if (entity.hasBrushEntity)
				UnloadRuntimeMeshes(entity.meshes);
		}

		if (m_DefaultTexture.id != 0)
		{
			UnloadTexture(m_DefaultTexture);
			m_DefaultTexture = {};
		}

		m_BVHNodes.clear();
		m_Entities.clear();
	}

}