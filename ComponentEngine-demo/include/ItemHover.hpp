#pragma once

#include <ComponentEngine\Components\Logic.hpp>
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

	class ItemHover : public Logic , public UI
	{
		enteez::Entity* m_entity;
		float m_delta_time;
		float m_move_distance;
		float m_last_move_distance;
		float m_spin_speed;
		float m_animation_duration_scalar;
		bool m_running;
	public:
		ItemHover(enteez::Entity* entity, bool running = true);
		virtual void Update(float frame_time);
		virtual void Display();
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);
	};
}