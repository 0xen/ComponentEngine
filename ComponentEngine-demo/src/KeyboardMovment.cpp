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

void ComponentEngine::KeyboardMovment::Load(std::ifstream & in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(KeyboardMovment, m_speed), PayloadSize());
}

void ComponentEngine::KeyboardMovment::Save(std::ofstream & out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(KeyboardMovment, m_speed), PayloadSize());
}

unsigned int ComponentEngine::KeyboardMovment::PayloadSize()
{
	return SizeOfOffsetRange(KeyboardMovment, m_speed, m_inEditor);
}

bool ComponentEngine::KeyboardMovment::DynamiclySized()
{
	return false;
}

enteez::BaseComponentWrapper* ComponentEngine::KeyboardMovment::EntityHookDefault(enteez::Entity& entity)
{
	enteez::ComponentWrapper<KeyboardMovment>* wrapper = entity.AddComponent<KeyboardMovment>(&entity);
	wrapper->SetName("Keyboard Movement");
	return wrapper;
}