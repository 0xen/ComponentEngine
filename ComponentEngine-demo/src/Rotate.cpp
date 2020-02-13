#include <Rotate.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Transformation.hpp>

#include <EnteeZ\EnteeZ.hpp>
#include <EnteeZ\Entity.hpp>

#include <imgui.h>

#include <sstream>

ComponentEngine::Rotate::Rotate(enteez::Entity * entity)
{
	m_rotateSpeed = 0;
	m_entity = entity;
}

void ComponentEngine::Rotate::Update(float frame_time)
{
	Engine* engine = Engine::Singlton();

	Transformation* trans = &m_entity->GetComponent<Transformation>();

	switch (m_axis_selection)
	{
	case 0:
		trans->RotateWorldX(m_rotateSpeed);
		break;
	case 1:
		trans->RotateWorldY(m_rotateSpeed);
		break;
	case 2:
		trans->RotateWorldZ(m_rotateSpeed);
		break;
	}
}

void ComponentEngine::Rotate::EditorUpdate(float frame_time)
{

}

void ComponentEngine::Rotate::Display()
{

	ImGui::Text("Rotate Speed");
	ImGui::DragFloat("##RotateSpeed", &m_rotateSpeed, 0.001, 0.0001f, 1.0f);

	const char* m_axis_lables[3] = { "X", "Y", "Z" };
	ImGui::Text("Axis");
	ImGui::Combo("##Axis", &m_axis_selection, m_axis_lables, 3, 2);


}

void ComponentEngine::Rotate::Load(std::ifstream & in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(Rotate, m_rotateSpeed), PayloadSize());
}

void ComponentEngine::Rotate::Save(std::ofstream & out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(Rotate, m_rotateSpeed), PayloadSize());
}

unsigned int ComponentEngine::Rotate::PayloadSize()
{
	return SizeOfOffsetRange(Rotate, m_rotateSpeed, m_axis);
}

bool ComponentEngine::Rotate::DynamiclySized()
{
	return false;
}

enteez::BaseComponentWrapper * ComponentEngine::Rotate::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<Rotate>* wrapper = entity.AddComponent<Rotate>(&entity);
	wrapper->SetName("Rotate");
	return wrapper;
}