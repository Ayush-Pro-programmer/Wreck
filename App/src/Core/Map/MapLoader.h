#pragma once
#include <string>
#include <vector>
#include "Core/Mesh.h"

namespace Wreck {

	class MapLoader
	{
	public:
		// loads wreck data file
		static bool LoadWDF(const std::string& filePath, std::vector<Mesh>& outMeshes, float worldScale = 1.0f / 32.0f);
	};
}