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

	class KeyboardMovment : public Logic , public UI
	{
		enteez::Entity* m_entity;
		struct KeyInstance
		{
			unsigned int key;
			bool focused;
		};
		/*
		0 : Forward
		1 : Back
		2 : Left
		3: Right
		*/
		KeyInstance keys[4];
		float m_speed;
	public:
		KeyboardMovment(enteez::Entity* entity);
		virtual void Update(float frame_time);
		virtual void EditorUpdate(float frame_time);
		virtual void Display();
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);
	};
}