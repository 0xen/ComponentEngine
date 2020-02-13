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

	class Rotate : public Logic, public UI, public IO
	{
		enteez::Entity* m_entity;

		float m_rotateSpeed;
		int m_axis_selection;
		glm::vec3 m_axis;


	public:
		Rotate(enteez::Entity* entity);
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