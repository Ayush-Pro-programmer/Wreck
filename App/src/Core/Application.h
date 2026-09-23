#pragma once
#include <string>
#include "LayerStack.h"

namespace Wreck {

	struct AppDesc
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		std::string Name = "";
	};

	class Application
	{
	public:
		Application(const AppDesc& desc);
		~Application();

		void Run();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }

	private:
		AppDesc m_AppDesc;
		LayerStack m_LayerStack;
		bool m_Running = true;

		static Application* s_Instance;
	};
}
