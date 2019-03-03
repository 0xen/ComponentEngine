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
IGraphicsPipeline* textured_lighting_pipeline = nullptr;
IGraphicsPipeline* textured_pipeline = nullptr;

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
		{ ShaderStage::VERTEX_SHADER, "../../ComponentEngine-demo/Shaders/Textured/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../../ComponentEngine-demo/Shaders/Textured/frag.spv" }
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
		{ ShaderStage::VERTEX_SHADER, "../../ComponentEngine-demo/Shaders/TexturedLighting/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../../ComponentEngine-demo/Shaders/TexturedLighting/frag.spv" }
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

void CleanupShaders()
{
	delete textured_pipeline;
	delete textured_lighting_pipeline;
}

int main(int argc, char **argv)
{

	engine = Engine::Singlton();
	engine->Start();

	SetupShaders();
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
	CleanupShaders();

	//engine->Join();
	engine->Stop();
	delete engine;

    return 0;
}
