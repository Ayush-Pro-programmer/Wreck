#pragma once
#include "raylib.h"
#include "PhysicsWorld.h"

namespace Wreck {

	struct MoveResult
	{
		bool hitWall = false;
		bool hitCeiling = false;
		bool isGrounded = false;
	};

	class PhysicsSolver
	{
	public:
		PhysicsSolver(PhysicsWorld& physicsWorld);
		~PhysicsSolver() = default;

		void ApplyGravity(Vector3& velocity, float dt);
		MoveResult Move(BoundingBox& box, Vector3& velocity, float dt);

	private:
		PhysicsWorld& m_PhysicsWorld;
	};

}