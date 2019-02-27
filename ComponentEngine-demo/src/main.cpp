#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <ItemHover.hpp>
#include <KeyboardMovment.hpp>
#include <MouseMovment.hpp>
#include <Flamable.hpp>
#include <BlockMoveController.hpp>
#include <WaterSourceController.hpp>
#include <TeapotController.hpp>
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
	engine->RegisterComponentBase("Flamable", Flamable::EntityHookDefault, Flamable::EntityHookXML);
	engine->RegisterBase<Flamable, UI, MsgRecive<OnCollisionEnter>>();
	engine->RegisterComponentBase("BlockMoveController", BlockMoveController::EntityHookDefault, BlockMoveController::EntityHookXML);
	engine->RegisterBase<BlockMoveController, Logic, UI>();
	engine->RegisterComponentBase("WaterSourceController", WaterSourceController::EntityHookDefault, WaterSourceController::EntityHookXML);
	engine->RegisterBase<WaterSourceController, UI, MsgRecive<OnCollisionEnter>>();
	engine->RegisterComponentBase("TeapotController", TeapotController::EntityHookDefault, TeapotController::EntityHookXML);
	engine->RegisterBase<TeapotController, UI, MsgRecive<OnCollisionEnter>, MsgRecive<OnCollisionExit>>();
}

int main(int argc, char **argv)
{
	engine = Engine::Singlton();
	engine->Start();
	RegisterCustomComponents();

	// Load the scene
	engine->GetThreadManager()->AddTask([&](float frameTime) {
		engine->LoadScene("../../ComponentEngine-demo/GameInstance.xml");
		engine->UpdateScene();
	});

	while (engine->Running())
	{
		engine->Update();
	}

	//engine->Join();
	engine->Stop();
	delete engine;

    return 0;
}
