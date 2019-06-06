#pragma once

#include <ComponentEngine\UI\UIBase.hpp>

namespace ComponentEngine
{
	class ThreadingWindow : public UIBase
	{
	public:
		ThreadingWindow(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open = true);

		virtual void Contents();
	};
}