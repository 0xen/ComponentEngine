#include <ComponentEngine\Components\ParticalSystem.hpp>
#include <ComponentEngine\Engine.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>

ComponentEngine::ParticalSystem::ParticalSystem(enteez::Entity * entity)
{
	m_entity = entity;
}

ComponentEngine::ParticalSystem::~ParticalSystem()
{
}

void ComponentEngine::ParticalSystem::Build()
{

	Renderer::IRenderer* renderer = Engine::Singlton()->GetRenderer();


	m_vertex_data.resize(1);
	m_vertex_buffer = renderer->CreateVertexBuffer(m_vertex_data.data(), sizeof(glm::vec4), m_vertex_data.size());
	m_vertex_buffer->SetData(BufferSlot::Primary);


	for (int i = 0; i < m_vertex_data.size(); i++)
	{

	}



	m_pipeline = renderer->CreateComputePipeline("../../ComponentEngine-demo/Shaders/Compute/Particle/comp.spv", 1, 1, 1);



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
		m_pipeline->AttachDescriptorSet(0, m_partical_vertex_set);
	}


	assert(m_pipeline->Build() && "Unable to build pipeline");




}

void ComponentEngine::ParticalSystem::Update(float frame_time)
{
}

void ComponentEngine::ParticalSystem::Display()
{
}

void ComponentEngine::ParticalSystem::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<ParticalSystem>* wrapper = entity.AddComponent<ParticalSystem>(&entity);
	wrapper->SetName("ParticalSystem");
}
