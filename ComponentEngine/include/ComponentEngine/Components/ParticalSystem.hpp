#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <vector>

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
namespace enteez
{
	class Entity;
}
namespace ComponentEngine
{
	struct ParticalSystemConfiguration
	{
		union
		{
			struct
			{
				glm::vec4 emiter_location;
				glm::vec4 start_color;
				glm::vec4 end_color;
				float frame_time;
				int partical_count;
			}data;
			float memory_padding[sizeof(glm::vec4) * 4];
		};
	};

	struct ParticalSystemInstanceValues
	{
		union
		{
			struct
			{
				glm::vec4 start_position;
				glm::vec4 velocity;
				float start_life;
				float life;
				float scale;
			}data;
			float memory_padding[sizeof(glm::vec4) * 3];
		};
	};



	struct DefaultMeshVertex;
	class Engine;
	const int FRAME_TIME = 0;
	class ParticalSystem
	{
	public:
		ParticalSystem(enteez::Entity* entity, Engine* engine);
		~ParticalSystem();
		void Build();
		void Update();
		void Rebuild();
		ParticalSystemConfiguration& GetConfig();
	private:
		std::vector<DefaultMeshVertex> m_vertex_data;

		std::vector<ParticalSystemInstanceValues> m_partical_system_instance_values;

		Engine* m_engine;
		enteez::Entity* m_entity;

		Renderer::IComputePipeline* m_pipeline;
		Renderer::IComputeProgram* m_program;

		Renderer::IVertexBuffer * m_vertex_buffer = nullptr;
		Renderer::IDescriptorPool* m_partical_vertex_pool = nullptr;
		Renderer::IDescriptorSet* m_partical_vertex_set = nullptr;

		Renderer::IUniformBuffer* m_partical_system_configuration_buffer = nullptr;
		Renderer::IDescriptorPool* m_partical_system_configuration_pool = nullptr;
		Renderer::IDescriptorSet* m_partical_system_configuration_set = nullptr;

		Renderer::IUniformBuffer* m_partical_system_values_buffer = nullptr;
		Renderer::IDescriptorPool* m_partical_system_values_pool = nullptr;
		Renderer::IDescriptorSet* m_partical_system_values_set = nullptr;

		Renderer::IModelPool* m_model_pool = nullptr;
		Renderer::IModel * m_model = nullptr;
		Renderer::IUniformBuffer* m_model_position_buffer = nullptr;

		glm::mat4 m_model_position;


		ParticalSystemConfiguration m_config;
	};
}