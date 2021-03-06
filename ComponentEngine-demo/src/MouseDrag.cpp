#include <MouseDrag.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Transformation.hpp>

#include <EnteeZ\EnteeZ.hpp>
#include <EnteeZ\Entity.hpp>

#include <imgui.h>

#include <sstream>

ComponentEngine::MouseDrag::MouseDrag(enteez::Entity * entity)
{
	m_lock_x = m_lock_y = m_flip_x = m_flip_y = false;
	offset = glm::vec3(0);
	m_rotateSpeed = 0;
	m_entity = entity;
}

void ComponentEngine::MouseDrag::Update(float frame_time)
{
	Engine* engine = Engine::Singlton();


	if (engine->MouseKeyDown(0) && Engine::Singlton()->GetHoveredWindowName() == "PlayWindow")
	{
		engine->GrabMouse(true);

		glm::vec2 mousePosition = engine->GetLastMouseMovment();


		Transformation* trans = &m_entity->GetComponent<Transformation>();

		glm::mat4 mat4 = trans->GetLocalMat4();

		glm::vec3 direction = mat4[2];
		glm::vec3 translation = mat4[3];

		if (!m_lock_x)
		{
			float rotAmountX = (m_flip_x ? -1.0f : 1) * m_rotateSpeed * mousePosition.x;
			direction = glm::rotate(direction, rotAmountX, glm::vec3(0, 1, 0));
		}

		if (!m_lock_y)
		{
			float rotAmountY = (direction.z >0.0f && m_flip_y || direction.z <0.0f && !m_flip_y ? -1.0f : 1) * m_rotateSpeed * mousePosition.y;
			direction = glm::rotate(direction, rotAmountY, glm::vec3(1, 0, 0));
		}

		glm::mat4 view = glm::inverse(glm::lookAt(translation + offset, translation + offset + -direction, glm::vec3(0, 1, 0)));
		view[3] -= glm::vec4(offset, 0);
		trans->SetLocalMat4(view);

	}
	else
	{
		engine->GrabMouse(false);
	}
}

void ComponentEngine::MouseDrag::EditorUpdate(float frame_time)
{

}

void ComponentEngine::MouseDrag::Display()
{

	ImGui::Text("Rotate Speed");
	ImGui::DragFloat("##RotateSpeed", &m_rotateSpeed, 0.001, 0.0001f, 1.0f);


	ImGui::Text("Flip X");
	ImGui::Checkbox("##FlipX", &m_flip_x);

	ImGui::Text("Flip Y");
	ImGui::Checkbox("##FlipY", &m_flip_y);

	ImGui::Text("Lock X");
	ImGui::Checkbox("##LockX", &m_lock_x);

	ImGui::Text("Lock Y");
	ImGui::Checkbox("##LockY", &m_lock_y);

	ImGui::Text("Offset");
	ImGui::InputFloat3("##offset", (float*)&offset, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
}

void ComponentEngine::MouseDrag::Load(pugi::xml_node& node)
{
	m_rotateSpeed = node.attribute("RotationSpeed").as_float(m_rotateSpeed);
	{
		pugi::xml_node offsedNode = node.child("Offset");
		offset.x = offsedNode.attribute("X").as_float(offset.x);
		offset.y = offsedNode.attribute("Y").as_float(offset.y);
		offset.z = offsedNode.attribute("Z").as_float(offset.z);
	}
	m_flip_x = node.attribute("FlipX").as_bool(m_flip_x);
	m_flip_y = node.attribute("FlipY").as_bool(m_flip_y);
	m_lock_x = node.attribute("LockX").as_bool(m_lock_x);
	m_lock_y = node.attribute("LockY").as_bool(m_lock_y);
}

void ComponentEngine::MouseDrag::Save(pugi::xml_node& node)
{
	node.append_attribute("RotationSpeed").set_value(m_rotateSpeed);
	{
		pugi::xml_node offsedNode = node.append_child("Offset");
		offsedNode.append_attribute("X").set_value(offset.x);
		offsedNode.append_attribute("Y").set_value(offset.y);
		offsedNode.append_attribute("Z").set_value(offset.z);
	}
	node.append_attribute("FlipX").set_value(m_flip_x);
	node.append_attribute("FlipY").set_value(m_flip_y);
	node.append_attribute("LockX").set_value(m_lock_x);
	node.append_attribute("LockY").set_value(m_lock_y);
}

enteez::BaseComponentWrapper * ComponentEngine::MouseDrag::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<MouseDrag>* wrapper = entity.AddComponent<MouseDrag>(&entity);
	wrapper->SetName("Mouse Drag");
	return wrapper;
}