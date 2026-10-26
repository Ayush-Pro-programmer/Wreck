#include "PlayerMove.h"

void PlayerMove::Init(Vector3 position)
{
	float width = 32.0f * Wreck::WORLD_SCALE;
	float height = 56.0f * Wreck::WORLD_SCALE;

	m_Box.min =
	{ 
		position.x - width * 0.5f,
		position.y,
		position.z - width * 0.5f 
	};
	m_Box.max = 
	{ 
		position.x + width * 0.5f, 
		position.y + height,
		position.z + width * 0.5f 
	};
}

void PlayerMove::UpdateVariables(const PlayerInput& input, float cameraYaw)
{
	UpdateInput(input, cameraYaw);
}

void PlayerMove::UpdateInput(const PlayerInput& input, float cameraYaw)
{
	Vector3 forward = { std::sin(cameraYaw), 0.0f, std::cos(cameraYaw) };
	Vector3 right = { std::cos(cameraYaw), 0.0f, -std::sin(cameraYaw) };

	// build wish direction 
	Vector3 wishDir = (forward * input.move.y) + (right * input.move.x);

	// normalize wish direction
	float len = Vector3Length(wishDir);
	if (len > 0.0f)
	{
		m_WishDir.x = wishDir.x / len;
		m_WishDir.y = wishDir.y / len;
		m_WishDir.z = wishDir.z / len;
	}
	else
	{
		m_WishDir = { 0 };
	}

	m_JumpPressed = input.jumpPressed;
}

void PlayerMove::UpdateMovement(float dt)
{
	Wreck::PhysicsWorld physicsWorld;
	Wreck::PhysicsSolver solver(physicsWorld);

	float wishSpeed = SPEED;

	if (m_IsGrounded)
	{
		Friction(dt);
		Accelerate(m_WishDir, wishSpeed, ACCELERATE, dt);

		if (m_JumpPressed)
		{
			m_Velocity.y = JUMP_FORCE;
			m_IsGrounded = false;
		}
	}
	else
	{
		AirAccelerate(m_WishDir, wishSpeed, ACCELERATE, dt);
		solver.ApplyGravity(m_Velocity, dt);
	}

	Wreck::MoveResult result = solver.Move(m_Box, m_Velocity, dt);

	m_IsGrounded = result.isGrounded;
	if (m_IsGrounded && m_Velocity.y < 0.0f)
		m_Velocity.y = 0.0f;
}

void PlayerMove::Friction(float dt)
{
	Vector3 velocity;
	float speed, newSpeed, control;
	float friction;
	float drop = 0.0f;

	velocity = m_Velocity;

	speed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
	if (speed < 1)
	{
		velocity.x = 0;
		velocity.z = 0;
		m_Velocity = velocity;
		return;
	}

	friction = FRICTION;

	control = speed < STOP_SPEED ? STOP_SPEED : speed;
	drop += control * friction * dt;

	newSpeed = speed - drop;
	if (newSpeed < 0)
		newSpeed = 0;

	newSpeed /= speed;

	velocity.x = velocity.x * newSpeed;
	velocity.y = velocity.y * newSpeed;
	velocity.z = velocity.z * newSpeed;

	m_Velocity = velocity;
}

void PlayerMove::Accelerate(Vector3& wishDir, float wishSpeed, float accel, float dt)
{
	float addSpeed, accelSpeed, currentSpeed;

	currentSpeed = Vector3DotProduct(m_Velocity, wishDir);
	addSpeed = wishSpeed - currentSpeed;
	if (addSpeed <= 0)
		return;

	accelSpeed = accel * dt * wishSpeed;
	if (accelSpeed > addSpeed)
		accelSpeed = addSpeed;

	m_Velocity.x += accelSpeed * wishDir.x;
	m_Velocity.y += accelSpeed * wishDir.y;
	m_Velocity.z += accelSpeed * wishDir.z;
}

void PlayerMove::AirAccelerate(Vector3& wishDir, float wishSpeed, float accel, float dt)
{
	float addSpeed, accelSpeed, currentSpeed, airWishSpeed;

	airWishSpeed = wishSpeed;

	if (airWishSpeed > AIR_SPEED)
		airWishSpeed = AIR_SPEED;

	currentSpeed = Vector3DotProduct(m_Velocity, wishDir);
	addSpeed = airWishSpeed - currentSpeed;
	if (addSpeed <= 0)
		return;

	accelSpeed = accel * wishSpeed * dt;
	if (accelSpeed > addSpeed)
		accelSpeed = addSpeed;

	m_Velocity.x += accelSpeed * wishDir.x;
	m_Velocity.y += accelSpeed * wishDir.y;
	m_Velocity.z += accelSpeed * wishDir.z;
}

Vector3 PlayerMove::GetPosition() const
{
	Vector3 pos;
	pos.x = m_Box.min.x + (m_Box.max.x - m_Box.min.x) * 0.5f;
	pos.y = m_Box.min.y;
	pos.z = m_Box.min.z + (m_Box.max.z - m_Box.min.z) * 0.5f;

	return pos;
}

void PlayerMove::RenderDebug() const
{
	// Draw the active bounding box wireframe in green
	DrawBoundingBox(m_Box, GREEN);

	// Optional: Draw a tiny red sphere at the feet position returned by GetPosition()
	DrawSphere(GetPosition(), 0.2f, RED);
}