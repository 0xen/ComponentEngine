#include <ComponentEngine\Components\Transformation.hpp>

using namespace ComponentEngine;

glm::mat4 & Transformation::Get()
{
	if (m_parent == nullptr)return *m_mat4;
	return *m_mat4 * m_parent->Get();
}

void Transformation::Translate(glm::vec3 translation)
{
	*m_mat4 = glm::translate(*m_mat4, translation);
}

void Transformation::Scale(glm::vec3 scale)
{
	*m_mat4 = glm::scale(*m_mat4, scale);
}

void ComponentEngine::Transformation::Rotate(glm::vec3 axis, float angle)
{
	*m_mat4 = glm::rotate(*m_mat4, angle, axis);
}

void ComponentEngine::Transformation::SetParent(Transformation* parent)
{
	m_parent = parent;
}

Transformation* ComponentEngine::Transformation::GetParent()
{
	return m_parent;
}