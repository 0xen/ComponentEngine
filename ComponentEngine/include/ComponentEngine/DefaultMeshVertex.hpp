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
	
	class MeshVertex
	{
	public:
		MeshVertex() {};
		MeshVertex(glm::vec3 position, glm::vec2 uv, glm::vec3 normal, glm::vec3 color) : position(position), uv(uv), normal(normal), color(color) {}
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 color;
	};
}