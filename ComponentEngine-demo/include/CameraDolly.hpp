#pragma once

#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\IO.hpp>

#include <glm\glm.hpp>

#include <vector>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
}

namespace ComponentEngine
{

	struct CameraDollySnapshot
	{
		float transitionTime;

		float fov;
	};

	class CameraDolly : public Logic, public UI, public IO
	{
		enteez::Entity* m_entity;

		std::vector<CameraDollySnapshot> snapshots;

		float deltaTime;
		int currentIndex;

	public:
		CameraDolly(enteez::Entity* entity);

		virtual void Update(float frame_time);
		virtual void EditorUpdate(float frame_time);
		virtual void Display();

		virtual void Load(std::ifstream& in);
		virtual void Save(std::ofstream& out);
		virtual unsigned int PayloadSize();
		virtual bool DynamiclySized();
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);

		float lerp(float a, float b, float f);
	};
}