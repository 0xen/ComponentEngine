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


		virtual void Load(std::ifstream& in);
		virtual void Save(std::ofstream& out);
		virtual unsigned int PayloadSize();
		virtual bool DynamiclySized();

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);


	private:
		void Rebuild();
		void CreateCollision();
		float m_rad; 
	};
}