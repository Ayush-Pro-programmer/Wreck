#pragma once
#include "Wreck.h"

class FuncButton : public Wreck::Entity
{
	LINK_ENTITY_TO_CLASS(FuncButton, "func_button");

public:
	void Spawn() override
	{
		if (targetEntity)
			TraceLog(LOG_INFO, "[FuncButton] Linked successfully to target!");
		else if (!target.empty())
			TraceLog(LOG_WARNING, "[FuncButton] Target '%s' couldn't be found!", target.c_str());
	}

	void Use(Entity* activator) override 
	{
		TraceLog(LOG_INFO, "[FuncButton] Pressed!");

		if (targetEntity)
			targetEntity->Use(this);
	}
};