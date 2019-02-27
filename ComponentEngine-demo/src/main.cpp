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
IGraphicsPipeline* m_default_pipeline = nullptr;

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

void SetupShaders()
{
	m_default_pipeline = engine->GetRenderer()->CreateGraphicsPipeline({
		{ ShaderStage::VERTEX_SHADER, "../../ComponentEngine-demo/Shaders/Textured/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../../ComponentEngine-demo/Shaders/Textured/frag.spv" }
		});

	// Tell the pipeline what data is should expect in the forum of Vertex input
	m_default_pipeline->AttachVertexBinding({
		VertexInputRate::INPUT_RATE_VERTEX,
		{
			{ 0, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,position) },
		{ 1, DataFormat::R32G32_FLOAT,offsetof(MeshVertex,uv) },
		{ 2, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,normal) },
		{ 3, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,color) },
		},
		sizeof(MeshVertex),
		0
		});

	m_default_pipeline->AttachVertexBinding({
		VertexInputRate::INPUT_RATE_INSTANCE, // Input Rate
		{
			{	// Vertex Bindings
				4, // Location
				DataFormat::MAT4_FLOAT, // Format
				0 // Offset from start of data structure
			}
		},
		sizeof(glm::mat4), // Total size
		1 // Binding
		});

	// Tell the pipeline what the input data will be payed out like
	m_default_pipeline->AttachDescriptorPool(engine->GetCameraPool());
	// Attach the camera descriptor set to the pipeline
	m_default_pipeline->AttachDescriptorSet(0, engine->GetCameraDescriptorSet());

	m_default_pipeline->AttachDescriptorPool(engine->GetTextureMapsPool());

	m_default_pipeline->UseCulling(true);

	bool sucsess = m_default_pipeline->Build();
}

void CleanupShaders()
{

	delete m_default_pipeline;
}

int main(int argc, char **argv)
{
	engine = Engine::Singlton();
	engine->Start();

	RegisterCustomComponents();
	SetupShaders();

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
