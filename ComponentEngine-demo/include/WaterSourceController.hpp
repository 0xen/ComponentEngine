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

	class WaterSourceController : public UI, public MsgRecive<OnCollisionEnter>
	{
		enteez::Entity* m_entity;
	public:
		WaterSourceController(enteez::Entity* entity);

		virtual void Display();
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);

		virtual void ReciveMessage(enteez::Entity* sender, OnCollisionEnter& message);
	};
}