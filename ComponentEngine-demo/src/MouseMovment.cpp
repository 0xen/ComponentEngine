#include <MouseMovment.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\UIManager.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>
#include <SDL.h>


ComponentEngine::MouseMovment::MouseMovment(enteez::Entity* entity)
{
	m_entity = entity;
	m_speed = 1.0f;
	m_unlockCameraKey = SDL_SCANCODE_ESCAPE;
	m_unlockCameraKeyFocus = false;
	status = MouseStatus::UnlockedUI;
}

void ComponentEngine::MouseMovment::Update(float frame_time)
{
	// Since we are focusing on a window, we want to ignore any keyboard inputs

	Engine* engine = Engine::Singlton();

	if (UIManager::IsWindowFocused())
	{
		status = MouseStatus::UnlockedUI;
		engine->GrabMouse(false);
	}
	else if (status == MouseStatus::UnlockedUI || engine->MouseKeyDown(0))
	{
		status = MouseStatus::Locked;
		engine->GrabMouse(true);
	}

	if (status == MouseStatus::UnlockedUI || status == MouseStatus::Unlocked)
	{
		return;
	}
	Transformation* trans = &m_entity->GetComponent<Transformation>();

	if (engine->KeyDown(m_unlockCameraKey))
	{
		status = MouseStatus::Unlocked;
		engine->GrabMouse(false);
		return;
	}

	glm::vec2 mouseDelta = engine->GetLastMouseMovment();

	trans->RotateWorldY(-mouseDelta.x * m_speed * frame_time);
}

void ComponentEngine::MouseMovment::Display()
{
	{ // Movment speed input
		ImGui::Text("Movment Speed");
		ImGui::DragFloat("##MouseMovment_Speed", &m_speed, 0.1, 0.1f, 10.0);
	}
	{
		ImGui::Text("UnlockMouseFocus");
		UIManager::KeyboardButtonInput("##MouseMovment_KeyForward", m_unlockCameraKeyFocus, m_unlockCameraKey);
	}
}

void ComponentEngine::MouseMovment::EntityHookDefault(enteez::Entity& entity)
{
	enteez::ComponentWrapper<MouseMovment>* wrapper = entity.AddComponent<MouseMovment>(&entity);
	wrapper->SetName("Mouse Movment");
}

void ComponentEngine::MouseMovment::EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data)
{
	enteez::ComponentWrapper<MouseMovment>* wrapper = entity.AddComponent<MouseMovment>(&entity);
	wrapper->SetName("Mouse Movment");
}
