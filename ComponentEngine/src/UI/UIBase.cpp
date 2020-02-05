#include <ComponentEngine\UI/UIBase.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\UI\UIManager.hpp>

using namespace ComponentEngine;

ComponentEngine::UIBase::UIBase(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open) : m_title(title), m_flags(flags), m_displayFlags(displayFlags), m_open(open)
{
}

void ComponentEngine::UIBase::Render()
{
	if (!m_open) return;
	PreDraw();
	if (ImGui::Begin(m_title, &m_open, m_flags))
	{
		if (ImGui::IsWindowHovered())
			Engine::Singlton()->SetHoveredWindowName(m_title);
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

bool ComponentEngine::UIBase::IsOpen()
{
	return m_open;
}

void ComponentEngine::UIBase::Open(bool open)
{
	m_open = open;
}

const char * ComponentEngine::UIBase::GetTitle()
{
	return m_title;
}
