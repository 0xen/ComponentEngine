#include <ComponentEngine\PhysicsWorld.hpp>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
#include <EnteeZ\Entity.hpp>
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


	/*{
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

		//collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -55, 0));

		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
	}*/


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

		//entity->SetName("");



		//std::cout << "test" << std::endl;



		//... here you can check for obA´s and obB´s user pointer again to see if the collision is alien and bullet and in that case initiate deletion.
	}

	for (int i = 0; i < collisionObjects.size(); i++)
	{
		enteez::Entity* entity = static_cast<enteez::Entity*>(collisionObjects[i]->getCollisionShape()->getUserPointer());
		Send(entity, CollisionRecording{ End });
	}


	//print positions of all objects
	/*for (int j = m_dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[j];
		btRigidBody* body = btRigidBody::upcast(obj);
		btTransform trans;
		if (body && body->getMotionState())
		{
			body->getMotionState()->getWorldTransform(trans);
		}
		else
		{
			trans = obj->getWorldTransform();
		}
		//printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
	}*/

	m_physics_lock.unlock();
}

void ComponentEngine::PhysicsWorld::SetGravity(btVector3 vec3)
{
	m_dynamicsWorld->setGravity(vec3);
}

btDiscreteDynamicsWorld * ComponentEngine::PhysicsWorld::GetDynamicsWorld()
{
	return m_dynamicsWorld;
}
