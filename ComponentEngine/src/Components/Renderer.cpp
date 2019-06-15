#include <ComponentEngine\Components\Renderer.hpp>

#include <EnteeZ\EnteeZ.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>

#include <imgui.h>

using namespace ComponentEngine;

ComponentEngine::RendererComponent::RendererComponent(enteez::Entity * entity) : m_entity(entity) /*: MsgSend(entity)*/
{
	m_render = true;
	Send(m_entity, m_entity, RenderStatus(m_render));
}

ComponentEngine::RendererComponent::~RendererComponent()
{
	Send(m_entity, m_entity, RenderStatus(false));
}

enteez::BaseComponentWrapper* ComponentEngine::RendererComponent::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<RendererComponent>* renderer = entity.AddComponent<RendererComponent>(&entity);
	renderer->SetName("Renderer");
	return renderer;
}

void ComponentEngine::RendererComponent::Display()
{
	bool change = ImGui::Checkbox("Render",&m_render);

	if (change)
	{
		Send(m_entity, m_entity, RenderStatus(m_render));
	}
}

void ComponentEngine::RendererComponent::ReciveMessage(enteez::Entity * sender, ComponentEngine::OnComponentEnter<Mesh>& message)
{
	Send(m_entity, m_entity, RenderStatus(m_render));
}

