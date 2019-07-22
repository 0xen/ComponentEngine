#include <ComponentEngine\Components\SphereCollision.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\PhysicsWorld.hpp>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <imgui.h>

ComponentEngine::SphereCollision::SphereCollision(enteez::Entity * entity) : ICollisionShape(entity)
{
	m_rad = 0.5f;

	CreateCollision();

	Send(m_entity, m_entity, OnComponentEnter<ICollisionShape>(this));
}

ComponentEngine::SphereCollision::~SphereCollision()
{
	Send(m_entity, m_entity, OnComponentExit<ICollisionShape>(this));
}

void ComponentEngine::SphereCollision::Display()
{

	btBoxShape* boxShape = (btBoxShape*)m_colShape;

	ImGui::Text("Radius");


	if (ImGui::InputFloat("##Rad_SphereCollider", &m_rad, 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
	{
		Rebuild();
	}

}

void ComponentEngine::SphereCollision::Load(std::ifstream & in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(SphereCollision, m_rad), PayloadSize());
}

void ComponentEngine::SphereCollision::Save(std::ofstream & out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(SphereCollision, m_rad), PayloadSize());
}

unsigned int ComponentEngine::SphereCollision::PayloadSize()
{
	return SizeOfOffsetRange(SphereCollision, m_rad, m_rad);
}

bool ComponentEngine::SphereCollision::DynamiclySized()
{
	return false;
}

enteez::BaseComponentWrapper* ComponentEngine::SphereCollision::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<SphereCollision>* mesh = entity.AddComponent<SphereCollision>(&entity);
	mesh->SetName("Sphere Collision");
	return mesh;
}

void ComponentEngine::SphereCollision::Rebuild()
{

	btCollisionShape * temp = m_colShape;

	CreateCollision();
	Send(m_entity, m_entity, OnComponentChange<ICollisionShape>(this));

	delete temp;
}

void ComponentEngine::SphereCollision::CreateCollision()
{
	btSphereShape * colShape = new btSphereShape(btScalar(m_rad));
	m_colShape = colShape;
	m_colShape->setUserPointer(m_entity);
}
