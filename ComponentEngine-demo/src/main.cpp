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
#include <iostream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* engine;

VulkanRaytracePipeline* ray_pipeline = nullptr;

void RegisterCustomComponents()
{
	engine->RegisterComponentBase("Keyboard Movement", KeyboardMovment::EntityHookDefault);

	engine->RegisterBase<KeyboardMovment, Logic, UI, IO>();
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
}

#include <iostream>
#include <cmath>
#include <thread>
#include <future>
#include <functional>



int main(int argc, char **argv)
{


	/*std::packaged_task<int()> task(std::bind([](int a, int b)
	{
		return std::pow(a, b);
	},
	2,1));
	std::future<int> result = task.get_future();




	// Multithread safe check to see if the result is ready
	if (result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		// Get the result from the thread
		std::cout << "task_lambda test 1:\t" << result.get() << '\n';
	}



	// To be preformed by the thread
	task();


	// Multithread safe check to see if the result is ready
	if (result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		// Get the result from the thread
		std::cout << "task_lambda test 2:\t" << result.get() << '\n';
	}




	exit(0);*/



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
			engine->LoadScene("../Room.obj");
		});

	}
	engine->GetThreadManager()->AddTask([&](float frameTime)
	{
		engine->LoadScene("../TransparencyTest.bin");
	});



	while (engine->Running())
	{
		engine->Update();
	}

	engine->Stop();
	delete engine;

    return 0;
}