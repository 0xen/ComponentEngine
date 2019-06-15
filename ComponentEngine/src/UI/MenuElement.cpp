#include <ComponentEngine\UI/MenuElement.hpp>

using namespace ComponentEngine;

ComponentEngine::MenuElement::MenuElement(MenuElementFlags flags, bool enabled) : m_flags(flags), m_enabled(enabled)
{
	m_triggered = false;
}

ComponentEngine::MenuElement::MenuElement(const char* text, std::vector<MenuElement*> children, bool enabled) : m_text(text), m_children(children), m_enabled(enabled)
{
	m_flags = DropDown;
	m_triggered = false;
}

ComponentEngine::MenuElement::MenuElement(const char* text, std::function<void()> on_click, bool enabled) : m_text(text), m_on_click(on_click), m_enabled(enabled)
{
	m_flags = Button;
	m_triggered = false;
}

ComponentEngine::MenuElement::MenuElement(const char * text, std::function<void()> on_click, std::function<void()> post_render, bool enabled) : m_text(text), m_on_click(on_click), m_post_render(post_render), m_enabled(enabled)
{
	m_flags = Button;
	m_triggered = false;
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

std::function<void()> ComponentEngine::MenuElement::PostRender()
{
	return m_post_render;
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

bool ComponentEngine::MenuElement::Triggered()
{
	return m_triggered;
}

void ComponentEngine::MenuElement::Triggered(bool triggered)
{
	m_triggered = triggered;
}
