#pragma once

#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
#include <ComponentEngine\Components\MsgRecive.hpp>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
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
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
	};
}