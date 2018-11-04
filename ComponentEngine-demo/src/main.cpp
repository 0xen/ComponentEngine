#include <ComponentEngine\Engine.hpp>
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
	camera->Translate(glm::vec3(0.0f, 0.0f, 10.0f));
	engine->GetRendererMutex().unlock();
	// Logic Updating
	while (engine->Running())
	{
		float thread_time = engine->GetThreadTime();
		em.ForEach<Transformation>([thread_time, camera](enteez::Entity* entity, Transformation& transformation)
		{
			if (&transformation != camera)
			{
				transformation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f * thread_time);
			}
		}, true);
		engine->GetRendererMutex().lock();
		std::cout << "L:" << thread_time << std::endl;
		engine->UpdateScene();
		engine->GetRendererMutex().unlock();
	}
}
int main(int argc, char **argv)
{
	engine = Engine::Singlton();
	engine->Start(LogicThread);
	// Rendering
	while (engine->Running())
	{
		engine->Update();
		engine->GetRendererMutex().lock();
		std::cout << "R:" << engine->GetFrameTime() << std::endl;
		engine->RenderFrame();
		engine->GetRendererMutex().unlock();
	}
	delete engine;
    return 0;
}
