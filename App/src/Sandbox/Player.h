#pragma once
#include "Wreck.h"

class Player : public Wreck::Entity 
{
	LINK_ENTITY_TO_CLASS(Player, "info_player_start")

public:
	void Spawn() override;
};