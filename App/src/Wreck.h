#pragma once

#include "raylib.h"
#include "raymath.h"

#include "Core/Layer.h"
#include "Core/Application.h"
#include "Core/ConsoleLayer.h"
#include "Core/CameraManager.h"

#include "Core/Transform.h"

#include "Core/World/Entity.h"
#include "Core/World/EntityRegistry.h"

#include "Core/World/MapLoader.h"
#include "Core/World/MapManager.h"
#include "Core/World/PhysicsWorld.h"

namespace Wreck {
	inline constexpr float WORLD_SCALE = 0.05f;
	inline constexpr float GRAVITY = 9.81f;
}