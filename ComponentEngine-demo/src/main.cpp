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

class ProcessTask
{
public:
	virtual void Update() = 0;
};

class Model : public ProcessTask
{
public:
	Model(Entity* entity, IModelPool* model_pool) : m_entity(entity), m_model(model_pool->CreateModel())
	{
	}

	~Model()
	{
		delete m_model;
	}

	virtual void Update()
	{
		if (m_entity->HasComponent<Transformation>())
		{
			// Set the data to buffer index 0
			m_model->SetData(0, m_entity->GetComponent<Transformation>().Get());
		}
	}
private:
	Entity * m_entity;
	IModel * m_model;
};


int main(int argc, char **argv)
{
	engine = new Engine();
	engine->RegisterBase<Model, ProcessTask>();

	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();


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

	unsigned int model_array_size = 2;
	glm::mat4* model_position_array = new glm::mat4[model_array_size];
	IUniformBuffer* model_position_buffer = renderer->CreateUniformBuffer(model_position_array, sizeof(glm::mat4), model_array_size);

	// Attach the buffer to buffer index 0
	model_pool->AttachBuffer(0, model_position_buffer);








	ParticalSystemConfiguration ps_config;
	ps_config.data.emiter_location = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	ps_config.data.start_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	ps_config.data.end_color = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
	ps_config.data.frame_time = 0.0f;
	ps_config.data.partical_count = 50000;

	{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(0.0f, 0.0f, -5.0f));
		entity->AddComponent<Model>(entity, model_pool);
		ParticalSystem* ps = entity->AddComponent<ParticalSystem>(entity, engine);
		ps->GetConfig() = ps_config;
		ps->Build();
	}

	
	Entity* moving_camera_block = em.CreateEntity();
	Transformation* moving_camera_block_transform = moving_camera_block->AddComponent<Transformation>();
	moving_camera_block_transform->Translate(glm::vec3(0.0f, 0.0f, -5.0f));
	moving_camera_block->AddComponent<Model>(moving_camera_block, model_pool);
	

	for (auto e : em.GetEntitys())
	{
		e->ForEach<ProcessTask>([](enteez::Entity* entity, ProcessTask& process_task)
		{
			process_task.Update();
		});
	}

	model_position_buffer->SetData();

	engine->GetDefaultGraphicsPipeline()->AttachModelPool(model_pool);

	float speed = 1.0f;

	while (engine->Running())
	{

		moving_camera_block_transform->Translate(glm::vec3(speed * engine->GetFrameTime(), 0.0f, 0.0f));

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
