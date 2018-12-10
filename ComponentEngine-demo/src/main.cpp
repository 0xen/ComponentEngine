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

class UIThread : public ThreadHandler
{
public:
	UIThread() : ThreadHandler(kUIUPS) {}
	virtual void Initilize()
	{

	}
	virtual void Loop()
	{
		engine->UpdateUI();
	}
	virtual void Cleanup()
	{
		std::cout << "UI Shutdown" << std::endl;
	}
};

class LogicThread : public ThreadHandler
{
public:
	LogicThread() : ThreadHandler(kUPS) {}
	virtual void Initilize()
	{
		engine->GetRendererMutex().lock();
		// Load the scene
		engine->LoadScene("../../ComponentEngine-demo/Scenes/GameInstance.xml");
	
		

		camera = engine->GetCameraTransformation();
		camera->Translate(glm::vec3(0.0f, 2.0f, 10.0f));
		engine->GetRendererMutex().unlock();
		engine->UpdateScene();
	}

	virtual void Loop()
	{
		float thread_time = engine->GetLastThreadTime();
		EntityManager& em = engine->GetEntityManager();
		em.ForEach<Transformation, Mesh, RendererComponent>([thread_time](enteez::Entity* entity, Transformation& transformation, Mesh& mesh, RendererComponent& renderer)
		{
			transformation.Rotate(glm::vec3(0.0f, 90.0f * thread_time, 0.0f));
		}, true);
		engine->UpdateScene();
	}

	virtual void Cleanup()
	{
		std::cout << "Logic Shutdown" << std::endl;
	}
};

int main(int argc, char **argv)
{

	engine = Engine::Singlton();
	engine->Start();

	engine->AddThread(new LogicThread(), "Logic");
	engine->AddThread(new UIThread(), "UI");

	// Rendering
	while (engine->Running(60))
	{
		engine->RenderFrame();
		engine->Update();
	}

	engine->Join();
	engine->Stop();
	delete engine;
    return 0;
}
