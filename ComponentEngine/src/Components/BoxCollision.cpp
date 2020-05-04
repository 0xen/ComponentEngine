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

void ComponentEngine::BoxCollision::Load(pugi::xml_node& node)
{
	pugi::xml_node shape = node.child("Shape");
	m_shape.x = shape.attribute("X").as_float(m_shape.x);
	m_shape.y = shape.attribute("Y").as_float(m_shape.y);
	m_shape.z = shape.attribute("Z").as_float(m_shape.z);
}

void ComponentEngine::BoxCollision::Save(pugi::xml_node& node)
{
	pugi::xml_node shape = node.append_child("Shape");
	shape.append_attribute("X").set_value(m_shape.x);
	shape.append_attribute("Y").set_value(m_shape.y);
	shape.append_attribute("Z").set_value(m_shape.z);
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
