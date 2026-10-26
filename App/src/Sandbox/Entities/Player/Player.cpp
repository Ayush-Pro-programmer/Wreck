#include "Player.h"

void Player::Spawn()
{
	m_PMove.Init(position);
	m_PCamera.Init(m_Target);

	TraceLog(LOG_INFO, "[Player] Spawned at position : (%.2f,%.2f,%.2f)",
		position.x, position.y, position.z);
}

void Player::Update(float dt)
{
	m_PInput.Sample();

	m_PMove.UpdateVariables(m_PInput, m_PCamera.GetYaw());
	m_PMove.UpdateMovement(dt);

	Vector3 position = m_PMove.GetPosition();
	m_Target.x = position.x;
	m_Target.y = position.y + eyeLevel;
	m_Target.z = position.z;

	m_PCamera.UpdatePosition(m_Target);
	m_PCamera.UpdateRotation(m_PInput.look);
	m_PCamera.SyncCamera();
}

void Player::Render()
{
	m_PMove.RenderDebug();
}

void Player::Unload()
{
}