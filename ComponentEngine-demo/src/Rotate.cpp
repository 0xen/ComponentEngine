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

void ComponentEngine::Rotate::Load(pugi::xml_node& node)
{
	m_rotateSpeed = node.attribute("Speed").as_float(m_rotateSpeed);
	{
		pugi::xml_node Axis = node.child("Axis");
		m_axis_selection = Axis.attribute("AxisSelection").as_int(m_axis_selection);
		m_axis.x = Axis.attribute("X").as_float(m_axis.x);
		m_axis.y = Axis.attribute("Y").as_float(m_axis.y);
		m_axis.z = Axis.attribute("Z").as_float(m_axis.z);
	}
}

void ComponentEngine::Rotate::Save(pugi::xml_node& node)
{
	node.append_attribute("Speed").set_value(m_rotateSpeed);
	{
		pugi::xml_node Axis = node.append_child("Axis");
		Axis.append_attribute("AxisSelection").set_value(m_axis_selection);
		Axis.append_attribute("X").set_value(m_axis.x);
		Axis.append_attribute("Y").set_value(m_axis.y);
		Axis.append_attribute("Z").set_value(m_axis.z);
	}
}

enteez::BaseComponentWrapper * ComponentEngine::Rotate::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<Rotate>* wrapper = entity.AddComponent<Rotate>(&entity);
	wrapper->SetName("Rotate");
	return wrapper;
}