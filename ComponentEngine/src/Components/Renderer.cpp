#include <ComponentEngine\Components\Renderer.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>

using namespace ComponentEngine;

void ComponentEngine::RendererComponent::EntityHook(enteez::Entity & entity, pugi::xml_node & component_data)
{
	entity.AddComponent<RendererComponent>();
}