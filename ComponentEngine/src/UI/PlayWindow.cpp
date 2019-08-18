#include <ComponentEngine\UI\PlayWindow.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\UI/UIManager.hpp>

#include <renderer\NativeWindowHandle.hpp>

#include <mutex>

using namespace ComponentEngine;

ComponentEngine::PlayWindow::PlayWindow(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open) :
	UIBase(title, flags, displayFlags, open)
{

}

void ComponentEngine::PlayWindow::PreDraw()
{
	NativeWindowHandle* windowHandle = Engine::Singlton()->GetWindowHandle();
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(windowHandle->width, windowHandle->height));
}

void ComponentEngine::PlayWindow::Contents()
{


	ImVec2 dim = ImGui::GetWindowSize();
	ImGui::Image((ImTextureID)0, dim, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));


	//UIManager::DrawScalingImage(0, 1080, 720, dim.x, dim.y);




}
