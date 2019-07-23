#include <KeyboardMovment.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\UI\UIManager.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>
#include <SDL.h>


ComponentEngine::KeyboardMovment::KeyboardMovment(enteez::Entity* entity)
{
	m_entity = entity;
	keys[0] = { SDL_SCANCODE_W };
	keys[1] = { SDL_SCANCODE_S };
	keys[2] = { SDL_SCANCODE_A };
	keys[3] = { SDL_SCANCODE_D };
	m_speed = 10.0f;
	m_ignore_axis[0] = false;
	m_ignore_axis[1] = false;
	m_ignore_axis[2] = false;
	m_local_movment = true;
}

void ComponentEngine::KeyboardMovment::Update(float frame_time)
{
	// Since we are focusing on a window, we want to ignore any keyboard inputs
	if (UIManager::IsWindowFocused())
		return;

	Engine* engine = Engine::Singlton();
	Transformation* trans = &m_entity->GetComponent<Transformation>();

	glm::mat4 mat4;
	if(m_local_movment)
		mat4 = trans->GetLocalMat4();
	else
		mat4 = trans->GetMat4();

	bool change = false;

	if (engine->KeyDown(keys[0].key)) // Forward
	{
		glm::vec3 zFacing = mat4[2];
		zFacing = glm::normalize(zFacing) * -m_speed * frame_time;
		if (!m_ignore_axis[0])(mat4)[3][0] += zFacing.x;
		if (!m_ignore_axis[1])(mat4)[3][1] += zFacing.y;
		if (!m_ignore_axis[2])(mat4)[3][2] += zFacing.z;
		change = true;
	}
	if (engine->KeyDown(keys[1].key)) // Back
	{
		glm::vec3 zFacing = mat4[2];
		zFacing = glm::normalize(zFacing) * m_speed * frame_time;
		if (!m_ignore_axis[0])(mat4)[3][0] += zFacing.x;
		if (!m_ignore_axis[1])(mat4)[3][1] += zFacing.y;
		if (!m_ignore_axis[2])(mat4)[3][2] += zFacing.z;
		change = true;
	}
	if (engine->KeyDown(keys[2].key)) // Left
	{
		glm::vec3 xFacing = mat4[0];
		xFacing = glm::normalize(xFacing) * -m_speed * frame_time;
		if (!m_ignore_axis[0])(mat4)[3][0] += xFacing.x;
		if (!m_ignore_axis[1])(mat4)[3][1] += xFacing.y;
		if (!m_ignore_axis[2])(mat4)[3][2] += xFacing.z;
		change = true;
	}
	if (engine->KeyDown(keys[3].key)) // Right
	{
		glm::vec3 xFacing = mat4[0];
		xFacing = glm::normalize(xFacing) * m_speed * frame_time;
		if (!m_ignore_axis[0])(mat4)[3][0] += xFacing.x;
		if (!m_ignore_axis[1])(mat4)[3][1] += xFacing.y;
		if (!m_ignore_axis[2])(mat4)[3][2] += xFacing.z;
		change = true;
	}

	if (m_local_movment)
		trans->SetLocalMat4(mat4);
	else
		trans->SetWorldMat4(mat4);
}

void ComponentEngine::KeyboardMovment::EditorUpdate(float frame_time)
{
	Update(frame_time);
}

void ComponentEngine::KeyboardMovment::Display()
{
	{ // Movment speed input
		ImGui::Text("Movment Speed");
		ImGui::DragFloat("##KeyboardMovment_Speed", &m_speed, 0.5f, 1.0f, 25.0f);
	}
	{
		ImGui::Text("Forward");
		UIManager::KeyboardButtonInput("##KeyboardMovment_KeyForward", keys[0].focused, keys[0].key);

		ImGui::Text("Back");
		UIManager::KeyboardButtonInput("##KeyboardMovment_KeyBack", keys[1].focused, keys[1].key);

		ImGui::Text("Left");
		UIManager::KeyboardButtonInput("##KeyboardMovment_KeyLeft", keys[2].focused, keys[2].key);

		ImGui::Text("Right");
		UIManager::KeyboardButtonInput("##KeyboardMovment_KeyRight", keys[3].focused, keys[3].key);

	}
	{
		ImGui::Text("Local Movement");
		ImGui::Checkbox("##LocalMovment", &m_local_movment);
	}
	{
		ImGui::Text("Ignore Axis");
		ImGui::Checkbox("X", &m_ignore_axis[0]);
		ImGui::Checkbox("Y", &m_ignore_axis[1]);
		ImGui::Checkbox("Z", &m_ignore_axis[2]);
	}
}

void ComponentEngine::KeyboardMovment::Load(std::ifstream & in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(KeyboardMovment, keys), PayloadSize());
}

void ComponentEngine::KeyboardMovment::Save(std::ofstream & out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(KeyboardMovment, keys), PayloadSize());
}

unsigned int ComponentEngine::KeyboardMovment::PayloadSize()
{
	return SizeOfOffsetRange(KeyboardMovment, keys, m_local_movment);
}

bool ComponentEngine::KeyboardMovment::DynamiclySized()
{
	return false;
}

enteez::BaseComponentWrapper* ComponentEngine::KeyboardMovment::EntityHookDefault(enteez::Entity& entity)
{
	enteez::ComponentWrapper<KeyboardMovment>* wrapper = entity.AddComponent<KeyboardMovment>(&entity);
	wrapper->SetName("Keyboard Movment");
	return wrapper;
}