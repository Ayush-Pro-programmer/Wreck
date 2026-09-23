#pragma once
#include <vector>
#include "raylib.h"
#include "Core/Mesh.h"

namespace Wreck {

	struct RenderBatch
	{
		::Mesh raylibMesh;
		::Material materials;
	};

	class MapRenderer
	{
	public:
		MapRenderer() = default;
		~MapRenderer();

		void Initialize(const std::vector<Mesh>& wreckMeshes);
		void Draw() const;
		void Unload();

		const std::vector<RenderBatch>& GetBatches() const { return m_Batches; }

	private:
		Texture2D m_DefaultTexture;
		std::vector<RenderBatch> m_Batches;
	};

}