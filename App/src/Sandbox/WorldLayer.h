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

private:
	float worldScale = 0.05f;

	Camera3D m_Camera = { 0 };
	Wreck::MapLoader m_TestMap;
	Wreck::PhysicsWorld physics;

	std::vector<std::unique_ptr<Wreck::Entity>> m_Entities;
};

