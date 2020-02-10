#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>

#include <ComponentEngine\ThreadManager.hpp>

#include <ComponentEngine\UI\MenuElement.hpp>
#include <ComponentEngine\UI\UITemplate.hpp>


#include <renderer\vulkan\VulkanRaytracePipeline.hpp>
#include <renderer\vulkan\VulkanAcceleration.hpp>
#include <renderer\vulkan\VulkanGraphicsPipeline.hpp>
#include <renderer\vulkan\VulkanModelPool.hpp>

#include <EnteeZ\EnteeZ.hpp>
#include <KeyboardMovment.hpp>
#include <MouseDrag.hpp>
#include <CameraDolly.hpp>
#include <iostream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* engine;

VulkanRaytracePipeline* ray_pipeline = nullptr;

void RegisterCustomComponents()
{
	engine->RegisterComponentBase("Keyboard Movement", KeyboardMovment::EntityHookDefault);
	engine->RegisterComponentBase("Mouse Drag", MouseDrag::EntityHookDefault);
	engine->RegisterComponentBase("Camera Dolly", CameraDolly::EntityHookDefault);

	engine->RegisterBase<KeyboardMovment, Logic, UI, IO>();
	engine->RegisterBase<MouseDrag, Logic, UI, IO>();
	engine->RegisterBase<CameraDolly, Logic, UI, IO>();
}

void AddUIWindows()
{


	engine->GetUIManager()->AddElement(new UITemplate("Status", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking, PlayState::Play, [&]()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		int titlebarHeight = ImGui::GetFontSize() + (style.FramePadding.y * 2);
		ImGui::SetNextWindowPos(ImVec2(titlebarHeight, titlebarHeight));
	},
		[&]()
	{
		std::vector<WorkerTask*>& tasks = engine->GetThreadManager()->GetSchedualedTasks();

		for (int i = 0; i < tasks.size(); i++)
		{
			ImGui::PushID(i);
			WorkerTask*& task = tasks[i];
			ImGui::Text("%s: ", task->name.c_str());

			float processTime = task->taskActivity[task->taskActivity.size() - 1];
			ImGui::SameLine();
			ImGui::Text("%.2f", processTime);

			ImGui::PopID();
		}
	}));
	engine->GetUIManager()->AddElement(new UITemplate("Configuration", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking, PlayState::Play, [&]()
	{},[&](){	engine->GetMainCamera()->Display();		}));


}


int main(int argc, char **argv)
{
	engine = Engine::Singlton();
	
	int flags = 0;// EngineFlags::ReleaseBuild;
	engine->SetFlag(flags);

	engine->Start();
	RegisterCustomComponents();
	AddUIWindows();

	if ((flags & EngineFlags::ReleaseBuild) == EngineFlags::ReleaseBuild)
	{
		// Load the scene
		engine->GetThreadManager()->AddTask([&](float frameTime) {
			//engine->LoadScene("../Room.obj");
			engine->LoadScene("../Head.bin");
		});
	}

	while (engine->Running())
	{
		engine->Update();
	}

	engine->Stop();
	delete engine;

    return 0;
}