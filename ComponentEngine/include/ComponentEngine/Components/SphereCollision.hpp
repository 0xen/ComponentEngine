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
	class SphereCollision : public ICollisionShape , public IO
	{
	public:
		SphereCollision(enteez::Entity* entity);
		~SphereCollision();

		virtual void Display();

		virtual void Load(pugi::xml_node& node);
		virtual void Save(pugi::xml_node& node);

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);


	private:
		void Rebuild();
		void CreateCollision();
		float m_rad; 
	};
}