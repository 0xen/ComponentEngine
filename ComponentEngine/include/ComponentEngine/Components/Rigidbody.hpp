#pragma once

#include <glm/glm.hpp>
#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\TransferBuffers.hpp>
#include <ComponentEngine\Components\MsgRecive.hpp>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
#include <ComponentEngine\Components\ICollisionShape.hpp>
#include <ComponentEngine\Components\IO.hpp>
#include <btBulletDynamicsCommon.h>

#include <vector>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
}

namespace Renderer
{
	class IUniformBuffer;
}

using namespace Renderer;

namespace ComponentEngine
{

	class Transformation;
	class ICollisionShape;

	class Rigidbody : 
		public MsgRecive<TransformationChange>, public MsgRecive<OnComponentEnter<ICollisionShape>>,
		public MsgRecive<CollisionRecording>, public MsgRecive<CollisionEvent>,
		public MsgRecive<OnComponentChange<ICollisionShape>>, public MsgRecive<OnComponentExit<ICollisionShape>>, 
		public MsgRecive<OnComponentExit<Rigidbody>>,
		public Logic, public UI, public IO
	{
	public:
		Rigidbody(enteez::Entity* entity);
		~Rigidbody();

		virtual void Update(float frame_time);
		virtual void Display();

		void SetMass(float mass);

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);


		virtual void Load(std::ifstream& in);
		virtual void Save(std::ofstream& out);
		virtual unsigned int PayloadSize();
		virtual bool DynamiclySized();

		virtual void ReciveMessage(enteez::Entity* sender, TransformationChange& message);
		virtual void ReciveMessage(enteez::Entity* sender, OnComponentEnter<ICollisionShape>& message);
		virtual void ReciveMessage(enteez::Entity* sender, OnComponentChange<ICollisionShape>& message);
		virtual void ReciveMessage(enteez::Entity* sender, OnComponentExit<ICollisionShape>& message);
		// to be called by other rigidbodys that know its colliding with
		virtual void ReciveMessage(enteez::Entity* sender, OnComponentExit<Rigidbody>& message); 
		virtual void ReciveMessage(enteez::Entity* sender, CollisionRecording& message);
		virtual void ReciveMessage(enteez::Entity* sender, CollisionEvent& message);
		
	private:
		void Rebuild();

		void AddRigidbody(ICollisionShape* shape);
		void RemoveRigidbody();

		enteez::Entity* m_entity = nullptr;

		btDefaultMotionState* m_myMotionState;
		btRigidBody* m_body;

		// PAYLOAD
		btScalar m_friction;
		btScalar m_RollingFriction;
		btScalar m_SpinningFriction;
		btVector3 m_AnisotropicFriction;
		btVector3 m_localInertia;
		float m_gravity = -10.0f;
		bool m_useGravity = true;
		bool m_useCollision = true;
		bool m_recordingCollisions = false;
		btScalar m_mass;
		// PAYLOAD END

		std::vector<enteez::Entity*>* m_collisionFrameRecording;
		std::vector<enteez::Entity*>* m_collisionStaging;
		std::vector<enteez::Entity*>* m_currentCollisions; 

		btCollisionShape* m_localShape = nullptr;
	};

}