#include <ComponentEngine\Components\Transformation.hpp>

using namespace ComponentEngine;

glm::mat4 & Transformation::Get()
{
	return m_mat4;
}
void Transformation::Translate(glm::vec3 translation)
{
	m_mat4 = glm::translate(m_mat4, translation);
}

void ComponentEngine::Transformation::Rotate(glm::vec3 axis, float angle)
{
	m_mat4 = glm::rotate(m_mat4, angle, axis);
}
