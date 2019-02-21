#include <ComponentEngine\Components\Rigidbody.hpp>
#include <ComponentEngine\pugixml.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\PhysicsWorld.hpp>

#include <EnteeZ\EnteeZ.hpp>

ComponentEngine::Rigidbody::Rigidbody(enteez::Entity * entity)
{
	m_entity = entity;
	//create a dynamic rigidbody

	//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
	//m_colShape = new btSphereShape(btScalar(1.));


	m_colShape = new btBoxShape(btVector3(btScalar(0.5f), btScalar(0.5f), btScalar(0.5f)));


	//collisionShapes.push_back(colShape);

	/// Create Dynamic Objects
	btTransform startTransform;
	startTransform.setIdentity();

	btScalar mass(1.f);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 2, 0);
	if (isDynamic)
		m_colShape->calculateLocalInertia(mass, localInertia);

	if (entity->HasComponent<Transformation>())
	{
		Transformation& trans = entity->GetComponent<Transformation>();
		startTransform.setFromOpenGLMatrix((btScalar*)&trans.GetLocalMat4());
		startTransform.setOrigin(btVector3(trans.GetWorldX(), trans.GetWorldY(), trans.GetWorldZ()));
	}

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	m_myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_myMotionState, m_colShape, localInertia);
	m_body = new btRigidBody(rbInfo);
	Engine::Singlton()->GetPhysicsWorld()->GetDynamicsWorld()->addRigidBody(m_body);

}

ComponentEngine::Rigidbody::~Rigidbody()
{
	delete m_body;
	delete m_myMotionState;
	delete m_colShape;
}

void ComponentEngine::Rigidbody::Update(float frame_time)
{
	Engine* engine = Engine::Singlton();
	btTransform trans;

	m_body->getMotionState()->getWorldTransform(trans);

	//printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
	glm::mat4 matrix;
	trans.getOpenGLMatrix((btScalar*)&matrix);

	m_entity->GetComponent<Transformation>().SetLocalMat4(matrix);
}

void ComponentEngine::Rigidbody::Display()
{

}

void ComponentEngine::Rigidbody::EntityHookDefault(enteez::Entity& entity)
{
	enteez::ComponentWrapper<Rigidbody>* mesh = entity.AddComponent<Rigidbody>(&entity);
	mesh->SetName("Rigidbody");

}

void ComponentEngine::Rigidbody::EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data)
{
	enteez::ComponentWrapper<Rigidbody>* mesh = entity.AddComponent<Rigidbody>(&entity);
	mesh->SetName("Rigidbody");
}
