#pragma once

#include <glm/glm.hpp>

namespace ComponentEngine
{
	struct Camera
	{
		glm::mat4 view;
		glm::mat4 projection;
	};
}