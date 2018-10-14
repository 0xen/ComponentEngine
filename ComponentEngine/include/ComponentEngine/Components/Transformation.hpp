#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace ComponentEngine
{
	class Transformation
	{
	public:
		Transformation() { m_mat4 = glm::mat4(1.0f); }
		void Translate(glm::vec3 translation);
		void Scale(glm::vec3 scale);
		void Rotate(glm::vec3 axis, float angle);
		void SetParent(Transformation* parent);
		glm::mat4& Get();
		Transformation* GetParent();
	private:
		glm::mat4 m_mat4;
		Transformation* m_parent;
	};
}