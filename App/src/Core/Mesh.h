#pragma once

#include <vector>
#include <string>
#include "raylib.h"

namespace Wreck {
	
	struct Vertex
	{
		Vector3 position;
		Vector3 normal;
		Vector2 texCoords;
	};

	class Mesh
	{
	public:
		Mesh() = default;
		~Mesh() = default;

		void AddVertex(const Vertex& vertex);
		void AddIndex(unsigned short index);

		void Clear();

		::Mesh ToRaylibMesh() const;

		std::vector<Vertex> Vertices;
		std::vector<unsigned short> Indices;
		std::string TextureName;
	};

}