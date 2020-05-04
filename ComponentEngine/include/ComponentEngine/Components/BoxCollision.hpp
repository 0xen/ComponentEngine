#pragma once

#include <ComponentEngine\Components\ICollisionShape.hpp>
#include <ComponentEngine\Components\IO.hpp>
#include <glm/glm.hpp>

namespace Renderer
{
	class IUniformBuffer;
}

using namespace Renderer;

namespace ComponentEngine
{
	class BoxCollision : public ICollisionShape, public IO
	{
	public:
		BoxCollision(enteez::Entity* entity);
		~BoxCollision();

		virtual void Display();

		virtual void Load(pugi::xml_node& node);
		virtual void Save(pugi::xml_node& node);

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);


	private:
		void Rebuild();
		void CreateCollision();
		glm::vec3 m_shape;
	};
}