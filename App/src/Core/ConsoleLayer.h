#pragma once
#include <sstream>
#include <vector>
#include <functional>
#include <unordered_map>

#include "Layer.h"

namespace Wreck {

	class ConsoleLayer : public Layer
	{
	public:
		ConsoleLayer();
		~ConsoleLayer() = default;

		void OnAttach() override;
		void OnUpdate(float dt) override;
		void OnRender() override;

		bool IsOpen() const { return m_IsOpen; }
		static ConsoleLayer* Get() { return s_Instance; }

		void Print(const std::string& text);

	private:
		void ExecuteCommand(const std::string& cmdLine);

		std::unordered_map<std::string, std::function<void(const std::vector<std::string>& args)>> m_CommandRegistry;

		bool m_IsOpen = false;
		float m_AnimProgress = 0.0f;
		
		std::vector<std::string> m_History;
		char m_InputBuffer[256] = { 0 };
		int m_InputLen = 0;

		static ConsoleLayer* s_Instance;
	};
}