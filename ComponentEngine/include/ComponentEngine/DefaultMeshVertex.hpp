#pragma once

#include <glm/glm.hpp>

namespace ComponentEngine
{
	
	class MeshVertex
	{
	public:
		MeshVertex() {};
		MeshVertex(glm::vec3 position, glm::vec3 normal, glm::vec3 color, glm::vec2 uv, int matID) : pos(position), texCoord(uv), nrm(normal), color(color), matID(matID) {}

		glm::vec3 pos;
		glm::vec3 nrm;
		glm::vec3 color;
		glm::vec2 texCoord;
		int matID = 0;

	};
}