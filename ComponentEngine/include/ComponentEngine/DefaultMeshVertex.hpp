#pragma once

#include <glm/glm.hpp>

namespace ComponentEngine
{
	struct DefaultMeshVertex
	{
	public:
		DefaultMeshVertex() {}
		DefaultMeshVertex(glm::vec4 position) : position(position) {}
		glm::vec4 position;
	};
}