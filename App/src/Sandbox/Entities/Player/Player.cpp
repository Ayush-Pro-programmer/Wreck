#include "Player.h"

void Player::Spawn()
{
	Wreck::Transform target = {};
	target.position = this->position;

	m_PCamera.Init(target);

	TraceLog(LOG_INFO, "[Player] Spawned at position : (%.2f,%.2f,%.2f)",
		position.x, position.y, position.z);
}

void Player::Update(float dt)
{
	m_PInput.Sample();

	m_PCamera.UpdateRotation(m_PInput.look);
	m_PCamera.SyncCamera();
}

void Player::Render()
{
}

void Player::Unload()
{
}