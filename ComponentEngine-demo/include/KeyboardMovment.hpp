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
		KeyInstance keys[4];
		float m_speed;
		bool m_ignore_axis[3];
		bool m_local_movment;
	public:
		KeyboardMovment(enteez::Entity* entity);
		virtual void Update(float frame_time);
		virtual void EditorUpdate(float frame_time);
		virtual void Display();

		virtual void Load(std::ifstream& in);
		virtual void Save(std::ofstream& out);
		virtual unsigned int PayloadSize();
		virtual bool DynamiclySized();
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
	};
}