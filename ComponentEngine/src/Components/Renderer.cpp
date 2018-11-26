#include <ComponentEngine\Components\Renderer.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>

using namespace ComponentEngine;

ComponentEngine::RendererComponent::RendererComponent(enteez::Entity * entity) : MsgSend(entity)
{
	Send(RenderStatus(true));
}

ComponentEngine::RendererComponent::~RendererComponent()
{
	Send(RenderStatus(false));
}

void ComponentEngine::RendererComponent::EntityHook(enteez::Entity & entity, pugi::xml_node & component_data)
{
	enteez::ComponentWrapper<RendererComponent>* renderer = entity.AddComponent<RendererComponent>(&entity);
	renderer->SetName("Renderer");
}