#pragma once
#include "Wreck.h"
#include "PlayerInput.h"

class PlayerMove
{
public:
	void Init(Vector3 position);

	void UpdateVariables(const PlayerInput& input, float cameraYaw);
	void UpdateMovement(float dt);

	Vector3 GetPosition() const;
	const BoundingBox& GetBox() const { return m_Box; }

	void RenderDebug() const;

private:
	void UpdateInput(const PlayerInput& input, float cameraYaw);

	void Friction(float dt);
	void Accelerate(Vector3& wishDir, float wishSpeed, float accel, float dt);
	void AirAccelerate(Vector3& wishDir, float wishSpeed, float accel, float dt);

private:
	BoundingBox m_Box;
	Vector3 m_Velocity = { 0 };
	Vector3 m_WishDir = { 0 };

	bool m_JumpPressed = false;
	bool m_IsGrounded = false;

	const float SPEED = 320.0f * Wreck::WORLD_SCALE;
	const float AIR_SPEED = 30.0f * Wreck::WORLD_SCALE;
	const float STOP_SPEED = 100.0f * Wreck::WORLD_SCALE;
	const float ACCELERATE = 10.0f;
	const float FRICTION = 4.0f;

	const float JUMP_FORCE = 20.0f;
};