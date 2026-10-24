#pragma once
#include "Wreck.h"

class FuncDoor : public Wreck::Entity
{
	LINK_ENTITY_TO_CLASS(FuncDoor, "func_door");

private:
	Vector3 m_ClosedPos = { 0 };
	Vector3 m_OpenPos = { 0 };
	Vector3 m_MoveDir = { 0, 1, 0 };
	float m_Distance = 64.0f;
	float m_Speed = 200.0f;

	enum State 
	{
		Closed,
		Closing,
		Open,
		Opening
	};
	State m_State = Closed;
	float m_MoveProgress = 0.0f;

public:

	void Spawn() override
	{
		m_ClosedPos = position;

		m_Distance = GetFloat("distance", 64.0f) * Wreck::WORLD_SCALE;
		m_Speed = GetFloat("speed", 15.0f);

		// TODO: allow setting a custom slide dir in trenchbroom
		m_MoveDir = { 0.0f, 1.0f, 0.0f };

		m_OpenPos = {
			m_ClosedPos.x + (m_MoveDir.x * m_Distance),
			m_ClosedPos.y + (m_MoveDir.y * m_Distance),
			m_ClosedPos.z + (m_MoveDir.z * m_Distance),
		};

		TraceLog(LOG_INFO, "[FuncDoor] Initialized at position (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
	}

	void Use(Entity* activator) override
	{
		if (m_State == Closed)
		{
			m_State = Opening;
			TraceLog(LOG_INFO, "[FuncDoor] Opening...");
		} 
		else if (m_State == Open)
		{
			m_State = Closing;
			TraceLog(LOG_INFO, "[FuncDoor] Closing...");
		}
	}

	void Update(float dt) override
	{
		if (m_State == Opening)
		{
			m_MoveProgress += (m_Speed * dt)
				/ (m_Distance > 0 ? m_Distance : 1.0f);

			if (m_MoveProgress >= 1.0f)
			{
				m_MoveProgress = 1.0f;
				m_State = Open;
			}

			position = Vector3Lerp(m_ClosedPos, m_OpenPos, m_MoveProgress);
		}
		else if (m_State == Closing)
		{
			m_MoveProgress -= (m_Speed * dt)
				/ (m_Distance > 0 ? m_Distance : 1.0f);

			if (m_MoveProgress <= 0.0f)
			{
				m_MoveProgress = 0.0f;
				m_State = Closed;
			}

			position = Vector3Lerp(m_ClosedPos, m_OpenPos, m_MoveProgress);
		}
	}
};