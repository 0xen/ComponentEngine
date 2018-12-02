#include <ComponentEngine\Components\Renderer.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>

#include <imgui.h>

using namespace ComponentEngine;

ComponentEngine::RendererComponent::RendererComponent(enteez::Entity * entity) : m_entity(entity) /*: MsgSend(entity)*/
{
	m_render = true;
	Send(m_entity, RenderStatus(m_render));
}

ComponentEngine::RendererComponent::~RendererComponent()
{
	Send(m_entity, RenderStatus(false));
}

void ComponentEngine::RendererComponent::EntityHook(enteez::Entity & entity, pugi::xml_node & component_data)
{
	enteez::ComponentWrapper<RendererComponent>* renderer = entity.AddComponent<RendererComponent>(&entity);
	renderer->SetName("Renderer");
}

void ComponentEngine::RendererComponent::Display()
{
	bool change = ImGui::Checkbox("Render",&m_render);
	if (change)
	{
		Send(m_entity, RenderStatus(m_render));
	}
}
