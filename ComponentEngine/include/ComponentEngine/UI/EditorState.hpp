#pragma once

#include <ComponentEngine\UI\UIBase.hpp>

namespace ComponentEngine
{
	class EditorState : public UIBase
	{
	public:
		EditorState(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open = true);

		virtual void PreDraw();
		virtual void Contents();
	};
}