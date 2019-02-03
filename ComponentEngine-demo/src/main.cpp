#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <ItemHover.hpp>
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
		engine->LoadScene("../../ComponentEngine-demo/GameInstance.xml");
		camera = engine->GetCameraTransformation();
		camera->Translate(glm::vec3(0.0f, 2.0f, 10.0f));
		engine->GetRendererMutex().unlock();
		engine->UpdateScene();
	}

	virtual void Loop()
	{
		float thread_time = engine->GetLastThreadTime();
		EntityManager& em = engine->GetEntityManager();

		for (auto e : em.GetEntitys())
		{
			e->ForEach<Logic>([&](enteez::Entity* entity, Logic& logic)
			{
				logic.Update(thread_time);
			});
		}
		

		if (engine->KeyDown(SDL_SCANCODE_W))
			engine->GetCameraTransformation()->MoveLocalZ(-10.0f * thread_time);
		if (engine->KeyDown(SDL_SCANCODE_S))
			engine->GetCameraTransformation()->MoveLocalZ(10.0f * thread_time);


		if (engine->KeyDown(SDL_SCANCODE_A))
			engine->GetCameraTransformation()->RotateWorldY(glm::radians(90.0f*thread_time));
		if (engine->KeyDown(SDL_SCANCODE_D))
			engine->GetCameraTransformation()->RotateWorldY(glm::radians(-90.0f*thread_time));


		engine->UpdateScene();
	}

	virtual void Cleanup()
	{
		std::cout << "Logic Shutdown" << std::endl;
	}
};



void RegisterCustomComponents()
{
	engine->RegisterComponentBase("ItemHover", ItemHover::EntityHookDefault, ItemHover::EntityHookXML);
	engine->RegisterBase<ItemHover, Logic, UI>();
}
	


int main(int argc, char **argv)
{
	engine = Engine::Singlton();
	engine->Start();

	RegisterCustomComponents();

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
