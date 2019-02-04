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

// Camera
Transformation* camera;


void RegisterCustomComponents()
{
	engine->RegisterComponentBase("ItemHover", ItemHover::EntityHookDefault, ItemHover::EntityHookXML);
	engine->RegisterBase<ItemHover, Logic, UI>();
}



void test(float a)
{

}

int main(int argc, char **argv)
{


	engine = Engine::Singlton();
	engine->Start();

	RegisterCustomComponents();

	// Load the scene
	engine->GetThreadManager()->AddTask([&](float frameTime) {
		engine->GetRendererMutex().lock();
		engine->LoadScene("../../ComponentEngine-demo/GameInstance.xml");
		camera = engine->GetCameraTransformation();
		camera->Translate(glm::vec3(0.0f, 2.0f, 10.0f));
		engine->GetRendererMutex().unlock();
		engine->UpdateScene();
	});


	engine->GetThreadManager()->AddTask([&](float frameTime) {
		if (engine->KeyDown(SDL_SCANCODE_W))
			engine->GetCameraTransformation()->MoveLocalZ(-10.0f * frameTime);
		if (engine->KeyDown(SDL_SCANCODE_S))
			engine->GetCameraTransformation()->MoveLocalZ(10.0f * frameTime);


		if (engine->KeyDown(SDL_SCANCODE_A))
			engine->GetCameraTransformation()->RotateWorldY(glm::radians(90.0f*frameTime));
		if (engine->KeyDown(SDL_SCANCODE_D))
			engine->GetCameraTransformation()->RotateWorldY(glm::radians(-90.0f*frameTime));
	}, 60, "Keyboard Control");

	
	while (engine->Running(60))
	{
		engine->Update();
	}

	engine->Join();
	engine->Stop();
	delete engine;
    return 0;
}
