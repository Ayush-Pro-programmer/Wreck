#pragma once
#include <string>
#include <unordered_map>

#include "raylib.h"

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

			auto targetIt = m_Properties.find("targetname");
			if (targetIt != m_Properties.end())
				targetname = targetIt->second;
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

		// lifecycle
		virtual void Spawn() {}
		virtual void Update(float dt) {}
		virtual void Render() {}
		virtual void Unload() {}

		Vector3 position;
		std::string targetname;

	private:
		std::unordered_map<std::string, std::string> m_Properties;
	};
}