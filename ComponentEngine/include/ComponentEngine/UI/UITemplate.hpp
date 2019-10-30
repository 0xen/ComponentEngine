#pragma once

#include <ComponentEngine\UI\UIBase.hpp>
#include <functional>

namespace ComponentEngine
{
	class UITemplate : public UIBase
	{
	public:
		UITemplate(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, std::function<void()> preDraw, std::function<void()> contents, bool open = true);

		virtual void PreDraw();
		virtual void Contents();

	private:
		std::function<void()> m_contents;
		std::function<void()> m_preDraw;
	};
}