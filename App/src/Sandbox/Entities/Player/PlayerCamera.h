#pragma once
#include "Wreck.h"

class PlayerCamera
{
public:
	void Init(Wreck::Transform target);
	void SyncCamera();

	void UpdateRotation(const Vector2& look);
	void UpdatePosition(Wreck::Transform target);

private:
	void SetupCamera();

private:
	Wreck::Transform m_Transform = {};

	float m_Yaw = 0.0f;
	float m_Pitch = 0.0f;
	float m_Sensitivity = 0.003f;
};