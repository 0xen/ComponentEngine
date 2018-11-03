#pragma once

namespace enteez
{
	class Entity;
}
namespace pugi
{
	class xml_node;
}

namespace ComponentEngine
{
	struct RendererComponent
	{
		static void EntityHook(enteez::Entity& entity, pugi::xml_node& component_data);
	};
}