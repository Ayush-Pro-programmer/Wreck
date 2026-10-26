#pragma once
#include "Wreck.h"

class PlayerCamera
{
public:
	void Init(Vector3 target);
	void SyncCamera();

	void UpdateRotation(const Vector2& look);
	void UpdatePosition(Vector3 target);

	float GetYaw() const { return m_Yaw; }
	float GetPitch() const { return m_Pitch; }
 
private:
	void SetupCamera();

private:
	Vector3 position;

	float m_Yaw = 0.0f;
	float m_Pitch = 0.0f;
	float m_Sensitivity = 0.003f;
};