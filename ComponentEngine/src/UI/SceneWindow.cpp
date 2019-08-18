#include <ComponentEngine\UI\SceneWindow.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\UI/UIManager.hpp>

#include <renderer\NativeWindowHandle.hpp>
#include <mutex>

using namespace ComponentEngine;

ComponentEngine::SceneWindow::SceneWindow(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open) :
	UIBase(title, flags, displayFlags, open)
{

}

void ComponentEngine::SceneWindow::Contents()
{



	NativeWindowHandle* windowHandle = Engine::Singlton()->GetWindowHandle();

	ImVec2 dim = ImGui::GetWindowSize();
	UIManager::DrawScalingImage(0, windowHandle->width, windowHandle->height, dim.x, dim.y);




}
