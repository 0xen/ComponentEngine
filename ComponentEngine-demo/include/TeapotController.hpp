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

	class TeapotController : public UI, public MsgRecive<OnCollisionEnter>, public MsgRecive<OnCollisionExit>
	{
		enteez::Entity* m_entity;
	public:
		TeapotController(enteez::Entity* entity);

		virtual void Display();
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);

		virtual void ReciveMessage(enteez::Entity* sender, OnCollisionEnter& message);
		virtual void ReciveMessage(enteez::Entity* sender, OnCollisionExit& message);
	};
}