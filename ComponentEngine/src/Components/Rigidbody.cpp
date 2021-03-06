#include <ComponentEngine\Components\Rigidbody.hpp>
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
	m_gravity = -10.0f;

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
	{ // Gravity
		ImGui::Text("Gravity Power");
		if (ImGui::InputFloat("##Gravity", &m_gravity, 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
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

enteez::BaseComponentWrapper* ComponentEngine::Rigidbody::EntityHookDefault(enteez::Entity& entity)
{
	enteez::ComponentWrapper<Rigidbody>* mesh = entity.AddComponent<Rigidbody>(&entity);
	mesh->SetName("Rigidbody");
	return mesh;
}

void ComponentEngine::Rigidbody::Load(pugi::xml_node& node)
{
	node.attribute("Friction").as_float(m_friction);
	node.attribute("RollingFriction").as_float(m_RollingFriction);
	node.attribute("SpinningFriction").as_float(m_SpinningFriction);
	{
		pugi::xml_node AnistropicFriction = node.child("AnistropicFriction");
		m_AnisotropicFriction = btVector3(
			AnistropicFriction.attribute("X").as_float(),
			AnistropicFriction.attribute("Y").as_float(),
			AnistropicFriction.attribute("Z").as_float());
	}
	{
		pugi::xml_node LocalInertia = node.child("LocalInertia");
		m_localInertia = btVector3(
			LocalInertia.attribute("X").as_float(),
			LocalInertia.attribute("Y").as_float(),
			LocalInertia.attribute("Z").as_float());
	}
	m_gravity = node.attribute("Gravity").as_float(m_gravity);
	m_useGravity = node.attribute("UseGravity").as_bool(m_useGravity);
	m_useCollision = node.attribute("UseCollision").as_bool(m_useCollision);
	m_recordingCollisions = node.attribute("RecordingCollisions").as_bool(m_recordingCollisions);
	m_mass = node.attribute("Mass").as_float(m_mass);
}

void ComponentEngine::Rigidbody::Save(pugi::xml_node& node)
{
	node.append_attribute("Friction").set_value(m_friction);
	node.append_attribute("RollingFriction").set_value(m_RollingFriction);
	node.append_attribute("SpinningFriction").set_value(m_SpinningFriction);
	{
		pugi::xml_node AnistropicFriction = node.append_child("AnistropicFriction");
		AnistropicFriction.append_attribute("X").set_value(m_AnisotropicFriction.x());
		AnistropicFriction.append_attribute("Y").set_value(m_AnisotropicFriction.y());
		AnistropicFriction.append_attribute("Z").set_value(m_AnisotropicFriction.z());
	}
	{
		pugi::xml_node LocalInertia = node.append_child("LocalInertia");
		LocalInertia.append_attribute("X").set_value(m_localInertia.x());
		LocalInertia.append_attribute("Y").set_value(m_localInertia.y());
		LocalInertia.append_attribute("Z").set_value(m_localInertia.z());
	}
	node.append_attribute("Gravity").set_value(m_gravity);
	node.append_attribute("UseGravity").set_value(m_useGravity);
	node.append_attribute("UseCollision").set_value(m_useCollision);
	node.append_attribute("RecordingCollisions").set_value(m_recordingCollisions);
	node.append_attribute("Mass").set_value(m_mass);
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
		m_body->setGravity(btVector3(0, m_gravity, 0));
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
