#pragma once
#include "Wreck.h"

class PlayerMove
{
public:
	void UpdateMovement(float dt);

private:
	void Friction(float dt);
	void Accelerate(Vector3& wishDir, float wishSpeed, float accel, float dt);
	void AirAccelerate(Vector3& wishDir, float wishSpeed, float accel, float dt);

private:
	Vector3 m_Velocity = { 0 };

	const float SPEED = 320.0f;
	const float AIR_SPEED = 30.0f;
	const float STOP_SPEED = 120.0f;
	const float ACCELERATE = 10.0f;
	const float FRICTION = 80.0f;

	const float JUMP_FORCE = 220.0f;
};