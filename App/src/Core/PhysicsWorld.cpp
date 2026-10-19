#include "PhysicsWorld.h"
#include "rayMath.h"

namespace Wreck {

	void PhysicsWorld::RaycastRecursive(
		const std::vector<RuntimeBVHNode>& nodes,
		int nodeIndex,
		const Ray& ray,
		RayCastHit& bestHit
	) const
	{
		if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes.size()))
			return;

		const auto& node = nodes[nodeIndex];

		BoundingBox box = {
			{ 
				fminf(node.bounds.min.x, node.bounds.max.x),
				fminf(node.bounds.min.y, node.bounds.max.y),
				fminf(node.bounds.min.z, node.bounds.max.z)
			},
			{ 
				fmaxf(node.bounds.min.x, node.bounds.max.x),
				fmaxf(node.bounds.min.y, node.bounds.max.y),
				fmaxf(node.bounds.min.z, node.bounds.max.z)
			}
		};

		RayCollision boxCollision = GetRayCollisionBox(ray, box);
		if (!boxCollision.hit || boxCollision.distance >= bestHit.distance)
			return;

		if (node.IsLeaf())
		{
			for (const auto& tri : node.triangles)
			{
				RayCollision meshCollision = GetRayCollisionTriangle(ray, tri.v0, tri.v1, tri.v2);
				if (meshCollision.hit && meshCollision.distance < bestHit.distance)
				{
					bestHit.hit = true;
					bestHit.distance = meshCollision.distance;
					bestHit.point = meshCollision.point;
					bestHit.normal = meshCollision.normal;
				}
			}
		}
		else
		{
			RaycastRecursive(nodes, node.leftChildIndex, ray, bestHit);
			RaycastRecursive(nodes, node.rightChildIndex, ray, bestHit);
		}
	}

	inline bool CheckCollisionBoxTriangle(
		const BoundingBox& box,
		const Vector3& v0,
		const Vector3& v1,
		const Vector3& v2)
	{
		Vector3 triMin = {
			fminf(v0.x, fminf(v1.x, v2.x)),
			fminf(v0.y, fminf(v1.y, v2.y)),
			fminf(v0.z, fminf(v1.z, v2.z)),
		};

		Vector3 triMax = {
			fmaxf(v0.x, fmaxf(v1.x, v2.x)),
			fmaxf(v0.y, fmaxf(v1.y, v2.y)),
			fmaxf(v0.z, fmaxf(v1.z, v2.z)),
		};

		BoundingBox triBox = { triMin, triMax };
		if (!CheckCollisionBoxes(box, triBox))
			return false;

		// compute box center and extents
		Vector3 center = {
			(box.min.x + box.max.x) * 0.5f,
			(box.min.y + box.max.y) * 0.5f,
			(box.min.z + box.max.z) * 0.5f
		};

		Vector3 extents = {
			(box.max.x - box.min.x) * 0.5f,
			(box.max.y - box.min.y) * 0.5f,
			(box.max.z - box.min.z) * 0.5f
		};

		Vector3 tv0 = v0 - center;
		Vector3 tv1 = v1 - center;
		Vector3 tv2 = v2 - center;

		Vector3 f0 = tv1 - tv0;
		Vector3 f1 = tv2 - tv1;
		Vector3 f2 = tv0 - tv2;

		// test
		auto TestAxis = [&](const Vector3& axis, float minVal, float maxVal)
			{
				float p0 = Vector3DotProduct(tv0, axis);
				float p1 = Vector3DotProduct(tv1, axis);
				float p2 = Vector3DotProduct(tv2, axis);
				float r = extents.x * fabsf(Vector3DotProduct(Vector3{ 1, 0, 0 }, axis)) +
					extents.y * fabsf(Vector3DotProduct(Vector3{ 0, 1, 0 }, axis)) +
					extents.z * fabsf(Vector3DotProduct(Vector3{ 0, 0, 1 }, axis));

				float tMin = fminf(p0, fminf(p1, p2));
				float tMax = fmaxf(p0, fmaxf(p1, p2));
				return !(-r <= tMax && r >= tMin);
			};

		if (TestAxis({ 1, 0, 0 }, triMin.x, triMax.x)) return false;
		if (TestAxis({ 0, 1, 0 }, triMin.y, triMax.y)) return false;
		if (TestAxis({ 0, 0, 1 }, triMin.z, triMax.z)) return false;

		// triangle normal axis
		Vector3 normal = Vector3Normalize(Vector3CrossProduct(f0, f1));
		float triDist = Vector3DotProduct(normal, tv0);
		float rBox = extents.x * fabsf(normal.x) + extents.y * fabsf(normal.y) + extents.z * fabsf(normal.z);
		if (fabsf(triDist) > rBox) return false;

		return true; // overlap confirmed
	}

	void PhysicsWorld::AABBOverlapRecursive(
		const std::vector<RuntimeBVHNode>& nodes,
		int nodeIndex,
		const BoundingBox& queryBox,
		bool& hasOverlap
	) const
	{
		if (hasOverlap || nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes.size()))
			return;

		const auto& node = nodes[nodeIndex];

		BoundingBox nodeBox = {
			{
				fminf(node.bounds.min.x, node.bounds.max.x),
				fminf(node.bounds.min.y, node.bounds.max.y),
				fminf(node.bounds.min.z, node.bounds.max.z),
			},
			{
				fmaxf(node.bounds.min.x, node.bounds.max.x),
				fmaxf(node.bounds.min.y, node.bounds.max.y),
				fmaxf(node.bounds.min.z, node.bounds.max.z),
			}
		};

		if (!CheckCollisionBoxes(queryBox, nodeBox))
			return;

		if (node.IsLeaf())
		{
			for (const auto& tri : node.triangles)
			{
				if (CheckCollisionBoxTriangle(queryBox, tri.v0, tri.v1, tri.v2))
				{
					hasOverlap = true;
					return;
				}
			}
		}
		else
		{
			AABBOverlapRecursive(nodes, node.leftChildIndex, queryBox, hasOverlap);
			AABBOverlapRecursive(nodes, node.rightChildIndex, queryBox, hasOverlap);
		}
	}

	RayCastHit PhysicsWorld::Raycast(const MapLoader& map, const Ray& ray) const
	{
		RayCastHit bestHit;

		const auto& nodes = map.GetBVHNodes();
		if (nodes.empty()) return bestHit;

		RaycastRecursive(nodes, 0, ray, bestHit);

		return bestHit;
	}

	bool PhysicsWorld::CheckAABBOverlap(
		const MapLoader& map,
		const BoundingBox& queryBox) const
	{
		const auto& nodes = map.GetBVHNodes();
		if (nodes.empty()) return false;

		bool hasOverlap = false;
		AABBOverlapRecursive(nodes, 0, queryBox, hasOverlap);

		return hasOverlap;
	}
}