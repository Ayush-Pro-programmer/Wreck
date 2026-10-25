#include "PlayerMove.h"


void PlayerMove::UpdateMovement(float dt)
{

}

void PlayerMove::Friction(float dt)
{
	Vector3 velocity;
	float speed, newSpeed, control;
	float friction;
	float drop = 0.0f;

	velocity = m_Velocity;

	speed = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
	if (speed < 1)
	{
		velocity.x = 0;
		velocity.z = 0;
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