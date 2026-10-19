#include "Player.h"

void Player::Spawn()
{
	TraceLog(LOG_INFO, "[Player] Spawned at position : (%.2f,%.2f,%.2f)",
		position.x, position.y, position.z);
}