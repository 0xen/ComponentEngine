#pragma once

#include <glm/glm.hpp>

namespace ComponentEngine
{
	struct DefaultMeshVertex
	{
	public:
		DefaultMeshVertex() {}
		DefaultMeshVertex(glm::vec4 position, glm::vec4 color) : position(position), color(color) {}
		glm::vec4 position;
		glm::vec4 color;
	};
}