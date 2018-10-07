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

struct ParticalSystemDefintion
{
	glm::vec4 emiter_location;
};

struct ParticalSystemValues
{
	// values.x = Life Time
	// values.y = Frame Time
	// values.z = Scale
	glm::vec4 values;
};

struct ParticalData
{
	glm::vec4 position;
	glm::vec4 velocity;
	// data.x = Life
	glm::vec4 data;
};

int main(int argc, char **argv)
{
	engine = new Engine();
	engine->RegisterBase<Model, ProcessTask>();

	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();


	unsigned int partical_count = 10000;

	std::vector<DefaultMeshVertex> vertex_data;
	vertex_data.resize(partical_count * 3);

	// Create buffers for both the index and vertex buggers
	IVertexBuffer* vertex_buffer = renderer->CreateVertexBuffer(vertex_data.data(), sizeof(DefaultMeshVertex), vertex_data.size());

	// Set the vertex data for the model
	vertex_buffer->SetData();





	ParticalSystemDefintion partical_system;
	partical_system.emiter_location = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

	// Create buffers for both the index and vertex buggers
	IUniformBuffer* partical_system_buffer = renderer->CreateUniformBuffer(&partical_system, sizeof(ParticalSystemDefintion), 1, true);

	// Set the vertex data for the model
	partical_system_buffer->SetData();




	ParticalSystemValues partical_system_values;
	partical_system_values.values.x = 3.0f;
	partical_system_values.values.y = 0.001f;
	partical_system_values.values.z = 0.02f;

	// Create buffers for both the index and vertex buggers
	IUniformBuffer* partical_system_values_buffer = renderer->CreateUniformBuffer(&partical_system_values, sizeof(ParticalSystemValues), 1, true);

	// Set the vertex data for the model
	partical_system_values_buffer->SetData();




	std::vector<ParticalData> partical_data;

	for (int i = 0; i < partical_count; i++)
	{
		ParticalData data;
		data.position = partical_system.emiter_location;
		data.data.x = 1.0f * (i + 1);
		//data.velocity = glm::vec4(-2.0f + ((4.0f / partical_count) * i), i % 2 == 0 ? 1.0f : -1.0f, 0.0f, 0.0f);

		data.velocity = glm::vec4(sin((float)i), cos((float)i), 0.0f, 0.0f);

		partical_data.push_back(data);
	}

	// Create buffers for both the index and vertex buggers
	IUniformBuffer* partical_buffer = renderer->CreateUniformBuffer(partical_data.data(), sizeof(ParticalData), partical_data.size(), true);

	// Set the vertex data for the model
	partical_buffer->SetData();

	float a = 2.1f;



	// Create camera pool
	// This is a layout for the camera input data
	IDescriptorPool* example_pool = renderer->CreateDescriptorPool({
		renderer->CreateDescriptor(Renderer::DescriptorType::UNIFORM, Renderer::ShaderStage::COMPUTE_SHADER, 0),
		renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 1),
		renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 2),
		renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 3)
		});

	// Create camera descriptor set from the tempalte
	IDescriptorSet* example_descriptor_set = example_pool->CreateDescriptorSet();
	// Attach the buffer
	
	example_descriptor_set->AttachBuffer(0, partical_system_buffer);
	example_descriptor_set->AttachBuffer(1, partical_system_values_buffer);
	example_descriptor_set->AttachBuffer(2, partical_buffer);
	example_descriptor_set->AttachBuffer(3, vertex_buffer);
	example_descriptor_set->UpdateSet();


	IComputePipeline* pipeline = renderer->CreateComputePipeline("../../ComponentEngine-demo/Shaders/Compute/Particle/comp.spv", partical_count, 1, 1);


	// Tell the pipeline what the input data will be payed out like
	pipeline->AttachDescriptorPool(example_pool);
	// Attach the camera descriptor set to the pipeline
	pipeline->AttachDescriptorSet(0, example_descriptor_set);



	assert(pipeline->Build() && "Unable to build pipeline");

	IComputeProgram* program = renderer->CreateComputeProgram();
	program->AttachPipeline(pipeline);
	program->Build();
	program->Run();



	// Define and creae a model pool
	IModelPool* model_pool = renderer->CreateModelPool(vertex_buffer);

	// Create a position buffer for the model pool
	unsigned int model_array_size = 5;
	glm::mat4* model_position_array = new glm::mat4[model_array_size];
	IUniformBuffer* model_position_buffer = renderer->CreateUniformBuffer(model_position_array, sizeof(glm::mat4), model_array_size);

	// Attach the buffer to buffer index 0
	model_pool->AttachBuffer(0, model_position_buffer);


	{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(0.0f, 0.0f, -5.0f));
		entity->AddComponent<Model>(entity, model_pool);
	}


	engine->GetDefaultGraphicsPipeline()->AttachModelPool(model_pool);


	for (auto e : em.GetEntitys())
	{
		e->ForEach<ProcessTask>([](enteez::Entity* entity, ProcessTask& process_task)
		{
			process_task.Update();
		});
	}

	// Update all model positions
	model_position_buffer->SetData();

	while (engine->Running())
	{

		partical_system_values.values.y = engine->GetFrameTime();
		partical_system_values_buffer->SetData();

		program->Run();
		engine->Update();
		engine->Render();

	}



	delete model_position_buffer;
	delete engine;
    return 0;
}
