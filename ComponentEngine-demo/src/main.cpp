#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <iostream>
using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;
Engine* engine;
void LogicThread()
{
	engine->GetRendererMutex().lock();
	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();
	// Load the scene
	engine->LoadScene("../../ComponentEngine-demo/Scenes/GameInstance.xml");
	Transformation* camera = engine->GetCameraTransformation();
	camera->Translate(glm::vec3(0.0f, 0.0f, 6.0f));
	engine->GetRendererMutex().unlock();
	// Logic Updating
	while (engine->Running())
	{
		float thread_time = engine->GetThreadTime();
		em.ForEach<Transformation,Mesh>([thread_time, camera](enteez::Entity* entity, Transformation& transformation, Mesh& mesh)
		{
			if (&transformation != camera)
			{
				transformation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f * thread_time);
			}
		}, true);
		engine->GetRendererMutex().lock();
		//std::cout << "L:" << thread_time << std::endl;
		engine->UpdateScene();
		engine->GetRendererMutex().unlock();
	}
}

#include <direct.h>
#define GetCurrentDir _getcwd

std::string GetCurrentWorkingDir(void)
{
	char buff[FILENAME_MAX];
	GetCurrentDir(buff, FILENAME_MAX);
	std::string current_working_dir(buff);
	return current_working_dir;
}



int main(int argc, char **argv)
{


	std::cout << GetCurrentWorkingDir() << std::endl;



	engine = Engine::Singlton();
	engine->Start(LogicThread);
	// Rendering
	while (engine->Running())
	{
		engine->Update();
		engine->GetRendererMutex().lock();

		{
			ImGui::NewFrame();
			{
				//ImGui::ShowTestWindow();
			}
			{
				if (ImGui::BeginMainMenuBar())
				{
					if (ImGui::BeginMenu("File"))
					{
						if (ImGui::MenuItem("Exit")) 
						{
							engine->Stop();
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Edit"))
					{
						if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
						if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
						ImGui::Separator();
						if (ImGui::MenuItem("Cut", "CTRL+X")) {}
						if (ImGui::MenuItem("Copy", "CTRL+C")) {}
						if (ImGui::MenuItem("Paste", "CTRL+V")) {}
						ImGui::EndMenu();
					}
					ImGui::EndMainMenuBar();
				}
			}
			{
				bool open = true;
				ImVec2 window_pos = ImVec2(10.0f, 25.0f);
				ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
				if (ImGui::Begin("Example: Simple Overlay", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
				{
					ImGui::Text("FPS:%f  TPS:%f", 1.0f / engine->GetFrameTime(), 0.0f);
					ImGui::Separator();
				}
				ImGui::End();
			}


			ImGui::Render();
		}


		engine->UpdateUI();
		//std::cout << "R:" << engine->GetFrameTime() << std::endl;
		engine->RenderFrame();
		engine->GetRendererMutex().unlock();
	}
	delete engine;
    return 0;
}
