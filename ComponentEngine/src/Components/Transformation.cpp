#include <ComponentEngine\Components\Transformation.hpp>

using namespace ComponentEngine;

glm::mat4 & ComponentEngine::Transformation::Get()
{
	return m_mat4;
}
