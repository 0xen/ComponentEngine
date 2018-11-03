#pragma once

#include <ComponentEngine\Components\MsgSend.hpp>

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
	struct RendererComponent : public MsgSend
	{
		RendererComponent(enteez::Entity* entity);
		~RendererComponent();
		static void EntityHook(enteez::Entity& entity, pugi::xml_node& component_data);
	};
}