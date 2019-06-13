#include <ComponentEngine\UI/MenuElement.hpp>

using namespace ComponentEngine;

ComponentEngine::MenuElement::MenuElement(MenuElementFlags flags, bool enabled) : m_flags(flags), m_enabled(enabled)
{
}

ComponentEngine::MenuElement::MenuElement(const char* text, std::vector<MenuElement*> children, bool enabled) : m_text(text), m_children(children), m_enabled(enabled)
{
	m_flags = DropDown;
}

ComponentEngine::MenuElement::MenuElement(const char* text, std::function<void()> on_click, bool enabled) : m_text(text), m_on_click(on_click), m_enabled(enabled)
{
	m_flags = Button;
}

ComponentEngine::MenuElement::~MenuElement()
{
	for (auto&e : m_children)
	{
		delete e;
	}
}

void ComponentEngine::MenuElement::AddChild(MenuElement * element)
{
	m_children.push_back(element);
}

std::vector<MenuElement*>& ComponentEngine::MenuElement::GetChildren()
{
	return m_children;
}

std::function<void()> ComponentEngine::MenuElement::OnClick()
{
	return m_on_click;
}

const char * ComponentEngine::MenuElement::GetText()
{
	return m_text;
}

MenuElementFlags ComponentEngine::MenuElement::GetFlags()
{
	return m_flags;
}

bool ComponentEngine::MenuElement::Enabled()
{
	return m_enabled;
}

void ComponentEngine::MenuElement::SetEnabled(bool enabled)
{
	m_enabled = enabled;
}
