#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\ParticalSystem.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* engine;

int main(int argc, char **argv)
{
	engine = new Engine();

	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();

	engine->GetCameraTransformation()->Translate(glm::vec3(0.0f, 0.0f, 10.0f));

	std::vector<DefaultMeshVertex> vertexData = {
		DefaultMeshVertex(glm::vec4(1.0f,1.0f,0.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f)),
		DefaultMeshVertex(glm::vec4(1.0f,-1.0f,0.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f)),
		DefaultMeshVertex(glm::vec4(-1.0f,-1.0f,0.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f)),
		DefaultMeshVertex(glm::vec4(-1.0f,1.0f,0.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f))
	};
	IVertexBuffer* vertexBuffer = renderer->CreateVertexBuffer(vertexData.data(), sizeof(DefaultMeshVertex), vertexData.size());
	vertexBuffer->SetData();

	std::vector<uint16_t> indexData{
		0,1,2,
		0,2,3
	};
	IIndexBuffer* indexBuffer = renderer->CreateIndexBuffer(indexData.data(), sizeof(uint16_t), indexData.size());
	indexBuffer->SetData();

	IModelPool* model_pool = renderer->CreateModelPool(vertexBuffer, indexBuffer);

	unsigned int model_array_size = 10;
	glm::mat4* model_position_array = new glm::mat4[model_array_size];
	for (int i = 0; i < model_array_size; i++)
	{
		model_position_array[i] = glm::mat4(1.0f);
	}
	IUniformBuffer* model_position_buffer = renderer->CreateUniformBuffer(model_position_array, sizeof(glm::mat4), model_array_size);

	// Attach the buffer to buffer index 0
	model_pool->AttachBuffer(0, model_position_buffer);



	ParticalSystemConfiguration ps_config;
	ps_config.data.emiter_location = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	ps_config.data.start_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	ps_config.data.end_color = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
	ps_config.data.frame_time = 0.0f;
	ps_config.data.partical_count = 50000;

	// Particles
	{
		{
			Entity* entity = em.CreateEntity();
			Transformation* transform = entity->AddComponent<Transformation>();
			transform->Translate(glm::vec3(2.5f, 2.5f, 0.0f));
			ParticalSystem* ps = entity->AddComponent<ParticalSystem>(entity, engine);
			ps->GetConfig() = ps_config;
			ps->Build();
		}
		{
			Entity* entity = em.CreateEntity();
			Transformation* transform = entity->AddComponent<Transformation>();
			transform->Translate(glm::vec3(-2.5f, 2.5f, 0.0f));
			ParticalSystem* ps = entity->AddComponent<ParticalSystem>(entity, engine);
			ps->GetConfig() = ps_config;
			ps->Build();
		}
		{
			Entity* entity = em.CreateEntity();
			Transformation* transform = entity->AddComponent<Transformation>();
			transform->Translate(glm::vec3(-2.5f, -2.5f, 0.0f));
			ParticalSystem* ps = entity->AddComponent<ParticalSystem>(entity, engine);
			ps->GetConfig() = ps_config;
			ps->Build();
		}
		{
			Entity* entity = em.CreateEntity();
			Transformation* transform = entity->AddComponent<Transformation>();
			transform->Translate(glm::vec3(2.5f, -2.5f, 0.0f));
			ParticalSystem* ps = entity->AddComponent<ParticalSystem>(entity, engine);
			ps->GetConfig() = ps_config;
			ps->Build();
		}
	}

	{ // Blocks
		{
			Entity* entity = em.CreateEntity();
			IModel * model = entity->AddComponent(model_pool->CreateModel());
			Transformation* transform = entity->AddComponent<Transformation>(&model->GetData<glm::mat4>(0));
			transform->Translate(glm::vec3(2.5f, 0.0f, 0.0f));
		}
		{
			Entity* entity = em.CreateEntity();
			IModel * model = entity->AddComponent(model_pool->CreateModel());
			Transformation* transform = entity->AddComponent<Transformation>(&model->GetData<glm::mat4>(0));
			transform->Translate(glm::vec3(-2.5f, 0.0f, 0.0f));
		}
		{
			Entity* entity = em.CreateEntity();
			IModel * model = entity->AddComponent(model_pool->CreateModel());
			Transformation* transform = entity->AddComponent<Transformation>(&model->GetData<glm::mat4>(0));
			transform->Translate(glm::vec3(0.0f, 2.5f, 0.0f));
		}
		{
			Entity* entity = em.CreateEntity();
			IModel * model = entity->AddComponent(model_pool->CreateModel());
			Transformation* transform = entity->AddComponent<Transformation>(&model->GetData<glm::mat4>(0));
			transform->Translate(glm::vec3(0.0f, -2.5f, 0.0f));
		}
	}
	

	 // Moving block /w Camera
	Entity* moving_camera_block = em.CreateEntity();
	IModel * model = moving_camera_block->AddComponent(model_pool->CreateModel());
	Transformation* moving_camera_block_transform = moving_camera_block->AddComponent<Transformation>(&model->GetData<glm::mat4>(0));
	moving_camera_block_transform->Translate(glm::vec3(0.0f, 0.0f, 0.0f));

	engine->GetCameraTransformation()->SetParent(moving_camera_block_transform);


	engine->GetDefaultGraphicsPipeline()->AttachModelPool(model_pool);

	float speed = 1.0f;

	while (engine->Running())
	{

		moving_camera_block_transform->Rotate(glm::vec3(0.0f, 0.0f, 1.0f), speed * 0.005f);

		em.ForEach<ParticalSystem>([](enteez::Entity* entity, ParticalSystem& ps)
		{
			ps.Update();
			ps.Rebuild();
		}, true);


		engine->Update();
		engine->Render();

		model_position_buffer->SetData();
	}


	delete engine;
    return 0;
}
