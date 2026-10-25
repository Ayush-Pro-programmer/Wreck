#include "PhysicsSolver.h"

#include "WreckConst.h"
#include "MapManager.h"

namespace Wreck {

	PhysicsSolver::PhysicsSolver(PhysicsWorld& physicsWorld)
		: m_PhysicsWorld(physicsWorld) {}

	void PhysicsSolver::ApplyGravity(Vector3& velocity, float dt)
	{
		velocity.y -= GRAVITY * dt;
	}

	MoveResult PhysicsSolver::Move(BoundingBox& box, Vector3& velocity, float dt)
	{
		MoveResult result;
		const auto& activeMap = MapManager::Get().GetActiveMap();

		// ----- X-AXIS MOVEMENT -----
		float displacementX = velocity.x * dt;
		if (displacementX != 0.0f)
		{
			// try moving the box along X
			box.min.x += displacementX;
			box.max.x += displacementX;

			// test if this new position overlap world geometry
			if (m_PhysicsWorld.CheckAABBOverlap(activeMap, box))
			{
				// collision!
				box.min.x -= displacementX;
				box.max.x -= displacementX;
				velocity.x = 0.0f;

				result.hitWall = true;
			}
		}

		// ----- Z-AXIS MOVEMENT -----
		float displacementZ = velocity.z * dt;
		if (displacementZ != 0.0f)
		{
			box.min.z += displacementZ;
			box.max.z += displacementZ;

			if (m_PhysicsWorld.CheckAABBOverlap(activeMap, box))
			{
				box.min.z -= displacementZ;
				box.max.z -= displacementZ;
				velocity.z = 0;

				result.hitWall = true;
			}
		}

		// ----- Y-AXIS MOVEMENT -----
		float displacementY = velocity.y * dt;
		if (displacementY != 0.0f)
		{
			box.min.y += displacementY;
			box.max.y += displacementY;

			if (m_PhysicsWorld.CheckAABBOverlap(activeMap, box))
			{
				box.min.y -= displacementY;
				box.max.y -= displacementY;

				// if we were falling down and hit something, we hit a floor!
				// if we were moving up and hit something, we hit a ceiling!
				if (velocity.y < 0.0f)
					result.isGrounded = true;
				else
					result.hitCeiling = true;

				velocity.y = 0;
			}
		}

		// ----- GROUND CHECK -----
		if (!result.isGrounded)
		{
			BoundingBox groundProbe = box;
			groundProbe.min.y -= 0.15f; // offset
			groundProbe.max.y = box.min.y;

			if (m_PhysicsWorld.CheckAABBOverlap(activeMap, groundProbe))
				result.isGrounded = true;
		}

		return result;
	}
}