#pragma once

#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\IO.hpp>

#include <glm\glm.hpp>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
}

namespace ComponentEngine
{

	class MouseDrag : public Logic, public UI, public IO
	{
		enteez::Entity* m_entity;

		float m_rotateSpeed;
		glm::vec3 offset;
		bool m_flip_x;
		bool m_flip_y;
		bool m_lock_x;
		bool m_lock_y;


	public:
		MouseDrag(enteez::Entity* entity);
		virtual void Update(float frame_time);
		virtual void EditorUpdate(float frame_time);
		virtual void Display();

		virtual void Load(pugi::xml_node& node);
		virtual void Save(pugi::xml_node& node);

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
	};
}