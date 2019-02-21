#pragma once

#include <glm/glm.hpp>
#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\TransferBuffers.hpp>
#include <btBulletDynamicsCommon.h>

#include <vector>

namespace enteez
{
	class Entity;
}

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

	class Transformation;

	class Rigidbody : public Logic, public UI
	{
	public:
		Rigidbody(enteez::Entity* entity);
		~Rigidbody();

		virtual void Update(float frame_time);
		virtual void Display();

		static void EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);

		
	private:

		enteez::Entity* m_entity = nullptr;

		btCollisionShape* m_colShape;

		btDefaultMotionState* m_myMotionState;
		btRigidBody* m_body; 
	};

}