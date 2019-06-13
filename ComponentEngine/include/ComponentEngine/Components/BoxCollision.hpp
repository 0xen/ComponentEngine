#pragma once

#include <ComponentEngine\Components\ICollisionShape.hpp>
#include <glm/glm.hpp>


namespace pugi
{
	class xml_node;
}
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
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);


	private:
		void Rebuild();
		void CreateCollision();
		glm::vec3 m_shape;
	};
}