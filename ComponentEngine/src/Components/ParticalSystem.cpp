#include <ComponentEngine\Components\ParticalSystem.hpp>
#include <ComponentEngine/DefaultMeshVertex.hpp>
#include <ComponentEngine\Engine.hpp>

ComponentEngine::ParticalSystem::ParticalSystem(Engine* engine) : m_engine(engine)
{

}

ComponentEngine::ParticalSystem::~ParticalSystem()
{
	delete m_model_pool;

	delete m_program;
	delete m_pipeline;
	
	delete m_program;
	delete m_pipeline;
	delete m_vertex_buffer;

	delete m_partical_system_values_set;
	delete m_partical_system_values_pool;
	delete m_partical_system_values_buffer;

	delete m_partical_system_configuration_set;
	delete m_partical_system_configuration_pool;
	delete m_partical_system_configuration_buffer;

}

void ComponentEngine::ParticalSystem::Build()
{

	Renderer::IRenderer* renderer = m_engine->GetRenderer();

	m_partical_system_configuration_buffer = renderer->CreateUniformBuffer(&m_config, sizeof(ParticalSystemConfiguration), 1, true);
	m_partical_system_configuration_buffer->SetData();


	m_vertex_data.resize(m_config.data.partical_count * 3);
	m_vertex_buffer = renderer->CreateVertexBuffer(m_vertex_data.data(), sizeof(DefaultMeshVertex), m_vertex_data.size());
	m_vertex_buffer->SetData();


	for (int i = 0; i < m_config.data.partical_count; i++)
	{
		ParticalSystemInstanceValues instance_value;
		float speed = ((rand() % 100)*0.01f);
		instance_value.data.start_position = m_config.data.emiter_location;
		instance_value.data.velocity = glm::vec4(sin(i)*speed, cos(i)*speed, 0.0f, 0.0f);
		instance_value.data.start_life = 2.0f + ((rand() % 10000)*0.0001f);
		instance_value.data.life = 2.0f + ((rand() % 10000)*0.0001f);
		instance_value.data.scale = 0.015f;

		m_partical_system_instance_values.push_back(instance_value);
	}
	m_partical_system_values_buffer = renderer->CreateUniformBuffer(m_partical_system_instance_values.data(), sizeof(ParticalSystemInstanceValues), m_partical_system_instance_values.size(), true);
	m_partical_system_values_buffer->SetData();


	m_pipeline = renderer->CreateComputePipeline("../../ComponentEngine-demo/Shaders/Compute/Particle/comp.spv", m_config.data.partical_count, 1, 1);


	{
		m_partical_system_configuration_pool = renderer->CreateDescriptorPool({
			renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 0),
			});

		// Create descriptor set from the tempalte
		m_partical_system_configuration_set = m_partical_system_configuration_pool->CreateDescriptorSet();
		// Attach the buffer

		m_partical_system_configuration_set->AttachBuffer(0, m_partical_system_configuration_buffer);
		m_partical_system_configuration_set->UpdateSet();

		m_pipeline->AttachDescriptorPool(m_partical_system_configuration_pool);
		m_pipeline->AttachDescriptorSet(0, m_partical_system_configuration_set);
	}

	{
		m_partical_system_values_pool = renderer->CreateDescriptorPool({
			renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 0),
			});

		// Create descriptor set from the tempalte
		m_partical_system_values_set = m_partical_system_values_pool->CreateDescriptorSet();
		// Attach the buffer

		m_partical_system_values_set->AttachBuffer(0, m_partical_system_values_buffer);
		m_partical_system_values_set->UpdateSet();

		m_pipeline->AttachDescriptorPool(m_partical_system_values_pool);
		m_pipeline->AttachDescriptorSet(1, m_partical_system_values_set);
	}

	{
		m_partical_vertex_pool = renderer->CreateDescriptorPool({
			renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 0)
			});

		// Create descriptor set from the tempalte
		m_partical_vertex_set = m_partical_vertex_pool->CreateDescriptorSet();
		// Attach the buffer

		m_partical_vertex_set->AttachBuffer(0, m_vertex_buffer);
		m_partical_vertex_set->UpdateSet();

		m_pipeline->AttachDescriptorPool(m_partical_vertex_pool);
		m_pipeline->AttachDescriptorSet(2, m_partical_vertex_set);
	}


	assert(m_pipeline->Build() && "Unable to build pipeline");

	m_program = renderer->CreateComputeProgram();
	m_program->AttachPipeline(m_pipeline);
	m_program->Build();
	m_program->Run();



	m_model_pool = renderer->CreateModelPool(m_vertex_buffer);

	m_model_position = glm::mat4(1.0f);
	m_model_position = glm::translate(m_model_position, glm::vec3(0.0f, 0.0f, -5.0f));
	m_model_position_buffer = renderer->CreateUniformBuffer(&m_model_position, sizeof(glm::mat4), 1);

	// Attach the buffer to buffer index 0
	m_model_pool->AttachBuffer(0, m_model_position_buffer);


	m_model = m_model_pool->CreateModel();
	m_model->SetData(0, m_model_position);

	m_model_position_buffer->SetData();


	m_engine->GetDefaultGraphicsPipeline()->AttachModelPool(m_model_pool);

}

void ComponentEngine::ParticalSystem::Update()
{
	m_partical_system_configuration_buffer->SetData();
	m_program->Run();
}

ComponentEngine::ParticalSystemConfiguration & ComponentEngine::ParticalSystem::GetConfig()
{
	return m_config;
}
