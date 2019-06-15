#pragma once

#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
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
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
	};
}