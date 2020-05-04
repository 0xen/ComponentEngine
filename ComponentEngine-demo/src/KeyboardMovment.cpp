#include <KeyboardMovment.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\UI\UIManager.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>
#include <SDL.h>


ComponentEngine::KeyboardMovment::KeyboardMovment(enteez::Entity* entity)
{
	m_entity = entity;
	keys0 = { SDL_SCANCODE_W };
	keys1 = { SDL_SCANCODE_S };
	keys2 = { SDL_SCANCODE_A };
	keys3 = { SDL_SCANCODE_D };
	keys4 = { SDL_SCANCODE_C };
	keys5 = { SDL_SCANCODE_SPACE };
	m_speed = 10.0f;
	m_ignore_axis_x = false;
	m_ignore_axis_y = false;
	m_ignore_axis_z = false;
	m_local_movment = true;
	m_inEditor = false;
}

void ComponentEngine::KeyboardMovment::Update(float frame_time)
{
	// Since we are focusing on a window, we want to ignore any keyboard inputs
	/*if (UIManager::IsWindowFocused())
		return;*/

	Engine* engine = Engine::Singlton();
	Transformation* trans = &m_entity->GetComponent<Transformation>();

	glm::mat4 mat4;
	if(m_local_movment)
		mat4 = trans->GetLocalMat4();
	else
		mat4 = trans->GetMat4();

	bool change = false;

	if (engine->KeyDown(keys0.key)) // Forward
	{
		glm::vec3 zFacing = mat4[2];
		zFacing = glm::normalize(zFacing) * -m_speed * frame_time;
		if (!m_ignore_axis_x)(mat4)[3][0] += zFacing.x;
		if (!m_ignore_axis_y)(mat4)[3][1] += zFacing.y;
		if (!m_ignore_axis_z)(mat4)[3][2] += zFacing.z;
		change = true;
	}
	if (engine->KeyDown(keys1.key)) // Back
	{
		glm::vec3 zFacing = mat4[2];
		zFacing = glm::normalize(zFacing) * m_speed * frame_time;
		if (!m_ignore_axis_x)(mat4)[3][0] += zFacing.x;
		if (!m_ignore_axis_y)(mat4)[3][1] += zFacing.y;
		if (!m_ignore_axis_z)(mat4)[3][2] += zFacing.z;
		change = true;
	}
	if (engine->KeyDown(keys2.key)) // Left
	{
		glm::vec3 xFacing = mat4[0];
		xFacing = glm::normalize(xFacing) * -m_speed * frame_time;
		if (!m_ignore_axis_x)(mat4)[3][0] += xFacing.x;
		if (!m_ignore_axis_y)(mat4)[3][1] += xFacing.y;
		if (!m_ignore_axis_z)(mat4)[3][2] += xFacing.z;
		change = true;
	}
	if (engine->KeyDown(keys3.key)) // Right
	{
		glm::vec3 xFacing = mat4[0];
		xFacing = glm::normalize(xFacing) * m_speed * frame_time;
		if (!m_ignore_axis_x)(mat4)[3][0] += xFacing.x;
		if (!m_ignore_axis_y)(mat4)[3][1] += xFacing.y;
		if (!m_ignore_axis_z)(mat4)[3][2] += xFacing.z;
		change = true;
	}
	if (engine->KeyDown(keys4.key)) // Down
	{
		glm::vec3 yFacing = mat4[1];
		yFacing = glm::normalize(yFacing) * -m_speed * frame_time;
		if (!m_ignore_axis_x)(mat4)[3][0] += yFacing.x;
		if (!m_ignore_axis_y)(mat4)[3][1] += yFacing.y;
		if (!m_ignore_axis_z)(mat4)[3][2] += yFacing.z;
		change = true;
	}
	if (engine->KeyDown(keys5.key)) // up
	{
		glm::vec3 yFacing = mat4[1];
		yFacing = glm::normalize(yFacing) * m_speed * frame_time;
		if (!m_ignore_axis_x)(mat4)[3][0] += yFacing.x;
		if (!m_ignore_axis_y)(mat4)[3][1] += yFacing.y;
		if (!m_ignore_axis_z)(mat4)[3][2] += yFacing.z;
		change = true;
	}

	if (m_local_movment)
		trans->SetLocalMat4(mat4);
	else
		trans->SetWorldMat4(mat4);
}

