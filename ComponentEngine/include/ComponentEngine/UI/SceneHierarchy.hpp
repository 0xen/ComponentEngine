#pragma once

#include <ComponentEngine\UI\UIBase.hpp>

namespace enteez
{
	class Entity;
}

namespace ComponentEngine
{
	class SceneHierarchy : public UIBase
	{
	public:
		SceneHierarchy(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open = true);

		virtual void Contents();

	private:
		void AddEntityDialougeMenu(enteez::Entity* parent);
		void AddEntity(enteez::Entity* parent);
		void RenderEntityTreeNode(enteez::Entity * entity);
	};
}