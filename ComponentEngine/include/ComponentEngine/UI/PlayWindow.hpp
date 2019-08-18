#pragma once

#include <ComponentEngine\UI\UIBase.hpp>

namespace ComponentEngine
{
	class PlayWindow : public UIBase
	{
	public:
		PlayWindow(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open = true);

		virtual void PreDraw();
		virtual void Contents();
	};
}