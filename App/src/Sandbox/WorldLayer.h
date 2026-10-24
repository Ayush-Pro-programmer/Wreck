#pragma once
#include <vector>
#include <memory>

#include "Wreck.h"

class WorldLayer : public Wreck::Layer
{
public:
	WorldLayer();
	~WorldLayer() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float dt) override;
	void OnRender() override;
};

