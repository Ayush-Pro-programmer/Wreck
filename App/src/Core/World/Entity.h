#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "raylib.h"
#include "raymath.h"
#include "MapLoader.h"

namespace Wreck {

	class Entity
	{
	public:
		virtual ~Entity() = default;

		virtual void ParseProperties(
			const std::unordered_map<std::string, std::string>& props,
			float scale)
		{
			m_Properties = props;

			auto it = m_Properties.find("origin");
			if (it != m_Properties.end())
			{
				// swizzle up coordinates
				float x = 0, y = 0, z = 0;
				if (sscanf(it->second.c_str(), "%f %f %f", &x, &y, &z) == 3)
					position = { x * scale, z * scale, y * scale };
			}

			auto targetNameIt = m_Properties.find("targetname");
			if (targetNameIt != m_Properties.end())
				targetname = targetNameIt->second;

			auto targetIt = m_Properties.find("target");
			if (targetIt != m_Properties.end())
				target = targetIt->second;
		}

		virtual void FindTarget(const std::vector<std::unique_ptr<Entity>>& allEntities)
		{
			if (target.empty()) return;

			for (const auto& entity : allEntities)
			{
				if (entity->targetname == target)
				{
					targetEntity = entity.get();
					break;
				}
			}
		}

		std::string GetValue(
			const std::string& key, 
			const std::string& defaultValue = "") const
		{
			auto it = m_Properties.find(key);
			if (it != m_Properties.end())
				return it->second;
			return defaultValue;
		}

		float GetFloat(
			const std::string& key,
			float defaultValue = 0.0f) const
		{
			std::string value = GetValue(key);
			if (value.empty()) return defaultValue;
			try {
				return std::stof(value);
			}
			catch (...) {
				return defaultValue;
			}
		}

		int GetInt(
			const std::string& key,
			int defaultValue = 0) const
		{
			std::string value = GetValue(key);
			if (value.empty()) defaultValue;
			try {
				return std::stoi(value);
			}
			catch (...) {
				return defaultValue;
			}
		}

		bool GetBool(
			const std::string& key,
			bool defaultValue = false) const
		{
			std::string value = GetValue(key);
			if (value.empty()) return defaultValue;
			return (value == "1" || value == "true" || value == "TRUE");
		}

		// lifecycle
		virtual void Spawn() {}
		virtual void Update(float dt) {}
		virtual void Render() 
		{
			for (const auto& mesh : meshes)
				DrawMesh(mesh.raylibMesh, mesh.material, MatrixTranslate(position.x, position.y, position.z));
		}
		virtual void Unload() {}
		virtual void Use(Entity* activator) {}

		Vector3 position = { 0 };
		std::string targetname = "";
		std::string target = "";
		Entity* targetEntity = nullptr;

		bool hasBrushEntity = false;
		std::vector<RuntimeMesh> meshes;
		std::vector<RuntimeTriangle> collisionTriangles;

	private:
		std::unordered_map<std::string, std::string> m_Properties;
	};
}