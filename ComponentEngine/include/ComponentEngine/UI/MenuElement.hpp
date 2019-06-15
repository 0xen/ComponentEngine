#pragma once

#include <functional>
#include <vector>

namespace ComponentEngine
{

	enum MenuElementFlags
	{
		Spacer,


		// Internal use only
		DropDown,
		Button
	};

	class MenuElement
	{
	public:
		MenuElement(MenuElementFlags flags, bool enabled = true);
		MenuElement(const char* text, std::vector<MenuElement*> children, bool enabled = true);
		MenuElement(const char* text, std::function<void()> on_click, bool enabled = true);
		MenuElement(const char* text, std::function<void()> on_click, std::function<void()> post_render, bool enabled = true);
		~MenuElement();
		void AddChild(MenuElement* element);

		std::vector<MenuElement*>& GetChildren();

		std::function<void()> OnClick();

		std::function<void()> PostRender();

		const char* GetText();

		MenuElementFlags GetFlags();

		bool Enabled();

		void SetEnabled(bool enabled);

		bool Triggered();
		void Triggered(bool triggered);

	private:
		bool m_enabled;
		bool m_triggered;
		const char* m_text;
		std::vector<MenuElement*> m_children;
		std::function<void()> m_on_click = nullptr;
		std::function<void()> m_post_render  =nullptr;
		MenuElementFlags m_flags;
	};

}