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
	
		/*EntityManager& em = engine->GetEntityManager();
		int width = 10;
		int height = 10;
		int depth = 10;
		for (int x = -width / 2; x < width / 2; x++)
		{
			for (int y = -height / 2; y < height / 2; y++)
			{

				for (int z = -depth; z < 0; z++)
				{
					enteez::Entity* ent = em.CreateEntity("Cube");
					{
						enteez::ComponentWrapper<Transformation>* trans_wrapper = ent->AddComponent<Transformation>(ent);
						trans_wrapper->SetName("Transformation");
						trans_wrapper->Get().Translate(glm::vec3(x, y, z));
						trans_wrapper->Get().Scale(glm::vec3(0.8f, 0.8f, 0.8f));
					}
					{
						enteez::ComponentWrapper<Mesh>* mesh = ent->AddComponent<Mesh>(ent, "../../ComponentEngine-demo/Resources/Models/cube.obj");
						mesh->SetName("Mesh");
					}
					{
						enteez::ComponentWrapper<RendererComponent>* renderer = ent->AddComponent<RendererComponent>(ent);
						renderer->SetName("Renderer");
					}
				}
			}
		}*/

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

	// Render a frame so you know it has not crashed xD
	engine->RenderFrame();

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
