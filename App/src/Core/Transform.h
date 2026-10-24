#pragma once
#include "raylib.h"

namespace Wreck {

	struct Transform
	{
		Vector3 position = { 0 };
		Vector3 rotation = { 0 };
		Vector3 scale = { 1 };
	};
}