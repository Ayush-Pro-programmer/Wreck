#include "Core/Application.h"
#include "Core/ConsoleLayer.h"
#include "Sandbox/WorldLayer.h"

int main()
{
	Wreck::AppDesc appDesc;
	appDesc.Width = 1280;
	appDesc.Height = 720;
	appDesc.Name = "[Wreck]";

	Wreck::Application app(appDesc);

	app.PushOverlay(new Wreck::ConsoleLayer());

    app.PushLayer(new WorldLayer());

	app.Run();
	return 0;
}