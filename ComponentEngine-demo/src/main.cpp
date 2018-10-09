#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\ParticalSystem.hpp>
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
	glm::vec4 start_color;
	glm::vec4 end_color;
	float data[4];
};

struct ParticalSystemValues
{
	glm::vec4 start_position;
	glm::vec4 velocity;
	// data.x = Start Life
	// data.y = Life
	// data.z = Scale
	glm::vec4 data;
};


int main(int argc, char **argv)
{
	engine = new Engine();
	engine->RegisterBase<Model, ProcessTask>();

	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();


	/*unsigned int partical_count = 50000;

	std::vector<DefaultMeshVertex> vertex_data;
	vertex_data.resize(partical_count * 3);

	// Create buffers for both the index and vertex buggers
	IVertexBuffer* vertex_buffer = renderer->CreateVertexBuffer(vertex_data.data(), sizeof(DefaultMeshVertex), vertex_data.size());

	// Set the vertex data for the model
	vertex_buffer->SetData();





	ParticalSystemDefintion partical_system;
	partical_system.emiter_location = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	partical_system.start_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	partical_system.end_color = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
	partical_system.data[ComponentEngine::FRAME_TIME] = 0.0f;

	// Create buffers for both the index and vertex buggers
	IUniformBuffer* partical_system_buffer = renderer->CreateUniformBuffer(&partical_system, sizeof(ParticalSystemDefintion), 1, true);

	// Set the vertex data for the model
	partical_system_buffer->SetData();


	std::vector<ParticalSystemValues> partical_system_values;

	for (int i = 0; i < partical_count; i++)
	{
		ParticalSystemValues partical_system_value;
		float speed = ((rand() % 100)*0.01f);
		partical_system_value.data.x = 2.0f + ((rand()%10000)*0.0001f);
		partical_system_value.data.y = partical_system_value.data.x;
		partical_system_value.data.z = 0.015f;
		partical_system_value.start_position = partical_system.emiter_location;
		partical_system_value.velocity = glm::vec4(sin(i)*speed, cos(i)*speed, 0.0f, 0.0f);

		partical_system_values.push_back(partical_system_value);
	}

	// Create buffers for both the index and vertex buggers
	IUniformBuffer* partical_system_values_buffer = renderer->CreateUniformBuffer(partical_system_values.data(), sizeof(ParticalSystemValues), partical_system_values.size(), true);

	// Set the vertex data for the model
	partical_system_values_buffer->SetData();





	IComputePipeline* pipeline = renderer->CreateComputePipeline("../../ComponentEngine-demo/Shaders/Compute/Particle/comp.spv", partical_count, 1, 1);


	{
		IDescriptorPool* partical_system_pool = renderer->CreateDescriptorPool({
			renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 0),
			});

		// Create camera descriptor set from the tempalte
		IDescriptorSet* partical_system_set = partical_system_pool->CreateDescriptorSet();
		// Attach the buffer

		partical_system_set->AttachBuffer(0, partical_system_buffer);
		partical_system_set->UpdateSet();

		pipeline->AttachDescriptorPool(partical_system_pool);
		pipeline->AttachDescriptorSet(0, partical_system_set);
	}

	{
		IDescriptorPool* partical_system_values_pool = renderer->CreateDescriptorPool({
			renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 0),
			});

		// Create camera descriptor set from the tempalte
		IDescriptorSet* partical_system_values_set = partical_system_values_pool->CreateDescriptorSet();
		// Attach the buffer

		partical_system_values_set->AttachBuffer(0, partical_system_values_buffer);
		partical_system_values_set->UpdateSet();

		pipeline->AttachDescriptorPool(partical_system_values_pool);
		pipeline->AttachDescriptorSet(1, partical_system_values_set);
	}

	{
		IDescriptorPool* partical_vertex_pool = renderer->CreateDescriptorPool({
			renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 0)
			});

		// Create camera descriptor set from the tempalte
		IDescriptorSet* partical_vertex_set = partical_vertex_pool->CreateDescriptorSet();
		// Attach the buffer

		partical_vertex_set->AttachBuffer(0, vertex_buffer);
		partical_vertex_set->UpdateSet();

		pipeline->AttachDescriptorPool(partical_vertex_pool);
		pipeline->AttachDescriptorSet(2, partical_vertex_set);
	}


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



	engine->GetDefaultGraphicsPipeline()->AttachModelPool(model_pool);


	*/

	//{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(0.0f, 0.0f, -5.0f));
		//entity->AddComponent<Model>(entity, model_pool);



		ParticalSystem* ps = entity->AddComponent<ParticalSystem>(engine);
		ParticalSystemConfiguration& ps_config = ps->GetConfig();
		ps_config.data.emiter_location = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		ps_config.data.start_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		ps_config.data.end_color = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
		ps_config.data.frame_time = 0.0f;
		ps_config.data.partical_count = 100;

		ps->Build();

	//}



	for (auto e : em.GetEntitys())
	{
		e->ForEach<ProcessTask>([](enteez::Entity* entity, ProcessTask& process_task)
		{
			process_task.Update();
		});
	}

	// Update all model positions
	//model_position_buffer->SetData();

	while (engine->Running())
	{

		ps_config.data.frame_time = engine->GetFrameTime();
		ps->Update();

		//partical_system.data[ComponentEngine::FRAME_TIME] = engine->GetFrameTime();
		//partical_system_buffer->SetData();

		//program->Run();
		engine->Update();
		engine->Render();

	}



	//delete model_position_buffer;
	delete engine;
    return 0;
}
