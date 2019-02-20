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

	class MouseMovment : public Logic , public UI
	{
		enum MouseStatus
		{
			Locked,    // Locked onto the scene
			Unlocked,  // Unlocked but still have scene focus
			UnlockedUI // Unlocked and interacting with the ui
		};
		enteez::Entity* m_entity;
		float m_speed;
		unsigned int m_unlockCameraKey;
		bool m_unlockCameraKeyFocus;
		MouseStatus status;
	public:
		MouseMovment(enteez::Entity* entity);
		virtual void Update(float frame_time);
		virtual void Display();
		static void EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);
	};
}