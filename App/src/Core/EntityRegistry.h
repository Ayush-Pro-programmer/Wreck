#pragma once
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

#include "Entity.h"

namespace Wreck {

	using EntityCreator = std::function<std::unique_ptr<Entity>()>;

	class EntityRegistry
	{
	public:
		static void Register(const std::string& classname, EntityCreator creator)
		{
			GetRegistry()[classname] = creator;
		}

		static std::unique_ptr<Entity> Create(const std::string& classname)
		{
			auto& reg = GetRegistry();
			auto it = reg.find(classname);
			if (it != reg.end())
				return it->second();

			return nullptr;
		}

	private:
		static std::unordered_map<std::string, EntityCreator>& GetRegistry()
		{
			static std::unordered_map<std::string, EntityCreator> registry;
			return registry;
		}
	};
}

#define	LINK_ENTITY_TO_CLASS(ClassName, ClassNameString) \
public: \
	static std::unique_ptr<Wreck::Entity> CreateInstance() { \
		return std::make_unique<ClassName>(); \
	} \
private: \
	struct Registration { \
		Registration() { \
			Wreck::EntityRegistry::Register(ClassNameString, &ClassName::CreateInstance); \
		} \
	}; \
	inline static Registration s_Registration;	