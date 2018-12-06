#pragma once

#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\UI.hpp>

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
	struct RendererComponent : public UI
	{
		RendererComponent(enteez::Entity* entity);
		~RendererComponent();
		virtual void Display();
		static void EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);
	private:
		enteez::Entity * m_entity;
		bool m_render = false;
	};
}