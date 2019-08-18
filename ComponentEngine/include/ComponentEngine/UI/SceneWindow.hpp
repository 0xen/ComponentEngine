#pragma once

#include <ComponentEngine\UI\UIBase.hpp>

namespace ComponentEngine
{
	class SceneWindow : public UIBase
	{
	public:
		SceneWindow(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open = true);

		virtual void Contents();
	};
}