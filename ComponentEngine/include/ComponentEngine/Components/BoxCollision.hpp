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

		virtual void Load(std::ifstream& in);
		virtual void Save(std::ofstream& out);
		virtual unsigned int PayloadSize();
		virtual bool DynamiclySized();

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);


	private:
		void Rebuild();
		void CreateCollision();
		glm::vec3 m_shape;
	};
}