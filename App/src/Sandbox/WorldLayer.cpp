#include "WorldLayer.h"

#include "Entities/FuncDoor.h"
#include "Entities/FuncButton.h"

WorldLayer::WorldLayer()
	: Layer("WorldLayer")
{}

void WorldLayer::OnAttach()
{
    DisableCursor();

    std::string mapPath = "E:/Programming/C++/Repos/Wreck/Assets/maps/sandbox_01.wdf";
    Wreck::MapManager::Get().LoadMap(mapPath);
}

void WorldLayer::OnDetach()
{
    Wreck::MapManager::Get().UnloadMap();
}

void WorldLayer::OnUpdate(float dt)
{
    if (Wreck::ConsoleLayer::Get()->IsOpen())
        return;

    Wreck::MapManager::Get().Update(dt);
}

void WorldLayer::OnRender()
{
    Camera3D& camera = Wreck::CameraManager::Get();
    BeginMode3D(camera);

    Wreck::MapManager::Get().Render();

    EndMode3D();

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    DrawRectangle(screenWidth / 2 - 8, screenHeight / 2 - 2, 16, 4, WHITE);
    DrawRectangle(screenWidth / 2 - 2, screenHeight / 2 - 8, 4, 16, WHITE);

    DrawFPS(10, 10);
}