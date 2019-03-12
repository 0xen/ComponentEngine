#include <KeyboardMovment.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\UIManager.hpp>
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
}

void ComponentEngine::KeyboardMovment::Update(float frame_time)
{
	// Since we are focusing on a window, we want to ignore any keyboard inputs
	if (UIManager::IsWindowFocused())
		return;

	Engine* engine = Engine::Singlton();
	Transformation* trans = &m_entity->GetComponent<Transformation>();

	// Check the movment axis and move accordingly
	if (engine->KeyDown(keys[0].key)) // Forward
		trans->MoveLocalZ(-m_speed * frame_time);
	if (engine->KeyDown(keys[1].key)) // Back
		trans->MoveLocalZ(m_speed * frame_time);
	if (engine->KeyDown(keys[2].key)) // Left
		trans->MoveLocalX(-m_speed * frame_time);
	if (engine->KeyDown(keys[3].key)) // Right
		trans->MoveLocalX(m_speed * frame_time);
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
}

void ComponentEngine::KeyboardMovment::EntityHookDefault(enteez::Entity& entity)
{
	enteez::ComponentWrapper<KeyboardMovment>* wrapper = entity.AddComponent<KeyboardMovment>(&entity);
	wrapper->SetName("Keyboard Movment");
}

void ComponentEngine::KeyboardMovment::EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data)
{
	enteez::ComponentWrapper<KeyboardMovment>* wrapper = entity.AddComponent<KeyboardMovment>(&entity);
	wrapper->SetName("Keyboard Movment");
}
