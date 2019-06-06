#pragma once

#include <ComponentEngine\UI\UIBase.hpp>

namespace ComponentEngine
{
	class Console : public UIBase
	{
	public:
		Console(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open = true);

		virtual void Contents();
	};
}