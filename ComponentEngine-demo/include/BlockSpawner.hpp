#pragma once

#include <ComponentEngine\Components\Logic.hpp>

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

	class BlockSpawner : public Logic
	{
		enteez::Entity* m_entity;
		float m_delta;
		float m_timeBetween;
	public:
		BlockSpawner(enteez::Entity* entity);

		void Update(float frame_time);

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);
	};
}