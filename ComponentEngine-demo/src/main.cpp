#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <ComponentEngine\UI\MenuElement.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <ItemHover.hpp>
#include <KeyboardMovment.hpp>
#include <MouseMovment.hpp>
#include <Flamable.hpp>
#include <BlockMoveController.hpp>
#include <WaterSourceController.hpp>
#include <TeapotController.hpp>
#include <BlockSpawner.hpp>
#include <TimedDestruction.hpp>
#include <iostream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* engine;
IGraphicsPipeline* textured_lighting_pipeline = nullptr;
IGraphicsPipeline* textured_pipeline = nullptr;

void RegisterCustomComponents()
{
	engine->RegisterComponentBase("ItemHover", ItemHover::EntityHookDefault);
	engine->RegisterComponentBase("Keyboard Movment", KeyboardMovment::EntityHookDefault);
	engine->RegisterComponentBase("Mouse Movment", MouseMovment::EntityHookDefault);
	engine->RegisterComponentBase("Flamable", Flamable::EntityHookDefault);
	engine->RegisterComponentBase("Block Move Controller", BlockMoveController::EntityHookDefault);
	engine->RegisterComponentBase("Water Source Controller", WaterSourceController::EntityHookDefault);
	engine->RegisterComponentBase("Teapot Controller", TeapotController::EntityHookDefault);
	engine->RegisterComponentBase("Block Spawner", BlockSpawner::EntityHookDefault);
	engine->RegisterComponentBase("Timed Destruction", TimedDestruction::EntityHookDefault);


	engine->RegisterBase<ItemHover, Logic, UI>();
	engine->RegisterBase<KeyboardMovment, Logic, UI>();
	engine->RegisterBase<MouseMovment, Logic, UI>();
	engine->RegisterBase<Flamable, UI, MsgRecive<OnCollisionEnter>>();
	engine->RegisterBase<BlockMoveController, Logic, UI>();
	engine->RegisterBase<WaterSourceController, UI, MsgRecive<OnCollisionEnter>>();
	engine->RegisterBase<TeapotController, UI, MsgRecive<OnCollisionEnter>, MsgRecive<OnCollisionExit>>();
	engine->RegisterBase<BlockSpawner, Logic>();
	engine->RegisterBase<TimedDestruction, Logic>();
}


void LoadTexturedShaderModel(IGraphicsPipeline* pipeline, IModelPool* modelPool,const char* workingDir, tinyobj::material_t material)
{
	std::string textureName = material.diffuse_texname;

	std::stringstream ss;
	if (textureName.empty())
	{
		ss << Engine::Singlton()->GetCurrentSceneDirectory();
		ss << "/Resources/Models/default.png";
		modelPool->AttachDescriptorSet(1, Engine::Singlton()->GetTexture(ss.str()).texture_descriptor_set);
	}
	else
	{
		ss << workingDir;
		ss << material.diffuse_texname;
		modelPool->AttachDescriptorSet(1, Engine::Singlton()->GetTexture(ss.str()).texture_descriptor_set);
	}

}

void SetupShaders()
{
	{// Textured Pipeline
		textured_pipeline = engine->GetRenderer()->CreateGraphicsPipeline({
		{ ShaderStage::VERTEX_SHADER, "../Shaders/Textured/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../Shaders/Textured/frag.spv" }
		});

		// Tell the pipeline what data is should expect in the forum of Vertex input
		textured_pipeline->AttachVertexBinding(engine->GetDefaultVertexModelBinding());

		textured_pipeline->AttachVertexBinding(engine->GetDefaultVertexModelPositionBinding());

		// Tell the pipeline what the input data will be payed out like
		textured_pipeline->AttachDescriptorPool(engine->GetCameraPool());
		// Attach the camera descriptor set to the pipeline
		textured_pipeline->AttachDescriptorSet(0, engine->GetCameraDescriptorSet());

		textured_pipeline->AttachDescriptorPool(engine->GetTextureMapsPool());

		textured_pipeline->UseCulling(false);

		bool sucsess = textured_pipeline->Build();

		engine->AddPipeline("Textured_", { textured_pipeline ,LoadTexturedShaderModel });
	}

	{// Textured Lighting Pipeline
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
	
}

int main(int argc, char **argv)
{
	engine = Engine::Singlton();
	
	int flags = 0;// EngineFlags::ReleaseBuild;
	engine->SetFlag(flags);

	engine->Start();
	SetupShaders();
	RegisterCustomComponents();

	engine->GetUIManager()->AddMenuElement(new MenuElement("Test", [&] {
		std::cout << "test" << std::endl;
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

	//engine->Join();
	engine->Stop();
	delete engine;

    return 0;
}