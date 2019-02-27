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

	class BlockMoveController : public Logic , public UI
	{
		enteez::Entity* m_entity;
		float m_speed;
	public:
		BlockMoveController(enteez::Entity* entity);

		virtual void Update(float frame_time);
		virtual void Display();
		static void EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);
	};
}