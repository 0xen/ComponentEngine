#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <ItemHover.hpp>
#include <KeyboardMovment.hpp>
#include <MouseMovment.hpp>
#include <iostream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;
Engine* engine;

void RegisterCustomComponents()
{
	engine->RegisterComponentBase("ItemHover", ItemHover::EntityHookDefault, ItemHover::EntityHookXML);
	engine->RegisterBase<ItemHover, Logic, UI>();
	engine->RegisterComponentBase("KeyboardMovment", KeyboardMovment::EntityHookDefault, KeyboardMovment::EntityHookXML);
	engine->RegisterBase<KeyboardMovment, Logic, UI>();
	engine->RegisterComponentBase("MouseMovment", MouseMovment::EntityHookDefault, MouseMovment::EntityHookXML);
	engine->RegisterBase<MouseMovment, Logic, UI>();
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
		engine->GetRendererMutex().unlock();
		engine->UpdateScene();
	});
	while (engine->Running(60))
	{
		engine->Update();
	}
	engine->Join();
	engine->Stop();
	delete engine;
    return 0;
}
