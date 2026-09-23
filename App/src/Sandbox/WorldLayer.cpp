#include "WorldLayer.h"

WorldLayer::WorldLayer()
	: Layer("WorldLayer")
{}

void WorldLayer::OnAttach()
{
    m_Camera.position = { 0.0f, 2.0f, 6.0f };
    m_Camera.target = { 0.0f, 0.0f, 0.0f };
    m_Camera.up = { 0.0f, 1.0f, 0.0f };
    m_Camera.fovy = 45.0f;
    m_Camera.projection = CAMERA_PERSPECTIVE;

    std::string mapPath = "E:/Programming/C++/Repos/Wreck/Assets/Maps/test.wdf";

    if (Wreck::MapLoader::LoadWDF(mapPath, m_MapMeshes))
    {
        m_MapRenderer.Initialize(m_MapMeshes);
        TraceLog(LOG_INFO, "[WorldLayer] Map loaded successfully!");
    }
    else
        TraceLog(LOG_ERROR, "[WorldLayer] Failed to load map: %u", mapPath);
}

void WorldLayer::OnDetach()
{
    m_MapRenderer.Unload();
}

void WorldLayer::OnUpdate(float dt)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (IsCursorHidden()) EnableCursor();
        else DisableCursor();
    }

    if (IsCursorHidden())
    {
        UpdateCamera(&m_Camera, CAMERA_FREE);
    }
}

void WorldLayer::OnRender()
{
    // 1. Setup ray casting against your loaded map meshes
    Ray ray = GetScreenToWorldRay(GetMousePosition(), m_Camera);
    RayCollision collision = { 0 };
    collision.distance = FLT_MAX;
    collision.hit = false;

    // Test ray collision against all sub-meshes in your loaded map
    for (const auto& batch : m_MapRenderer.GetBatches()) // (Ensure you have a getter or access to raylib meshes)
    {
        RayCollision meshHit = GetRayCollisionMesh(ray, batch.raylibMesh, MatrixIdentity());
        if (meshHit.hit && meshHit.distance < collision.distance)
        {
            collision = meshHit;
        }
    }

    BeginMode3D(m_Camera);

    // Draw your world
    m_MapRenderer.Draw();
    DrawGrid(20, 1.0f);

    // 2. If we ray-hit a mesh face, draw the indicator cube and normal ray vector!
    if (collision.hit)
    {
        // Draw a small cube at the exact point where the mouse hit the mesh
        DrawCube(collision.point, 0.2f, 0.2f, 0.2f, ORANGE);
        DrawCubeWires(collision.point, 0.2f, 0.2f, 0.2f, RED);

        // Calculate normal vector projection endpoint
        Vector3 normalEnd = {
            collision.point.x + (collision.normal.x * 0.75f),
            collision.point.y + (collision.normal.y * 0.75f),
            collision.point.z + (collision.normal.z * 0.75f)
        };

        // Draw the normal ray line coming straight off the face
        DrawLine3D(collision.point, normalEnd, GREEN);
    }

    DrawRay(ray, MAROON);

    EndMode3D();

    // 3. Screen Text Debug Overlay
    DrawText("Right-click to toggle camera look", 10, 10, 20, DARKGRAY);
    if (collision.hit)
    {
        DrawText(TextFormat("Hit Normal: %.2f, %.2f, %.2f", collision.normal.x, collision.normal.y, collision.normal.z), 10, 30, 20, BLACK);
    }
    else
    {
        DrawText("Hit: None", 10, 30, 20, BLACK);
    }

    DrawFPS(10, 50);
}