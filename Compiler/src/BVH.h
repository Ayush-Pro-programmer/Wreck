#pragma once

#include <vector>
#include <algorithm>

#include "utils/Math.h"
#include "Brush.h"

struct AABB
{
	Vector3 min = { 1e30f, 1e30f, 1e30f };
	Vector3 max = { -1e30f, -1e30f, -1e30f };

	// expand the box the encompass a point
	void Grow(const Vector3& p)
	{
		min.x = fminf(min.x, p.x);
		min.y = fminf(min.y, p.y);
		min.z = fminf(min.z, p.z);
		max.x = fmaxf(max.x, p.x);
		max.y = fmaxf(max.y, p.y);
		max.z = fmaxf(max.z, p.z);
	}

	// expand the box to encompass another box
	void Grow(const AABB& other)
	{
		if (other.min.x > 1e29f) return; // ignore unintialized boxes
		Grow(other.min);
		Grow(other.max);
	}

	// calculate the center of point of the box
	Vector3 Center() const
	{
		return (min + max) * 0.5f;
	}

	// check if this box intersects with other
	bool Intersects(const AABB& other) const
	{
		return 
			(min.x <= other.max.x && max.x >= other.min.x) &&
			(min.y <= other.max.y && max.y >= other.min.y) &&
			(min.z <= other.max.z && max.z >= other.min.z);
	}
};

struct BVHNode
{
	AABB bounds;

	int leftChildIndex = -1;
	int rightChildIndex = -1;

	std::vector<size_t> brushIndices;

	bool IsLeaf() const
	{
		return leftChildIndex == -1 && rightChildIndex == -1;
	}
};

struct BVHWorldMap
{
	std::vector<BVHNode> nodes;
};

namespace BVHBuilder 
{
	inline Vector3 ConvertToRaylib(const Vector3& v)
	{
		return { -v.x, v.z, v.y };
	}

	inline AABB GetBrushBounds(const BrushData& brush)
	{
		AABB box;

		for (const auto& plane : brush.planes)
		{
			box.Grow(ConvertToRaylib(plane.p1));
			box.Grow(ConvertToRaylib(plane.p2));
			box.Grow(ConvertToRaylib(plane.p3));
		}

		// verify box integrity
		if (box.min.x > box.max.x || box.min.x > 1e29f)
		{
			box.min = { 0.0f, 0.0f, 0.0f };
			box.max = { 0.0f, 0.0f, 0.0f };
		}

		return box;
	}

	// recursive function to partition brushes and build tree nodes
	inline int BuildRecursive(
		const std::vector<BrushData>& brushes,
		const std::vector<AABB>& brushBounds,
		std::vector<size_t>& indices,
		std::vector<BVHNode>& nodes)
	{
		BVHNode node;

		for (size_t idx : indices)
			node.bounds.Grow(brushBounds[idx]);

		// base case: if 4 or fewer brushes, make this a leaf node
		if (indices.size() <= 4)
		{
			node.brushIndices = indices;
			node.leftChildIndex = -1;
			node.rightChildIndex = -1;
			int nodeIndex = static_cast<int>(nodes.size());
			nodes.push_back(node);
			return nodeIndex;
		}

		// find the longest axis of our bounding box to split along
		Vector3 extent = node.bounds.max - node.bounds.min;
		int axis = 0;
		if (extent.y > extent.x && extent.y > extent.z) axis = 1;
		else if (extent.z > extent.x && extent.z > extent.y) axis = 2;

		// sort indices
		std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b)
			{
				Vector3 centerA = brushBounds[a].Center();
				Vector3 centerB = brushBounds[b].Center();

				float valA = (axis == 0) ? centerA.x : (axis == 1) ? centerA.y : centerA.z;
				float valB = (axis == 0) ? centerB.x : (axis == 1) ? centerB.y : centerB.z;

				if (valA != valB) return valA < valB;
				return a < b;
			});

		// split the indices into 2 halves
		size_t mid = indices.size() / 2;
		std::vector<size_t> leftIndices(indices.begin(), indices.begin() + mid);
		std::vector<size_t> rightIndices(indices.begin() + mid, indices.end());

		int nodeIndex = static_cast<int>(nodes.size());
		nodes.push_back(node);

		// resursively build children
		int leftChild = BuildRecursive(brushes, brushBounds, leftIndices, nodes);
		int rightChild = BuildRecursive(brushes, brushBounds, rightIndices, nodes);

		// update node children indices
		nodes[nodeIndex].leftChildIndex = leftChild;
		nodes[nodeIndex].rightChildIndex = rightChild;

		return nodeIndex;
	}

	inline BVHWorldMap Build(std::vector<BrushData>& brushes)
	{
		BVHWorldMap bvhMap;
		if (brushes.empty()) return bvhMap;

		bvhMap.nodes.reserve(brushes.size() * 2);

		std::vector<AABB> brushBounds(brushes.size());
		for (size_t i = 0; i < brushes.size(); i++)
			brushBounds[i] = GetBrushBounds(brushes[i]);

		std::vector<size_t> indices(brushes.size());
		for (size_t i = 0; i < brushes.size(); i++) indices[i] = i;

		BuildRecursive(brushes, brushBounds, indices, bvhMap.nodes);
		std::cout << " - BVH Build: Generated " << bvhMap.nodes.size() << " total tree nodes.\n";
		return bvhMap;
	}
}