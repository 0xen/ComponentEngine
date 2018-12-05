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
const int kUPS = 60;
// UI updates per second
const int kUIUPS = 30;

// Camera
Transformation* camera;

void UIThreadLoop()
{
	engine->Sync(kUIUPS);
	engine->UpdateUI();
}

void UIThread()
{
	while (engine->Running())
	{
		UIThreadLoop();
	}
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

void LogicThreadLoop()
{
	EntityManager& em = engine->GetEntityManager();
	float thread_time = engine->Sync(kUPS);
	em.ForEach<Transformation, Mesh>([thread_time](enteez::Entity* entity, Transformation& transformation, Mesh& mesh)
	{
		if (&transformation != camera)
		{
			transformation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f * thread_time);
		}
	}, true);
	engine->GetRendererMutex().lock();
	engine->UpdateScene();
	engine->GetRendererMutex().unlock();

}

void LogicThread()
{
	LogicThreadInit();
	// Logic Updating
	while (engine->Running())
	{
		LogicThreadLoop();
	}
}

int main(int argc, char **argv)
{
	engine = Engine::Singlton();
	engine->Start();
	engine->AddThread(LogicThread);
	engine->AddThread(UIThread);
	EntityManager& em = engine->GetEntityManager();
	// Rendering
	while (engine->Running())
	{
		engine->RenderFrame();
		engine->Update();
	}
	engine->Stop();
	engine->Join();
	delete engine;
    return 0;
}
