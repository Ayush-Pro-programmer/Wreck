#include <iostream>

#include "MapRenderer.h"
#include "raymath.h"

namespace Wreck {

	MapRenderer::~MapRenderer()
	{
		Unload();
	}

	void MapRenderer::Initialize(const std::vector<Mesh>& wreckMeshes)
	{
		m_Batches.clear();
		m_Batches.reserve(wreckMeshes.size());

		Image checkImage = GenImageChecked(64, 64, 32, 32, LIGHTGRAY, GRAY);
		m_DefaultTexture = LoadTextureFromImage(checkImage);
		UnloadImage(checkImage);

		for (const auto& mesh : wreckMeshes)
		{
			RenderBatch batch;
			batch.raylibMesh = mesh.ToRaylibMesh();
			batch.materials = LoadMaterialDefault();

			if (mesh.TextureName == "__TB_empty" || mesh.TextureName.empty())
				batch.materials.maps[MATERIAL_MAP_DIFFUSE].texture = m_DefaultTexture;
			else
				batch.materials.maps[MATERIAL_MAP_DIFFUSE].texture = m_DefaultTexture; // FIXME:future asset manager plugs here

			m_Batches.push_back(batch);
		}
	}

	void MapRenderer::Draw() const
	{
		for (const auto& batch : m_Batches)
		{
			DrawMesh(batch.raylibMesh, batch.materials, MatrixIdentity());
		}
	}

	void MapRenderer::Unload()
	{
		for (auto& batch : m_Batches)
			UnloadMesh(batch.raylibMesh);

		UnloadTexture(m_DefaultTexture);
		m_Batches.clear();
	}
}