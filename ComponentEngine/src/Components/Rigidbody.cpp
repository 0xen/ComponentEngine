#include <ComponentEngine\Components\Rigidbody.hpp>
#include <ComponentEngine\pugixml.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\PhysicsWorld.hpp>
#include <ComponentEngine\Components\BoxCollision.hpp>

#include <EnteeZ\EnteeZ.hpp>

ComponentEngine::Rigidbody::Rigidbody(enteez::Entity * entity)
{
	m_entity = entity;

	m_mass = btScalar(1.f);
	m_localInertia = btVector3(0, 0, 0);

	m_friction = btScalar(0.0f);
	m_RollingFriction = btScalar(0.0f);
	m_SpinningFriction = btScalar(0.0f);
	m_AnisotropicFriction = btVector3(0, 0, 0);

	m_collisionFrameRecording = new std::vector<enteez::Entity*>();
	m_collisionStaging = new std::vector<enteez::Entity*>();
	m_currentCollisions = new std::vector<enteez::Entity*>();

	ICollisionShape* shape = nullptr;

	entity->ForEach<ICollisionShape>([&](enteez::Entity* entity, ICollisionShape& found_shape)
	{
		shape = &found_shape;
	});

	AddRigidbody(shape);
	

}

ComponentEngine::Rigidbody::~Rigidbody()
{
	Engine* engine = Engine::Singlton();
	engine->GetPhysicsWorld()->m_physics_lock.lock();
	RemoveRigidbody();

	for (int i = 0 ; i < m_currentCollisions->size(); i ++)
	{
		Send(m_currentCollisions->at(i), m_entity, OnComponentExit<Rigidbody>(this));
	}


	delete m_collisionFrameRecording;
	delete m_currentCollisions;
	engine->GetPhysicsWorld()->m_physics_lock.unlock();
}

void ComponentEngine::Rigidbody::Update(float frame_time)
{

	Engine* engine = Engine::Singlton();
	engine->GetPhysicsWorld()->m_physics_lock.lock();
	btTransform trans;

	m_body->getMotionState()->getWorldTransform(trans);

	glm::mat4 matrix;
	trans.getOpenGLMatrix((btScalar*)&matrix);


	Transformation& transformation = m_entity->GetComponent<Transformation>();

	if (transformation.GetParent() != nullptr)
	{
		glm::mat4 parentWorld = transformation.GetParent()->GetComponent<Transformation>().Get();
		transformation.SetWorldMat4(matrix);
	}
	else
	{
		transformation.SetWorldMat4(matrix);
	}



	engine->GetPhysicsWorld()->m_physics_lock.unlock();
}

