#pragma once

#include <btBulletDynamicsCommon.h>

namespace ComponentEngine
{
	class Engine;

	class PhysicsWorld
	{
	public:
		PhysicsWorld(Engine* engine);
		~PhysicsWorld();
		void Update(float update_time);
		void SetGravity(btVector3 vec3);
		btDiscreteDynamicsWorld* GetDynamicsWorld();
	private:
		Engine * m_engine;

		btDefaultCollisionConfiguration* m_collisionConfiguration;
		btCollisionDispatcher* m_dispatcher;
		btBroadphaseInterface* m_overlappingPairCache;
		btSequentialImpulseConstraintSolver* m_solver;
		btDiscreteDynamicsWorld* m_dynamicsWorld;
	};

}