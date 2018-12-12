#pragma once

#include <ComponentEngine\Components\Logic.hpp>
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

	class ItemHover : public Logic , public UI
	{
		enteez::Entity* m_entity;
		float m_delta_time;
		float m_move_distance;
		float m_last_move_distance;
		float m_spin_speed;
	public:
		ItemHover(enteez::Entity* entity);
		virtual void Update(float frame_time);
		virtual void Display();
		static void EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);
	};
}