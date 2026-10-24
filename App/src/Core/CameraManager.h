#pragma once
#include "raylib.h"

namespace Wreck {

	class CameraManager
	{
	public:
		static Camera3D& Get()
		{
			static Camera3D s_Camera = { 0 };
			return s_Camera;
		}
	};
}