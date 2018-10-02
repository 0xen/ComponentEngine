#include <ComponentEngine\Engine.hpp>
#include <iostream>
#include <vector>

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


	std::vector<DefaultMeshVertex> vertex_data = {
		DefaultMeshVertex(glm::vec3(1.0f,1.0f,0.0f)),
		DefaultMeshVertex(glm::vec3(1.0f,-1.0f,0.0f)),
		DefaultMeshVertex(glm::vec3(-1.0f,-1.0f,0.0f)),
		DefaultMeshVertex(glm::vec3(-1.0f,1.0f,0.0f))
	};

	std::vector<uint16_t> index_data{
		0,1,2,
		0,2,3
	};

	// Create buffers for both the index and vertex buggers
	IVertexBuffer* vertex_buffer = renderer->CreateVertexBuffer(vertex_data.data(), sizeof(DefaultMeshVertex), vertex_data.size());
	IIndexBuffer* index_buffer = renderer->CreateIndexBuffer(index_data.data(), sizeof(uint16_t), index_data.size());

	// Set the vertex data for the model
	vertex_buffer->SetData();
	index_buffer->SetData();


	// Define and creae a model pool
	IModelPool* model_pool = renderer->CreateModelPool(vertex_buffer, index_buffer);

	// Create a position buffer for the model pool
	unsigned int model_array_size = 4;
	glm::mat4* model_position_array = new glm::mat4[model_array_size];
	IUniformBuffer* model_position_buffer = renderer->CreateUniformBuffer(model_position_array, sizeof(glm::mat4), model_array_size);

	// Attach the buffer to buffer index 0
	model_pool->AttachBuffer(0, model_position_buffer);

	{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(-2.0f, 0.0f, -5.0f));
		entity->AddComponent<Model>(entity, model_pool);
	}

	{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(2.0f, 0.0f, -5.0f));
		entity->AddComponent<Model>(entity, model_pool);
	}

	{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(0.0f, 2.0f, -5.0f));
		entity->AddComponent<Model>(entity, model_pool);
	}

	{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(0.0f, -2.0f, -5.0f));
		entity->AddComponent<Model>(entity, model_pool);
	}


	engine->GetDefaultGraphicsPipeline()->AttachModelPool(model_pool);




	while (engine->Running())
	{
		em.ForEach<Transformation>([](enteez::Entity* entity, Transformation& transformation)
		{
			transformation.Rotate(glm::vec3(0.0f, 0.0f, 1.0f), 0.001f);
		}, true);

		for (auto e : em.GetEntitys())
		{
			e->ForEach<ProcessTask>([](enteez::Entity* entity, ProcessTask& process_task)
			{
				process_task.Update();
			});
		}

		// Update all model positions
		model_position_buffer->SetData();

		engine->Update();
		engine->Render();

	}



	delete model_position_buffer;
	delete engine;
    return 0;
}
