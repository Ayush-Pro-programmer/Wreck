#include "Mesh.h"

namespace Wreck {

	void Mesh::AddVertex(const Vertex& vertex)
	{
		Vertices.push_back(vertex);
	}

	void Mesh::AddIndex(unsigned short index)
	{
		Indices.push_back(index);
	}

	void Mesh::Clear()
	{
		Vertices.clear();
		Indices.clear();
		TextureName.clear();
	}

	::Mesh Mesh::ToRaylibMesh() const
	{
		::Mesh rlMesh = { 0 };

		rlMesh.vertexCount = static_cast<int>(Vertices.size());
		rlMesh.triangleCount = static_cast<int>(Indices.size() / 3);

		if (rlMesh.vertexCount == 0 || Indices.empty())
			return rlMesh;

		// allocate memory
		rlMesh.vertices = (float*)MemAlloc(rlMesh.vertexCount * 3 * sizeof(float));
		rlMesh.normals  = (float*)MemAlloc(rlMesh.vertexCount * 3 * sizeof(float));
		rlMesh.texcoords = (float*)MemAlloc(rlMesh.vertexCount * 2 * sizeof(float));
		rlMesh.indices = (unsigned short*)MemAlloc(Indices.size() * sizeof(unsigned short));

		// populate buffers
		for (int i = 0; i < rlMesh.vertexCount; i++)
		{
			rlMesh.vertices[i * 3 + 0] = Vertices[i].position.x;
			rlMesh.vertices[i * 3 + 1] = Vertices[i].position.y;
			rlMesh.vertices[i * 3 + 2] = Vertices[i].position.z;

			rlMesh.normals[i * 3 + 0] = Vertices[i].normal.x;
			rlMesh.normals[i * 3 + 1] = Vertices[i].normal.y;
			rlMesh.normals[i * 3 + 2] = Vertices[i].normal.z;

			rlMesh.texcoords[i * 2 + 0] = Vertices[i].texCoords.x;
			rlMesh.texcoords[i * 2 + 1] = Vertices[i].texCoords.y;
		}

		std::memcpy(rlMesh.indices, Indices.data(), Indices.size() * sizeof(unsigned short));

		UploadMesh(&rlMesh, false);

		return rlMesh;
	}
}