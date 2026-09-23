#include "Core/Application.h"
#include "Sandbox/WorldLayer.h"

int main()
{
	Wreck::AppDesc appDesc;
	appDesc.Width = 1280;
	appDesc.Height = 720;
	appDesc.Name = "[Wreck]";

	Wreck::Application app(appDesc);

    app.PushLayer(new WorldLayer());

	app.Run();
	return 0;
}