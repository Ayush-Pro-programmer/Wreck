#pragma once
#include "Wreck.h"

#include "PlayerInput.h"

#include "PlayerCamera.h"
#include "PlayerMove.h"

class Player : public Wreck::Entity 
{
	LINK_ENTITY_TO_CLASS(Player, "info_player_start")

public:
	void Spawn() override;
	void Update(float dt) override;
	void Render() override;
	void Unload() override;

private:
	PlayerInput m_PInput;

	PlayerCamera m_PCamera;
	PlayerMove m_PMove;
};