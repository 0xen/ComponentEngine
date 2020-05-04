#pragma once

#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\IO.hpp>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
}

namespace ComponentEngine
{

	class KeyboardMovment : public Logic, public UI, public IO
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
		float m_speed;
		bool m_ignore_axis_x;
		bool m_ignore_axis_y;
		bool m_ignore_axis_z;
		bool m_local_movment;
		bool m_inEditor;
		KeyInstance keys0;
		KeyInstance keys1;
		KeyInstance keys2;
		KeyInstance keys3;
		KeyInstance keys4;
		KeyInstance keys5; 
	public:
		KeyboardMovment(enteez::Entity* entity);
		virtual void Update(float frame_time);
		virtual void EditorUpdate(float frame_time);
		virtual void Display();

		virtual void Load(pugi::xml_node& node);
		virtual void Save(pugi::xml_node& node);

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
	};
}