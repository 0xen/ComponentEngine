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
	class SphereCollision : public ICollisionShape
	{
	public:
		SphereCollision(enteez::Entity* entity);
		~SphereCollision();

		virtual void Display();

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);


	private:
		void Rebuild();
		void CreateCollision();
		float m_rad; 
	};
}