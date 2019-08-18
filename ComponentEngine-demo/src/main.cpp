#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <ComponentEngine\UI\MenuElement.hpp>

#include <renderer\vulkan\VulkanRaytracePipeline.hpp>
#include <renderer\vulkan\VulkanAcceleration.hpp>
#include <renderer\vulkan\VulkanGraphicsPipeline.hpp>
#include <renderer\vulkan\VulkanModelPool.hpp>

#include <EnteeZ\EnteeZ.hpp>
#include <KeyboardMovment.hpp>
#include <iostream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* engine;

VulkanRaytracePipeline* ray_pipeline = nullptr;

void RegisterCustomComponents()
{
	engine->RegisterComponentBase("Keyboard Movment", KeyboardMovment::EntityHookDefault);

	engine->RegisterBase<KeyboardMovment, Logic, UI, IO>();
}

int main(int argc, char **argv)
{
	engine = Engine::Singlton();
	
	int flags = 0;// EngineFlags::ReleaseBuild;
	engine->SetFlag(flags);

	engine->Start();
	RegisterCustomComponents();

	engine->GetUIManager()->AddMenuElement(new MenuElement("Debugging", {
		new MenuElement("Add Mesh", [&] {
			if (engine->GetUIManager()->GetCurrentSceneFocus().entity == nullptr)return;
			engine->GetThreadManager()->AddTask([&](float frameTime) {
				engine->GetLogicMutex().lock();
				engine->GetRendererMutex().lock();

				Mesh::EntityHookDefault(*engine->GetUIManager()->GetCurrentSceneFocus().entity);
				RendererComponent::EntityHookDefault(*engine->GetUIManager()->GetCurrentSceneFocus().entity);

				engine->GetRendererMutex().unlock();
				engine->GetLogicMutex().unlock();
			});

		})
	}));

	if ((flags & EngineFlags::ReleaseBuild) == EngineFlags::ReleaseBuild)
	{
		// Load the scene
		engine->GetThreadManager()->AddTask([&](float frameTime) {
			engine->LoadScene("../Scene.bin");
		});
	}

	while (engine->Running())
	{
		engine->Update();
	}

	engine->Stop();
	delete engine;

    return 0;
}