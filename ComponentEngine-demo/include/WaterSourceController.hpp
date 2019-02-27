#pragma once

#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
#include <ComponentEngine\Components\MsgRecive.hpp>

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

	class WaterSourceController : public UI, public MsgRecive<OnCollisionEnter>
	{
		enteez::Entity* m_entity;
	public:
		WaterSourceController(enteez::Entity* entity);

		virtual void Display();
		static void EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);

		virtual void ReciveMessage(enteez::Entity* sender, OnCollisionEnter& message);
	};
}