#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\ParticalSystem.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include <lodepng.h>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* engine;

IGraphicsPipeline* textured_pipeline = nullptr;

class MeshVertex
{
public:
	MeshVertex(glm::vec3 position, glm::vec2 uv, glm::vec3 normal, glm::vec3 color) : position(position), uv(uv), normal(normal), color(color) {}
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 color;
};


void LogicThread()
{
	engine->GetRendererMutex().lock();
	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();

	engine->GetCameraTransformation()->Translate(glm::vec3(0.0f, 0.0f, 10.0f));

	// Create texture pipeline

	IGraphicsPipeline* textured_pipeline = renderer->CreateGraphicsPipeline({
		{ ShaderStage::VERTEX_SHADER, "../../ComponentEngine-demo/Shaders/Textured/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../../ComponentEngine-demo/Shaders/Textured/frag.spv" }
		});

	textured_pipeline->AttachVertexBinding({
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

	textured_pipeline->AttachVertexBinding({
		VertexInputRate::INPUT_RATE_INSTANCE,
		{
			{ 4, DataFormat::MAT4_FLOAT,0 }
		},
		sizeof(glm::mat4),
		1
		});


	// Tell the pipeline what the input data will be payed out like
	textured_pipeline->AttachDescriptorPool(engine->GetCameraPool());
	// Attach the camera descriptor set to the pipeline
	textured_pipeline->AttachDescriptorSet(0, engine->GetCameraDescriptorSet());


	IDescriptorPool* texture_pool = renderer->CreateDescriptorPool({
		renderer->CreateDescriptor(Renderer::DescriptorType::IMAGE_SAMPLER, Renderer::ShaderStage::FRAGMENT_SHADER, 0),
		});
	textured_pipeline->AttachDescriptorPool(texture_pool);

	textured_pipeline->Build();


	std::vector<unsigned char> image; //the raw pixels
	unsigned width;
	unsigned height;
	unsigned error = lodepng::decode(image, width, height, "../../ComponentEngine-demo/Images/cobble.png");
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	ITextureBuffer* texture = renderer->CreateTextureBuffer(image.data(), Renderer::DataFormat::R8G8B8A8_FLOAT, width, height);
	texture->SetData();

	IDescriptorSet* texture_descriptor_set1 = texture_pool->CreateDescriptorSet();
	texture_descriptor_set1->AttachBuffer(0, texture);
	texture_descriptor_set1->UpdateSet();



	std::vector<MeshVertex> vertexData = {
		MeshVertex(glm::vec3(1.0f,1.0f,0.0f), glm::vec2(0.0f,0.0f) , glm::vec3(1.0f,1.0f,1.0f),glm::vec3(1.0f,1.0f,0.0f)),
		MeshVertex(glm::vec3(1.0f,-1.0f,0.0f), glm::vec2(0.0f,1.0f) , glm::vec3(1.0f,1.0f,1.0f),glm::vec3(0.0f,1.0f,0.0f)),
		MeshVertex(glm::vec3(-1.0f,-1.0f,0.0f), glm::vec2(1.0f,1.0f) , glm::vec3(1.0f,1.0f,1.0f),glm::vec3(.0f,1.0f,1.0f)),
		MeshVertex(glm::vec3(-1.0f,1.0f,0.0f), glm::vec2(1.0f,0.0f) , glm::vec3(1.0f,1.0f,1.0f),glm::vec3(1.0f,0.0f,1.0f))
	};

	IVertexBuffer* vertexBuffer = renderer->CreateVertexBuffer(vertexData.data(), sizeof(MeshVertex), vertexData.size());
	vertexBuffer->SetData();

	std::vector<uint16_t> indexData{
		0,1,2,
		0,2,3
	};
	IIndexBuffer* indexBuffer = renderer->CreateIndexBuffer(indexData.data(), sizeof(uint16_t), indexData.size());
	indexBuffer->SetData();

	IModelPool* model_pool = renderer->CreateModelPool(vertexBuffer, indexBuffer);


	model_pool->AttachDescriptorSet(1, texture_descriptor_set1);


	unsigned int model_array_size = 10000;
	glm::mat4* model_position_array = new glm::mat4[model_array_size];
	float scale = 0.1f;
	for (int i = 0; i < model_array_size; i++)
	{
		model_position_array[i] = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
		
	}
	IUniformBuffer* model_position_buffer = renderer->CreateUniformBuffer(model_position_array, sizeof(glm::mat4), model_array_size);

	// Attach the buffer to buffer index 0
	model_pool->AttachBuffer(0, model_position_buffer);
	float padding_scale = 1.0f;
	int grid_width = 100;
	int grid_height = 100;
	for (int x = 0; x < grid_width; x++)
	{
		for (int y = 0; y < grid_height; y++)
		{
			Entity* entity = em.CreateEntity();
			IModel * model = entity->AddComponent(model_pool->CreateModel());
			Transformation* transform = entity->AddComponent<Transformation>(&model->GetData<glm::mat4>(0));
			transform->Translate(glm::vec3(
				-(grid_width * padding_scale / 2) + (x * padding_scale),
				-(grid_height * padding_scale / 2) + (y * padding_scale),
				0.0f
				));
			transform->Scale(glm::vec3(0.3f, 0.3f, 0.3f));
		}
	}

	textured_pipeline->AttachModelPool(model_pool);

	engine->GetRendererMutex().unlock();
	// Logic Updating
	while (engine->Running())
	{
		float thread_time = engine->GetThreadTime();
		em.ForEach<Transformation>([thread_time](enteez::Entity* entity, Transformation& transformation)
		{
			if (entity != engine->GetCameraEntity())
			{
				transformation.Rotate(glm::vec3(0.0f, 0.0f, 1.0f), 1.0f * thread_time);
			}
		}, true);

		engine->GetRendererMutex().lock();
		model_pool->Update();
		engine->GetRendererMutex().unlock();
	}

	delete texture;

}

int main(int argc, char **argv)
{

	engine = new Engine();

	engine->Start(LogicThread);

	// Rendering
	while (engine->Running())
	{

		engine->Update();
		engine->GetRendererMutex().lock();
		engine->RenderFrame();
		engine->GetRendererMutex().unlock();
	}
	delete engine;

    return 0;
}
