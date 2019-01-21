#pragma once

#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\Logic.hpp>

#include <vector>

namespace enteez
{
	class Entity;
}
namespace Renderer
{
	class IVertexBuffer;
	class IUniformBuffer;
	class IModelPool;
	class IModel;
	class IComputePipeline;
	class IComputeProgram;
	class IDescriptorPool;
	class IDescriptorSet;
}

namespace ComponentEngine
{
	
	class ParticalSystem : public Logic, public UI
	{
	public:
		ParticalSystem(enteez::Entity * entity);
		~ParticalSystem();

		virtual void Build();

		virtual void Update(float frame_time);
		virtual void Display();
		static void EntityHookDefault(enteez::Entity& entity);
	private:

		std::vector<glm::vec4> m_vertex_data;

		enteez::Entity * m_entity;

		Renderer::IComputePipeline* m_pipeline;
		Renderer::IComputeProgram* m_program;

		Renderer::IVertexBuffer * m_vertex_buffer = nullptr;
		Renderer::IDescriptorPool* m_partical_vertex_pool = nullptr;
		Renderer::IDescriptorSet* m_partical_vertex_set = nullptr;

	};
}