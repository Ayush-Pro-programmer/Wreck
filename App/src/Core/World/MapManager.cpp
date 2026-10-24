#include "raylib.h"
#include "MapManager.h"
#include "EntityRegistry.h"

namespace Wreck {

	bool MapManager::LoadMap(const std::string& path, float scale)
	{
		UnloadMap();

		m_CurrentScale = scale;

		if (!m_ActiveMap.Load(path, m_CurrentScale))
		{
			TraceLog(LOG_ERROR, "[MapManager] Failed to load map: %s", path.c_str());
			return false;
		}

        for (const auto& rawEntities : m_ActiveMap.GetEntities())
        {
            if (rawEntities.classname == "worldspawn") continue;

            auto entity = EntityRegistry::Create(rawEntities.classname);
            if (entity)
            {
                entity->ParseProperties(rawEntities.properties, m_CurrentScale);

                entity->hasBrushEntity = rawEntities.hasBrushEntity;
                entity->meshes = rawEntities.meshes;
                entity->collisionTriangles = rawEntities.collisionTriangles;

                m_Entities.push_back(std::move(entity));
            }
            else
                TraceLog(LOG_WARNING, "[MapManager] Unregistered entity classname: %s", rawEntities.classname.c_str());
        }

        for (auto& entity : m_Entities)
        {
            entity->FindTarget(m_Entities);
            entity->Spawn();
        }

        TraceLog(LOG_INFO, "[MapManager] Successfully initialized map with %zu active entities.", m_Entities.size());
        return true;
	}

    void MapManager::UnloadMap()
    {
        for (auto& entity : m_Entities)
            if (entity) entity->Unload();

        m_Entities.clear();
        m_ActiveMap.Unload();
    }

    void MapManager::Update(float dt)
    {
        for (auto& entity : m_Entities)
            entity->Update(dt);
    }

    void MapManager::Render() const
    {
        m_ActiveMap.RenderMap();

        for (auto& entity : m_Entities)
            entity->Render();
    }
}