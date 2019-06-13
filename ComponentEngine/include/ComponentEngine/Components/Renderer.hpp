#pragma once

#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\MsgRecive.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
#include <ComponentEngine\Components\UI.hpp>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
}
namespace pugi
{
	class xml_node;
}

namespace ComponentEngine
{
	class Mesh;
	struct RendererComponent : public UI , public MsgRecive<OnComponentEnter<Mesh>>
	{
		RendererComponent(enteez::Entity* entity);
		~RendererComponent();
		virtual void Display();
		virtual void ReciveMessage(enteez::Entity* sender, OnComponentEnter<Mesh>& message);
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);
	private:
		enteez::Entity * m_entity;
		bool m_render = false;
	};
}