void ComponentEngine::KeyboardMovment::EditorUpdate(float frame_time)
{
	if(m_inEditor)
		Update(frame_time);
}

void ComponentEngine::KeyboardMovment::Display()
{
	{
		ImGui::Text("Work In Editor");
		ImGui::Checkbox("##KeyboardMovmentInEditor",&m_inEditor);
	}
	{ // Movment speed input
		ImGui::Text("Movment Speed");
		ImGui::DragFloat("##KeyboardMovment_Speed", &m_speed, 0.5f, 1.0f, 25.0f);
	}
	{
		ImGui::Text("Forward");
		UIManager::KeyboardButtonInput("##KeyboardMovment_KeyForward", keys0.focused, keys0.key);

		ImGui::Text("Back");
		UIManager::KeyboardButtonInput("##KeyboardMovment_KeyBack", keys1.focused, keys1.key);

		ImGui::Text("Left");
		UIManager::KeyboardButtonInput("##KeyboardMovment_KeyLeft", keys2.focused, keys2.key);

		ImGui::Text("Right");
		UIManager::KeyboardButtonInput("##KeyboardMovment_KeyRight", keys3.focused, keys3.key);

	}
	{
		ImGui::Text("Local Movement");
		ImGui::Checkbox("##LocalMovment", &m_local_movment);
	}
	{
		ImGui::Text("Ignore Axis");
		ImGui::Checkbox("X", &m_ignore_axis_x);
		ImGui::Checkbox("Y", &m_ignore_axis_y);
		ImGui::Checkbox("Z", &m_ignore_axis_z);
	}
}

void ComponentEngine::KeyboardMovment::Load(pugi::xml_node& node)
{
	m_speed = node.attribute("Speed").as_float(m_speed);
	m_ignore_axis_x = node.attribute("IgnoreAxisX").as_bool(m_ignore_axis_x);
	m_ignore_axis_y = node.attribute("IgnoreAxisY").as_bool(m_ignore_axis_y);
	m_ignore_axis_z = node.attribute("IgnoreAxisZ").as_bool(m_ignore_axis_z);
	m_local_movment = node.attribute("LocalMovment").as_bool(m_local_movment);
	m_inEditor = node.attribute("InEditor").as_bool(m_inEditor);


	keys0.key = node.child("Key0").attribute("Value").as_uint(keys0.key);
	keys1.key = node.child("Key1").attribute("Value").as_uint(keys1.key);
	keys2.key = node.child("Key2").attribute("Value").as_uint(keys2.key);
	keys3.key = node.child("Key3").attribute("Value").as_uint(keys3.key);
	keys4.key = node.child("Key4").attribute("Value").as_uint(keys4.key);
	keys5.key = node.child("Key5").attribute("Value").as_uint(keys5.key);
}

void ComponentEngine::KeyboardMovment::Save(pugi::xml_node& node)
{
	node.append_attribute("Speed").set_value(m_speed);
	node.append_attribute("IgnoreAxisX").set_value(m_ignore_axis_x);
	node.append_attribute("IgnoreAxisY").set_value(m_ignore_axis_y);
	node.append_attribute("IgnoreAxisZ").set_value(m_ignore_axis_z);
	node.append_attribute("LocalMovment").set_value(m_local_movment);
	node.append_attribute("InEditor").set_value(m_inEditor);


	node.append_child("Key0").append_attribute("Value").set_value(keys0.key);
	node.append_child("Key1").append_attribute("Value").set_value(keys1.key);
	node.append_child("Key2").append_attribute("Value").set_value(keys2.key);
	node.append_child("Key3").append_attribute("Value").set_value(keys3.key);
	node.append_child("Key4").append_attribute("Value").set_value(keys4.key);
	node.append_child("Key5").append_attribute("Value").set_value(keys5.key);
}

enteez::BaseComponentWrapper* ComponentEngine::KeyboardMovment::EntityHookDefault(enteez::Entity& entity)
{
	enteez::ComponentWrapper<KeyboardMovment>* wrapper = entity.AddComponent<KeyboardMovment>(&entity);
	wrapper->SetName("Keyboard Movement");
	return wrapper;
}