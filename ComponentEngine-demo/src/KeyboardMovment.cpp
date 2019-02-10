#include <KeyboardMovment.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <ComponentEngine\Engine.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>
#include <SDL.h>


ComponentEngine::KeyboardMovment::KeyboardMovment(enteez::Entity* entity)
{
	m_entity = entity;
}

void ComponentEngine::KeyboardMovment::Update(float frame_time)
{
	Engine* engine = Engine::Singlton();
	Transformation* trans = &m_entity->GetComponent<Transformation>();

	if (engine->KeyDown(SDL_SCANCODE_W))
		trans->MoveLocalZ(-10.0f * frame_time);
	if (engine->KeyDown(SDL_SCANCODE_S))
		trans->MoveLocalZ(10.0f * frame_time);


	if (engine->KeyDown(SDL_SCANCODE_A))
		trans->RotateWorldY(glm::radians(90.0f * frame_time));
	if (engine->KeyDown(SDL_SCANCODE_D))
		trans->RotateWorldY(glm::radians(-90.0f * frame_time));
}

void ComponentEngine::KeyboardMovment::Display()
{
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
