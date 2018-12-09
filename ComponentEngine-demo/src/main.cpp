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
// UI updates per second
const int kUIUPS = 15;

// Camera
Transformation* camera;

void UIThreadLoop()
{
	engine->UpdateUI();
}

void UIThread()
{
	while (engine->Running(kUIUPS))
	{
		UIThreadLoop();
	}
	std::cout << "b" << std::endl;
}

void LogicThreadInit()
{
	engine->GetRendererMutex().lock();
	// Load the scene
	engine->LoadScene("../../ComponentEngine-demo/Scenes/GameInstance.xml");
	
	camera = engine->GetCameraTransformation();
	camera->Translate(glm::vec3(0.0f, 2.0f, 10.0f));
	engine->GetRendererMutex().unlock();
}

void LogicThreadLoop(float thread_time)
{
	EntityManager& em = engine->GetEntityManager();
	em.ForEach<Transformation, Mesh, RendererComponent>([thread_time](enteez::Entity* entity, Transformation& transformation, Mesh& mesh, RendererComponent& renderer)
	{

	}, true);
	engine->UpdateScene();
}

void LogicThread()
{
	LogicThreadInit();
	// Logic Updating
	engine->UpdateScene();
	while (engine->Running(kUPS))
	{
		LogicThreadLoop(engine->GetLastThreadTime());
	}
	std::cout << "a" << std::endl;
}

int main(int argc, char **argv)
{

	engine = Engine::Singlton();
	engine->Start();

	EntityManager& em = engine->GetEntityManager();
	{
		/*LogicThreadInit();
		// Rendering
		while (engine->Running())
		{
			UIThreadLoop();
			LogicThreadLoop(engine->GetLastThreadTime());
			engine->RenderFrame();
			engine->Update();
		}*/
	}
	{
		engine->AddThread(LogicThread, "Logic");
		engine->AddThread(UIThread, "UI");
		EntityManager& em = engine->GetEntityManager();
		// Rendering
		while (engine->Running(60))
		{
			engine->RenderFrame();
			engine->Update();
		}
	}


	engine->Join();
	engine->Stop();
	delete engine;
    return 0;
}
