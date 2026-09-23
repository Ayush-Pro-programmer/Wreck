#include "raylib.h"
#include "Application.h"

namespace Wreck {

	Application* Application::s_Instance = nullptr;

	Application::Application(const AppDesc& desc)
		: m_AppDesc(desc)
	{
		s_Instance = this;

		int WindowWidth = m_AppDesc.Width;
		int WindowHeight = m_AppDesc.Height;
		const char* WindowTitle = m_AppDesc.Name.c_str();

		InitWindow(WindowWidth, WindowHeight, WindowTitle);
		SetTargetFPS(60);
	}

	Application::~Application()
	{
		CloseWindow();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
	}

	void Application::PopOverlay(Layer* overlay)
	{
		m_LayerStack.PopOverlay(overlay);
	}

	void Application::Run()
	{
		while (!WindowShouldClose() && m_Running)
		{
			float dt = GetFrameTime();

			// Update all layers
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(dt);

			BeginDrawing();
			ClearBackground(SKYBLUE);

			// Render all layers
			for (Layer* layer : m_LayerStack)
				layer->OnRender();

			EndDrawing();
		}
	}
}