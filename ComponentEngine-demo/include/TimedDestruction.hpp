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

	class TimedDestruction : public Logic
	{
		enteez::Entity* m_entity;
		float m_delta;
		float m_timetillDealth;
		void DestroyEntity(enteez::Entity* entity);
	public:
		TimedDestruction(enteez::Entity* entity);

		virtual void Update(float frame_time);

		static void EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);
	};
}