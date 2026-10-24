#pragma once
#include "raylib.h"

struct PlayerInput
{
	Vector2 move = { 0 };
	Vector2 look = { 0 };

	bool jumpPressed  = false;
	bool jumpReleased = false;

	void Sample()
	{
		move = { 0 };

		if (IsKeyDown(KEY_W)) move.y += 1.0f;
		if (IsKeyDown(KEY_S)) move.y -= 1.0f;
		if (IsKeyDown(KEY_A)) move.x -= 1.0f;
		if (IsKeyDown(KEY_D)) move.x += 1.0f;

		jumpPressed = IsKeyPressed(KEY_SPACE);
		jumpReleased = IsKeyReleased(KEY_SPACE);

		look = GetMouseDelta();
	}
};