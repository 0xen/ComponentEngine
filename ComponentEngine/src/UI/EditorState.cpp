#include <ComponentEngine\UI\EditorState.hpp>
#include <ComponentEngine\Engine.hpp>
#include <mutex>

using namespace ComponentEngine;

ComponentEngine::EditorState::EditorState(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open) :
	UIBase(title, flags, displayFlags, open)
{

}

void ComponentEngine::EditorState::PreDraw()
{
	ImGuiStyle& style = ImGui::GetStyle();
	int titlebarHeight = ImGui::GetFontSize() + (style.FramePadding.y * 2);

	static const int windowWidth = 47;
	ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2) - windowWidth, titlebarHeight));
}

void ComponentEngine::EditorState::Contents()
{
	if (Engine::Singlton()->GetPlayState() == PlayState::Play)
	{
		if (ImGui::Button("||"))
		{
			Engine::Singlton()->SetPlayState(PlayState::Editor);
		}
	}
	else
	{
		if (ImGui::ArrowButton("##left", ImGuiDir_Right))
		{
			Engine::Singlton()->SetPlayState(PlayState::Play);
		}
		//ImGui::SameLine();
		//ImGui::Checkbox("Fullscreen on play", &m_fullscreenOnPlay);
	}
}
