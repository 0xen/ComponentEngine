#include <ComponentEngine\Components\BoxCollision.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\PhysicsWorld.hpp>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <imgui.h>

ComponentEngine::BoxCollision::BoxCollision(enteez::Entity * entity) : ICollisionShape(entity)
{
	m_shape = glm::vec3(0.5f, 0.5f, 0.5f);

	CreateCollision();

	Send(m_entity, m_entity, OnComponentEnter<ICollisionShape>(this));
}

ComponentEngine::BoxCollision::~BoxCollision()
{
	Send(m_entity, m_entity, OnComponentExit<ICollisionShape>(this));
}

void ComponentEngine::BoxCollision::Display()
{

	btBoxShape* boxShape = (btBoxShape*)m_colShape;

	ImGui::Text("Dimentions");


	if(ImGui::InputFloat3("##Dimentions_BoxCollider", (float*)&m_shape, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
	{
		Rebuild();
	}

}

enteez::BaseComponentWrapper* ComponentEngine::BoxCollision::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<BoxCollision>* mesh = entity.AddComponent<BoxCollision>(&entity);
	mesh->SetName("Box Collision");
	return mesh;
}

void ComponentEngine::BoxCollision::Rebuild()
{

	btCollisionShape * temp = m_colShape;

	CreateCollision();
	Send(m_entity, m_entity, OnComponentChange<ICollisionShape>(this));

	delete temp;
}

void ComponentEngine::BoxCollision::CreateCollision()
{
	m_colShape = new btBoxShape(btVector3(btScalar(m_shape.x), btScalar(m_shape.y), btScalar(m_shape.z)));
	m_colShape->setUserPointer(m_entity);
}
