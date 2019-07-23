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
/*IGraphicsPipeline* textured_lighting_pipeline = nullptr;
IGraphicsPipeline* textured_pipeline = nullptr;
*/
VulkanRaytracePipeline* ray_pipeline = nullptr;

void RegisterCustomComponents()
{
	engine->RegisterComponentBase("Keyboard Movment", KeyboardMovment::EntityHookDefault);

	engine->RegisterBase<KeyboardMovment, Logic, UI, IO>();
}

void SetupShaders()
{
	/*{// Textured Lighting Pipeline
		textured_lighting_pipeline = engine->GetRenderer()->CreateGraphicsPipeline({
		{ ShaderStage::VERTEX_SHADER, "../Shaders/TexturedLighting/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../Shaders/TexturedLighting/frag.spv" }
		});

		// Tell the pipeline what data is should expect in the forum of Vertex input
		textured_lighting_pipeline->AttachVertexBinding(engine->GetDefaultVertexModelBinding());

		textured_lighting_pipeline->AttachVertexBinding(engine->GetDefaultVertexModelPositionBinding());

		// Tell the pipeline what the input data will be payed out like
		textured_lighting_pipeline->AttachDescriptorPool(engine->GetCameraPool());
		// Attach the camera descriptor set to the pipeline
		textured_lighting_pipeline->AttachDescriptorSet(0, engine->GetCameraDescriptorSet());

		textured_lighting_pipeline->AttachDescriptorPool(engine->GetTextureMapsPool());

		textured_lighting_pipeline->UseCulling(true);

		bool sucsess = textured_lighting_pipeline->Build();

		engine->AddPipeline("TexturedLighting_", { textured_lighting_pipeline ,LoadTexturedShaderModel });
	}
	*/





}

int main(int argc, char **argv)
{
	engine = Engine::Singlton();
	
	int flags = 0;// EngineFlags::ReleaseBuild;
	engine->SetFlag(flags);

	engine->Start();
	SetupShaders();
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


	/*
	new MenuElement("Debugging", [&] {
		std::cout << "test" << std::endl;
	})
	*/
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