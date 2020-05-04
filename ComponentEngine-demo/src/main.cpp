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
#include <Rotate.hpp>
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
	engine->RegisterComponentBase("Rotate", Rotate::EntityHookDefault);

	engine->RegisterBase<KeyboardMovment, Logic, UI, IO>();
	engine->RegisterBase<MouseDrag, Logic, UI, IO>();
	engine->RegisterBase<CameraDolly, Logic, UI, IO>();
	engine->RegisterBase<Rotate, Logic, UI, IO>();
}

void AddUIWindows()
{


	/*engine->GetUIManager()->AddElement(new UITemplate("Status", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking, PlayState::Play, [&]()
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
	}));*/
	//engine->GetUIManager()->AddElement(new UITemplate("Configuration", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking, PlayState::Play, [&]()
	//{},[&](){	engine->GetMainCamera()->Display();		}));


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
	engine->GetUIManager()->AddMenuElement(new MenuElement("Spawn",
		{
			new MenuElement("Large Tree",[&]
			{
				Entity* tree = engine->GetEntityManager().CreateEntity("Large Tree");
				Transformation::EntityHookDefault(*tree);
				Transformation& transform = tree->GetComponent<Transformation>();
				float x = ((rand() % 1000) * 0.016f) - 8.0f;
				float y = ((rand() % 1000) * 0.016f) - 8.0f;
				transform.Translate(glm::vec3(x, 0, y));

				Mesh::EntityHookDefault(*tree);
				Mesh& mesh = tree->GetComponent<Mesh>();
				mesh.ChangePath("../Resources/Models/Trees/Red Maple Young/HighPoly/ReadMaple.obj");
			}),
		new MenuElement("Grass",[&]
			{
				for (int i = 0; i < 10; i++)
				{
					Entity* tree = engine->GetEntityManager().CreateEntity("Grass");
					Transformation::EntityHookDefault(*tree);
					Transformation& transform = tree->GetComponent<Transformation>();
					float x = ((rand() % 1000) * 0.016f) - 8.0f;
					float y = ((rand() % 1000) * 0.016f) - 8.0f;
					transform.Translate(glm::vec3(x, 0, y));

					Mesh::EntityHookDefault(*tree);
					Mesh& mesh = tree->GetComponent<Mesh>();
					mesh.ChangePath("../Resources/Models/Trees/Backyard Grass/HighPoly/Grass.obj");
				}
			}),
			new MenuElement("Boston Fern",[&]
			{
				for (int i = 0; i < 10; i++)
				{
					Entity* tree = engine->GetEntityManager().CreateEntity("Boston Fern");
					Transformation::EntityHookDefault(*tree);
					Transformation& transform = tree->GetComponent<Transformation>();
					float x = ((rand() % 1000) * 0.016f) - 8.0f;
					float y = ((rand() % 1000) * 0.016f) - 8.0f;
					transform.Translate(glm::vec3(x, 0, y));

					Mesh::EntityHookDefault(*tree);
					Mesh& mesh = tree->GetComponent<Mesh>();
					mesh.ChangePath("../Resources/Models/Trees/Boston Fern/HighPoly/BostonFern.obj");
				}
			}),
			new MenuElement("PBR Boy",[&]
			{
				for (int i = 0; i < 10; i++)
				{
					Entity* tree = engine->GetEntityManager().CreateEntity("PBR Boy");
					Transformation::EntityHookDefault(*tree);
					Transformation& transform = tree->GetComponent<Transformation>();
					float x = ((rand() % 1000) * 0.002f) - 1.0f;
					float y = ((rand() % 1000) * 0.002f) - 1.0f;
					transform.Translate(glm::vec3(x, 0, y));
					transform.RotateWorldY((rand() % 1000) * 2.234f);
					Mesh::EntityHookDefault(*tree);
					Mesh& mesh = tree->GetComponent<Mesh>();
					mesh.ChangePath("../Resources/Models/PBRBoys/PBRBoy.obj");
				}
			})
		}
	));
	
	while (engine->Running())
	{
		engine->Update();
	}

	engine->Stop();
	delete engine;

    return 0;
}