#include <ComponentEngine\PhysicsWorld.hpp>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
#include <EnteeZ\Entity.hpp>
#include <ComponentEngine/Engine.hpp>
#include <iostream>

using namespace ComponentEngine;

ComponentEngine::PhysicsWorld::PhysicsWorld(Engine * engine)
{
	m_engine = engine;

	m_collisionConfiguration = new btDefaultCollisionConfiguration();

	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

	m_overlappingPairCache = new btDbvtBroadphase();

	m_solver = new btSequentialImpulseConstraintSolver;

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);
	m_dynamicsWorld->setGravity(btVector3(0, -10, 0));

}

ComponentEngine::PhysicsWorld::~PhysicsWorld()
{
	for (int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}
	
	delete m_dynamicsWorld;
	delete m_solver;
	delete m_overlappingPairCache;
	delete m_dispatcher;
	delete m_collisionConfiguration;
}

void ComponentEngine::PhysicsWorld::Update(float update_time)
{
	m_engine->GetLogicMutex().lock();
	m_physics_lock.lock();

	m_dynamicsWorld->stepSimulation(update_time, 10);


	btCollisionObjectArray& collisionObjects = m_dynamicsWorld->getCollisionObjectArray();
	for (int i = 0; i < collisionObjects.size(); i++)
	{
		enteez::Entity* entity = static_cast<enteez::Entity*>(collisionObjects[i]->getCollisionShape()->getUserPointer());
		Send(entity, CollisionRecording{ Begin });
	}

	int numManifolds = m_dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold = m_dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
		btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());


		enteez::Entity* entity = static_cast<enteez::Entity*>(obA->getCollisionShape()->getUserPointer());
		enteez::Entity* entity2 = static_cast<enteez::Entity*>(obB->getCollisionShape()->getUserPointer());

		Send(entity, CollisionEvent{ entity2 });
		Send(entity2, CollisionEvent{ entity });
	}

	for (int i = 0; i < collisionObjects.size(); i++)
	{
		enteez::Entity* entity = static_cast<enteez::Entity*>(collisionObjects[i]->getCollisionShape()->getUserPointer());
		Send(entity, CollisionRecording{ End });
	}

	m_physics_lock.unlock();
	m_engine->GetLogicMutex().unlock();
}

void ComponentEngine::PhysicsWorld::SetGravity(btVector3 vec3)
{
	m_dynamicsWorld->setGravity(vec3);
}

btDiscreteDynamicsWorld * ComponentEngine::PhysicsWorld::GetDynamicsWorld()
{
	return m_dynamicsWorld;
}
