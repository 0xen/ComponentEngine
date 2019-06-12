#include <ComponentEngine\UI/UIBase.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\UI\UIManager.hpp>

using namespace ComponentEngine;

ComponentEngine::UIBase::UIBase(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open) : m_title(title), m_flags(flags), m_displayFlags(displayFlags), m_open(open)
{
}

void ComponentEngine::UIBase::Render()
{
	PreDraw();
	if (ImGui::Begin(m_title, &m_open, m_flags))
	{
		Contents();
	}
	ImGui::End();
}

void ComponentEngine::UIBase::PreDraw()
{
}

UIDisplayFlags ComponentEngine::UIBase::GetDisplayFlags()
{
	return m_displayFlags;
}
