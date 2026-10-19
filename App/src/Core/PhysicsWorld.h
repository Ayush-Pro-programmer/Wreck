#pragma once
#include <vector>

#include "raylib.h"
#include "MapLoader.h"

namespace Wreck {
	
	struct RayCastHit
	{
		bool hit = false;
		float distance = 1e30;
		Vector3 point = {};
		Vector3 normal = {};
	};

	class PhysicsWorld
	{
	public:
		PhysicsWorld() = default;
		~PhysicsWorld() = default;

		RayCastHit Raycast(const MapLoader& map, const Ray& ray) const;

		bool CheckAABBOverlap(const MapLoader& map, const BoundingBox& queryBox) const;

	private:
		void RaycastRecursive(
			const std::vector<RuntimeBVHNode>& nodes,
			int nodeIndex,
			const Ray& ray,
			RayCastHit& bestHit
		) const;

		void AABBOverlapRecursive(
			const std::vector<RuntimeBVHNode>& nodes,
			int nodeIndex,
			const BoundingBox& queryBox,
			bool& hasOverlap
		) const;
	};

}