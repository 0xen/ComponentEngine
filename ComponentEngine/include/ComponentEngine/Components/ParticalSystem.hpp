#pragma once

#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine/Components/ComponentMessages.hpp>
#include <ComponentEngine\ThreadHandler.hpp>
#include <ComponentEngine\Components\MsgRecive.hpp>

#include <glm\glm.hpp>

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
	class IGraphicsPipeline;
	class IComputePipeline;
	class IComputeProgram;
	class IDescriptorPool;
	class IDescriptorSet;
}


namespace pugi
{
	class xml_node;
}
namespace ComponentEngine
{
	struct ParticleSystemBufferConfig
	{

		union
		{
			float _memory[4 * 6];
			struct
			{
				glm::vec4 startColor;   // Byte 0 - 3
				glm::vec4 endColor;     // Byte 4 - 7
				glm::vec3 emitter;      // Byte 8 - 10
				float totalTime;        // Byte 11
				float updateTime;       // Byte 12
				float maxLife;          // Byte 13
				float emissionRate;     // Byte 14
				float scale;            // Byte 15
			}memory;

		};
	};

	struct ParticleSystemConfig
	{
		ParticleSystemBufferConfig buffer_config;
		glm::vec3 emmitter_offset;
		glm::vec2 xVelocity;
		glm::vec2 yVelocity;
		glm::vec2 zVelocity;
		float directionalVelocity;
		bool m_useColorRange = true;
		bool m_dynamicParticleCount = true;
		bool xVelocityStatic = false;
		bool yVelocityStatic = false;
		bool zVelocityStatic = false;
	};

	struct ParticlePayload
	{

		union
		{
			float _memory[8];
			struct
			{
				glm::vec3 origin;       // Byte 0 - 2
				float life;             // Byte 3
				glm::vec3 velocity;     // Byte 4 - 6
			}memory;

		};

	};

	struct ParticleInstance
	{
		glm::vec4 position;
		glm::vec4 color;
	};

	class ParticleSystem : public Logic, public UI, public MsgRecive<ParticleSystemVisibility>
	{
	public:
		ParticleSystem(enteez::Entity * entity);
		~ParticleSystem();

		virtual void Build(); 

		virtual void Update(float frame_time);
		virtual void Display();
		static void EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);

		virtual void ReciveMessage(enteez::Entity* sender, ParticleSystemVisibility& message);
		void ResetTimer();
		bool IsRunning();
		void SetRunning(bool running);
		bool IsVisible();
		void SetVisible(bool visible);
	private:

		void RebuildConfig();
		void RebuildAll();

		ordered_lock m_particle_lock;

		std::vector<ParticleInstance> m_vertex_data;
		std::vector<ParticlePayload> m_particle_payloads;

		enteez::Entity * m_entity;

		Renderer::IGraphicsPipeline* m_graphics_pipeline = nullptr;
		Renderer::IComputePipeline* m_compute_pipeline = nullptr;
		Renderer::IComputeProgram* m_program = nullptr;
		Renderer::IComputeProgram* m_compute_program = nullptr;

		Renderer::IVertexBuffer * m_vertex_buffer = nullptr;
		Renderer::IDescriptorPool* m_partical_vertex_pool = nullptr;
		Renderer::IDescriptorSet* m_partical_vertex_set = nullptr;

		// Particle Payloads
		Renderer::IUniformBuffer* m_particle_payload_buffer = nullptr;

		// Particle Payloads
		Renderer::IUniformBuffer* m_config_buffer = nullptr;


		Renderer::IModelPool* m_model_pool = nullptr;
		Renderer::IModel * m_model = nullptr;


		ParticleSystemConfig m_config;

		int m_particleCount;

		bool m_running = true;
		bool m_visible = true;

	};
}