void ComponentEngine::Rigidbody::Display()
{
	bool Rebuild = false;
	{ // Gravity
		ImGui::Text("Use Gravity");
		if (ImGui::Checkbox("##UseGravity", &m_useGravity))
		{
			Rebuild = true;
		}
	}
	{ // Collision
		ImGui::Text("Use Collision");
		if (ImGui::Checkbox("##UseCollision", &m_useCollision))
		{
			Rebuild = true;
		}
	}
	{ // Mass
		ImGui::Text("Mass");
		if (ImGui::InputFloat("##Rigidbody_mass", &m_mass, 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			Rebuild = true;
		}
	}
	{ // Friction
		ImGui::Text("Friction");
		if (ImGui::InputFloat("##Rigidbody_friction", &m_friction, 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			Rebuild = true;
		}
	}
	{ // Rolling Friction
		ImGui::Text("Rolling Friction");
		if (ImGui::InputFloat("##Rigidbody_rollingfriction", &m_RollingFriction, 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			Rebuild = true;
		}
	}
	{ // Spinning Friction
		ImGui::Text("Spinning Friction");
		if (ImGui::InputFloat("##Rigidbody_Spinningfriction", &m_SpinningFriction, 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			Rebuild = true;
		}
	}
	{ // Anisotropic Friction
		ImGui::Text("Anisotropic Friction");
		if (ImGui::InputFloat3("##Rigidbody_Anisotropicfriction", (float*)&m_AnisotropicFriction, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			Rebuild = true;
		}
	}

	if (Rebuild)
	{
		ICollisionShape* shape = nullptr;
		m_entity->ForEach<ICollisionShape>([&](enteez::Entity* entity, ICollisionShape& found_shape)
		{
			shape = &found_shape;
		});

		RemoveRigidbody();
		AddRigidbody(shape);
	}
}

void ComponentEngine::Rigidbody::SetMass(float mass)
{
	m_mass = mass;
	ICollisionShape* shape = nullptr;
	m_entity->ForEach<ICollisionShape>([&](enteez::Entity * entity, ICollisionShape & found_shape)
	{
		shape = &found_shape;
	});

	RemoveRigidbody();
	AddRigidbody(shape);
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

	Rigidbody& rigidbody = mesh->Get();



	rigidbody.m_useGravity = component_data.child("UseGravity").attribute("value").as_bool(true);
	rigidbody.m_useCollision = component_data.child("UseCollision").attribute("value").as_bool(true);

	rigidbody.m_mass = component_data.child("Mass").attribute("value").as_float(1.0f);

	rigidbody.m_friction = component_data.child("Friction").attribute("value").as_float(0.0f);
	rigidbody.m_RollingFriction = component_data.child("RollingFriction").attribute("value").as_float(0.0f);
	rigidbody.m_SpinningFriction = component_data.child("SpinningFriction").attribute("value").as_float(0.0f);
	rigidbody.m_AnisotropicFriction = btVector3(
		component_data.child("AnisotropicFriction").attribute("x").as_float(0.0f),
		component_data.child("AnisotropicFriction").attribute("y").as_float(0.0f),
		component_data.child("AnisotropicFriction").attribute("z").as_float(0.0f)
	);

	rigidbody.Rebuild();



	pugi::xml_node position_node = component_data.child("Position");



}

void ComponentEngine::Rigidbody::ReciveMessage(enteez::Entity * sender, TransformationChange & message)
{
	RemoveRigidbody();
	ICollisionShape* shape = nullptr;
	m_entity->ForEach<ICollisionShape>([&](enteez::Entity* entity, ICollisionShape& found_shape)
	{
		shape = &found_shape;
	});

	AddRigidbody(shape);
}

void ComponentEngine::Rigidbody::ReciveMessage(enteez::Entity * sender, OnComponentEnter<ICollisionShape>& message)
{
	RemoveRigidbody();
	AddRigidbody(&message.GetComponent());
}

void ComponentEngine::Rigidbody::ReciveMessage(enteez::Entity * sender, OnComponentChange<ICollisionShape>& message)
{
	RemoveRigidbody();
	AddRigidbody(&message.GetComponent());
}

void ComponentEngine::Rigidbody::ReciveMessage(enteez::Entity * sender, OnComponentExit<ICollisionShape>& message)
{
	RemoveRigidbody();
	AddRigidbody(nullptr);
}

void ComponentEngine::Rigidbody::ReciveMessage(enteez::Entity* sender, OnComponentExit<Rigidbody>& message)
{
	auto it = std::find(m_currentCollisions->begin(), m_currentCollisions->end(), sender);
	if (m_currentCollisions->end() != it)
	{
		m_currentCollisions->erase(it);
	}
}

void ComponentEngine::Rigidbody::ReciveMessage(enteez::Entity * sender, CollisionRecording & message)
{
	Engine* engine = Engine::Singlton();
	engine->GetPhysicsWorld()->m_physics_lock.lock();
	// Update if we are recording collisions or not
	m_recordingCollisions = message.state == Begin;
	// If we have stopped recording collisions, stop
	if (!m_recordingCollisions)
	{
		*m_collisionStaging = *m_collisionFrameRecording;



		for (int i = m_collisionStaging->size() - 1; i >= 0; i--)
		{
			bool contained = false;
			for (int j = 0; j < m_currentCollisions->size(); j++)
			{
				if (m_collisionStaging->at(i) == m_currentCollisions->at(j))
				{
					// Still colliding
					contained = true;
					Send(m_entity, m_entity, OnCollision{ m_currentCollisions->at(j) });
					m_currentCollisions->erase(m_currentCollisions->begin() + j);
					break;
				}
			}

			if (!contained)
			{
				// Just collided
				Send(m_entity, m_entity, OnCollisionEnter{ m_collisionStaging->at(i) });
			}

			m_collisionStaging->erase(m_collisionStaging->begin() + i);

		}

		for (int i = 0; i < m_currentCollisions->size(); i++)
		{
			// Exited collision
			Send(m_entity, m_entity, OnCollisionExit{ m_currentCollisions->at(i) });
		}



		m_collisionStaging->clear();
		m_currentCollisions->clear();


		{// Swap over frame recording and currenct collision

			std::vector<enteez::Entity*>* temp = m_currentCollisions;
			m_currentCollisions = m_collisionFrameRecording;
			m_collisionFrameRecording = temp;
		}


	}
	engine->GetPhysicsWorld()->m_physics_lock.unlock();
}

void ComponentEngine::Rigidbody::ReciveMessage(enteez::Entity * sender, CollisionEvent & message)
{
	if (m_recordingCollisions)
	{
		//std::cout << "Colliding" << std::endl;
		m_collisionFrameRecording->push_back(message.collider);
	}
}

void ComponentEngine::Rigidbody::Rebuild()
{
	ICollisionShape* shape = nullptr;
	m_entity->ForEach<ICollisionShape>([&](enteez::Entity* entity, ICollisionShape& found_shape)
	{
		shape = &found_shape;
	});

	RemoveRigidbody();
	AddRigidbody(shape);
}

void ComponentEngine::Rigidbody::AddRigidbody(ICollisionShape * shape)
{

	/// Create Dynamic Objects
	btTransform startTransform;
	startTransform.setIdentity();
	if (m_entity->HasComponent<Transformation>())
	{
		Transformation& trans = m_entity->GetComponent<Transformation>();
		startTransform.setFromOpenGLMatrix((btScalar*)&trans.GetMat4());
		//startTransform.setOrigin(btVector3(trans.GetWorldX(), trans.GetWorldY(), trans.GetWorldZ()));
	}

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	m_myMotionState = new btDefaultMotionState(startTransform);

	btCollisionShape* collShape = nullptr;
	
	if (shape == nullptr)
	{
		collShape = new btSphereShape(btScalar(1.));
		m_localShape = collShape;
		m_localShape->setUserPointer(m_entity);
	}
	else
	{
		collShape = shape->GetCollisionShape();
	}

	bool isDynamic = (m_mass != 0.f);

	if (isDynamic)
		collShape->calculateLocalInertia(m_mass, m_localInertia);


	btRigidBody::btRigidBodyConstructionInfo rbInfo(m_mass, m_myMotionState, collShape, m_localInertia);
	Engine::Singlton()->GetPhysicsWorld()->m_physics_lock.lock();
	m_body = new btRigidBody(rbInfo);

	if (m_friction != 0)
		m_body->setFriction(m_friction);
	if (m_RollingFriction != 0)
		m_body->setRollingFriction(m_RollingFriction);
	if (m_SpinningFriction != 0)
		m_body->setSpinningFriction(m_SpinningFriction);
	if (m_AnisotropicFriction != btVector3(0,0,0))
		m_body->setAnisotropicFriction(m_AnisotropicFriction);

	Engine::Singlton()->GetPhysicsWorld()->GetDynamicsWorld()->addRigidBody(m_body);

	if (shape == nullptr || !m_useCollision)
	{
		m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}
	if (m_useGravity)
	{
		m_body->setGravity(btVector3(0, -10, 0));
	}
	else
	{
		m_body->setGravity(btVector3(0, 0, 0));
	}
	Engine::Singlton()->GetPhysicsWorld()->m_physics_lock.unlock();
}

void ComponentEngine::Rigidbody::RemoveRigidbody()
{

	Engine::Singlton()->GetPhysicsWorld()->m_physics_lock.lock();
	m_localInertia = m_body->getLocalInertia();
	Engine::Singlton()->GetPhysicsWorld()->GetDynamicsWorld()->removeRigidBody(m_body);
	if (m_localShape != nullptr)
	{
		delete m_localShape;
		m_localShape = nullptr;
	}
	delete m_myMotionState;
	delete m_body;

	Engine::Singlton()->GetPhysicsWorld()->m_physics_lock.unlock();
}
