#include <ComponentEngine\UI\UITemplate.hpp>
#include <ComponentEngine\Engine.hpp>
#include <mutex>

using namespace ComponentEngine;

ComponentEngine::UITemplate::UITemplate(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, std::function<void()> preDraw, std::function<void()> contents, bool open) :
	m_contents(contents),
	m_preDraw(preDraw),
	UIBase(title, flags, displayFlags, open)
{

}

void ComponentEngine::UITemplate::PreDraw()
{
	m_preDraw();
}


void ComponentEngine::UITemplate::Contents()
{
	m_contents();
}