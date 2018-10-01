#pragma once

#include <glm/glm.hpp>

namespace ComponentEngine
{
	struct DefaultMeshVertex
	{
	public:
		DefaultMeshVertex(glm::vec3 position) : position(position) {}
		glm::vec3 position;
	};
}