#pragma once
#include <string>

namespace Wreck {

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float dt) {}
		virtual void OnRender() {}

		const std::string GetName() { return m_DebugName; }

	protected:
		std::string m_DebugName;
	};
}