#pragma once
#include "Wreck.h"

#include <vector>

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
	Camera3D m_Camera = { 0 };
	
	std::vector<Wreck::Mesh> m_MapMeshes;
	Wreck::MapRenderer m_MapRenderer;
};

//FIXME: Add scope based object creation