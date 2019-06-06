#pragma once

#include <functional>
#include <vector>

namespace ComponentEngine
{

	class MenuElement
	{
	public:
		MenuElement(const char* text,std::vector<MenuElement*> children);
		MenuElement(const char* text, std::function<void()> on_click);
		~MenuElement();
		void AddChild(MenuElement* element);

		std::vector<MenuElement*>& GetChildren();

		std::function<void()> OnClick();

		const char* GetText();

	private:
		const char* m_text;
		std::vector<MenuElement*> m_children;
		std::function<void()> m_on_click = nullptr;
	};

}