#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Entity.h"
#include "MapLoader.h"

namespace Wreck {

	class MapManager
	{
	public:
		static MapManager& Get()
		{
			static MapManager instance;
			return instance;
		}

		// non-copyable
		MapManager(const MapManager&) = delete;
		MapManager& operator=(const MapManager&) = delete;

		bool LoadMap(const std::string& path);
		void UnloadMap();

		void Update(float dt);
		void Render() const;

		const MapLoader& GetActiveMap() const { return m_ActiveMap; }
		const std::vector<std::unique_ptr<Entity>>& GetEntities() const { return m_Entities; }

	private:
		MapManager() = default;
		~MapManager() { UnloadMap(); }

	private:
		MapLoader m_ActiveMap;
		std::vector<std::unique_ptr<Entity>> m_Entities;
		
		float m_CurrentScale = 1.0f;
	};
}