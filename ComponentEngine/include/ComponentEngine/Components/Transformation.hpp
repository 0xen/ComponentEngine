#pragma once

#include <glm/glm.hpp>

namespace ComponentEngine
{
	class Transformation
	{
	public:
		Transformation() { m_mat4 = glm::mat4(1.0f); }
		glm::mat4& Get();
	private:
		glm::mat4 m_mat4;
	};
}