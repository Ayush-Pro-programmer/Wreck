#include "Wreck.h"
#include "Application.h"

namespace Wreck {

	Application::Application()
	{
		int WindowWidth = 1280;
		int WindowHeight = 720;
		const char* WindowTitle = "[Wreck]";

		InitWindow(WindowWidth, WindowHeight, WindowTitle);
		SetTargetFPS(60);
	}

	Application::~Application()
	{
		CloseWindow();
	}

	void Application::Run()
	{
		while (!WindowShouldClose())
		{
			BeginDrawing();
			ClearBackground(SKYBLUE);

			DrawFPS(10, 10);
			DrawText("Wreck Engine", 10, 30, 20, RAYWHITE);

			EndDrawing();
		}
	}
}