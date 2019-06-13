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

namespace pugi
{
	class xml_node;
}

namespace ComponentEngine
{

	class Flamable : public UI, public MsgRecive<OnCollisionEnter>
	{
		enteez::Entity* m_entity;
		bool m_onFire;
	public:
		Flamable(enteez::Entity* entity);
		bool OnFire();
		void SetOnFire(bool fire);

		virtual void Display();
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);

		virtual void ReciveMessage(enteez::Entity* sender, OnCollisionEnter& message);
	};
}