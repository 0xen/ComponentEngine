#pragma once

#include <ComponentEngine\Components\ICollisionShape.hpp>
#include <glm/glm.hpp>

namespace Renderer
{
	class IUniformBuffer;
}

using namespace Renderer;

namespace ComponentEngine
{
	class BoxCollision : public ICollisionShape
	{
	public:
		BoxCollision(enteez::Entity* entity);
		~BoxCollision();

		virtual void Display();

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);


	private:
		void Rebuild();
		void CreateCollision();
		glm::vec3 m_shape;
	};
}