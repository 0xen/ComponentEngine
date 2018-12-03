#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <iostream>
using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;
Engine* engine;
// Updates per second
const int kUPS = 30;

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
	float thread_time = 0.0f;
	while (engine->Running())
	{
		float start = engine->GetThreadTime();
		thread_time += start;
		em.ForEach<Transformation,Mesh>([thread_time, camera](enteez::Entity* entity, Transformation& transformation, Mesh& mesh)
		{
			if (&transformation != camera)
			{
				transformation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f * thread_time);
			}
		}, true);
		engine->GetRendererMutex().lock();
		engine->UpdateUI();
		//std::cout << "L:" << thread_time << std::endl;
		engine->UpdateScene();
		engine->GetRendererMutex().unlock();


		float stop = engine->GetThreadTime();
		thread_time = stop;
		// Calculate the amount of time we should pause for
		int pause_time = (int)(1000 / kUPS - ((1000 / (int)(1.0f / stop)) - (1000 / (int)(1.0f / start))));
		std::this_thread::sleep_for(std::chrono::milliseconds(pause_time));
		thread_time += engine->GetThreadTime();
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
	EntityManager& em = engine->GetEntityManager();

	// Rendering
	while (engine->Running())
	{
		engine->GetRendererMutex().lock();

		engine->Update();


		//std::cout << "R:" << engine->GetFrameTime() << std::endl;
		engine->RenderFrame();
		engine->GetRendererMutex().unlock();
	}
	delete engine;
    return 0;
}
