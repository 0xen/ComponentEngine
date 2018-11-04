#pragma once

#include <glm/glm.hpp>

namespace ComponentEngine
{
	struct RenderStatus
	{
		RenderStatus(bool should_renderer) : should_renderer(should_renderer) {}
		bool should_renderer;
	};

	struct TransformationPtrRedirect
	{
		TransformationPtrRedirect(glm::mat4* mat_ptr) : mat_ptr(mat_ptr) {}
		glm::mat4* mat_ptr;
	};
}