#include "PlayerCamera.h"

void PlayerCamera::Init(Wreck::Transform target)
{
	m_Transform.position = target.position;
	m_Transform.rotation = target.rotation;

	SetupCamera();
}

void PlayerCamera::SetupCamera()
{
	Camera3D& camera = Wreck::CameraManager::Get();
	camera.position = m_Transform.position;
	camera.target = { camera.position.x, camera.position.y, camera.position.z + 1.0f };
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;
}

void PlayerCamera::SyncCamera()
{
	Vector3 forward = {
		cosf(m_Pitch) * sinf(m_Yaw),
		sinf(m_Pitch),
		cosf(m_Pitch) * cosf(m_Yaw)
	};

	Camera3D& camera = Wreck::CameraManager::Get();
	camera.position = m_Transform.position;
	camera.target = camera.position + forward;
}

void PlayerCamera::UpdateRotation(const Vector2& look)
{
	m_Yaw -= look.x * m_Sensitivity;
	m_Pitch -= look.y * m_Sensitivity;

	// clamp pitch
	if (m_Pitch > 1.57f) m_Pitch = 1.57f;
	if (m_Pitch < -1.57f) m_Pitch = -1.57f;

	m_Transform.rotation.x = m_Pitch;
	m_Transform.rotation.y = m_Yaw;
}

void PlayerCamera::UpdatePosition(Wreck::Transform target)
{
	m_Transform.position = target.position;
}