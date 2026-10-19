#include "WorldLayer.h"

WorldLayer::WorldLayer()
	: Layer("WorldLayer")
{}

void WorldLayer::OnAttach()
{
    DisableCursor();

    m_Camera.position = { 0.0f, 2.0f, 6.0f };
    m_Camera.target = { 0.0f, 0.0f, 0.0f };
    m_Camera.up = { 0.0f, 1.0f, 0.0f };
    m_Camera.fovy = 45.0f;
    m_Camera.projection = CAMERA_PERSPECTIVE;

    std::string mapPath = "E:/Programming/C++/Repos/Wreck/Assets/Maps/test_01.wdf";

    if (m_TestMap.Load(mapPath, worldScale))
    {
        for (const auto& rawEntities : m_TestMap.GetEntities())
        {
            auto entity = Wreck::EntityRegistry::Create(rawEntities.classname);
            if (entity)
            {
                entity->ParseProperties(rawEntities.properties, worldScale);
                entity->Spawn();
                m_Entities.push_back(std::move(entity));
            }
            else
                TraceLog(LOG_WARNING, "[Entity] Unregistered entity classname: %s", rawEntities.classname.c_str());
        }
    }
}

void WorldLayer::OnDetach()
{
    for (auto& entity : m_Entities)
        entity->Unload();
    m_Entities.clear();

    m_TestMap.Unload();
}

static float testBoxY = 1.0f;
static Vector3 boxSize = { 1.0f, 1.0f, 1.0f };
static Vector3 boxCenter = { 0.0f, testBoxY, 0.0f };
static BoundingBox testBox = {
    {boxCenter.x - boxSize.x * 0.5f, boxCenter.y - boxSize.y * 0.5f, boxCenter.z - boxSize.z * 0.5f, },
    {boxCenter.x + boxSize.x * 0.5f, boxCenter.y + boxSize.y * 0.5f, boxCenter.z + boxSize.z * 0.5f, },
};
static Color boxColor = GREEN;

void WorldLayer::OnUpdate(float dt)
{
    if (Wreck::ConsoleLayer::Get()->IsOpen())
        return;

    UpdateCamera(&m_Camera, CAMERA_FREE);

    for (auto& entity : m_Entities)
        entity->Update(dt);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 screenCenter = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        Ray ray = GetScreenToWorldRay(screenCenter, m_Camera);
        Wreck::RayCastHit hit = physics.Raycast(m_TestMap, ray);

        if (hit.hit)
            TraceLog(LOG_INFO, "Hit geometry at distance: %.2f.", hit.distance);
        else
            TraceLog(LOG_INFO, "Ray missed all geometry.");
    }

    if (IsKeyDown(KEY_I)) testBoxY += 2.0f * dt;
    if (IsKeyDown(KEY_K)) testBoxY -= 2.0f * dt;

    // recalculate box coordinates every frame
    boxCenter = { 0.0f, testBoxY, 0.0f };
    testBox = {
        { boxCenter.x - boxSize.x * 0.5f, boxCenter.y - boxSize.y * 0.5f, boxCenter.z - boxSize.z * 0.5f, },
        { boxCenter.x + boxSize.x * 0.5f, boxCenter.y + boxSize.y * 0.5f, boxCenter.z + boxSize.z * 0.5f, },
    };

    // AABB overlap
    bool isColliding = physics.CheckAABBOverlap(m_TestMap, testBox);
    boxColor = isColliding ? RED : GREEN;
}

void WorldLayer::OnRender()
{
    BeginMode3D(m_Camera);

    for (auto& entity : m_Entities)
        entity->Render();

    DrawGrid(40, 1);
    m_TestMap.RenderMap(); 
    //m_TestMap.RenderBVH(); 

    DrawCubeV(boxCenter, boxSize, Fade(boxColor, 0.4f));
    DrawCubeWiresV(boxCenter, boxSize, boxColor);

    EndMode3D();

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    DrawRectangle(screenWidth / 2 - 8, screenHeight / 2 - 2, 16, 4, WHITE);
    DrawRectangle(screenWidth / 2 - 2, screenHeight / 2 - 8, 4, 16, WHITE);

    DrawFPS(10, 10);
}