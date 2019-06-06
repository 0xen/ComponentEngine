#include <ComponentEngine\UI/MenuElement.hpp>

using namespace ComponentEngine;

ComponentEngine::MenuElement::MenuElement(const char* text, std::vector<MenuElement*> children) : m_text(text), m_children(children)
{
}

ComponentEngine::MenuElement::MenuElement(const char* text, std::function<void()> on_click) : m_text(text), m_on_click(on_click)
{
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
