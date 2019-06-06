#pragma once

#include <ComponentEngine\UI\UIBase.hpp>

namespace enteez
{
	class Entity;
};

namespace ComponentEngine
{
	class ComponentHierarchy : public UIBase
	{
	public:
		ComponentHierarchy(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open = true);

		virtual void Contents();

	private:

		void DestroyEntity(enteez::Entity * entity);

		void RenderComponentTitleBar(enteez::Entity * entity);

		void RenderComponentList(enteez::Entity* entity);
	};
}