#pragma once

#include <string>
#include <map>

#include <ComponentEngine\Components\MsgRecive.hpp>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
#include <ComponentEngine\tiny_obj_loader.h>

#include <glm/glm.hpp>

namespace enteez
{
	class Entity;
}
namespace pugi
{
	class xml_node;
}
namespace Renderer
{
	class IVertexBuffer;
	class IIndexBuffer;
	class IUniformBuffer;
	class IGraphicsPipeline;
	class IModelPool;
	class IModel;
}
namespace ComponentEngine
{
	struct ModelBuffers
	{
		~ModelBuffers();
		Renderer::IVertexBuffer* vertexBuffer;
		Renderer::IIndexBuffer* indexBuffer;
		unsigned int used_instances = 0;
	};
	struct ShaderStorage
	{
		~ShaderStorage();
		Renderer::IGraphicsPipeline* pipeline;
	};

	struct ModelPoolData
	{
		Renderer::IModelPool* model_pool;
		glm::mat4* model_position_array;
		Renderer::IUniformBuffer* model_position_buffer;

	};

	struct Mesh : public MsgSend, public MsgRecive<RenderStatus>
	{
		Mesh(enteez::Entity* entity, std::string path);
		static void EntityHook(enteez::Entity& entity, pugi::xml_node& component_data);
		std::string GetPath();
		bool Loaded();
		virtual void ReciveMessage(enteez::Entity* sender, const RenderStatus& message);
	private:
		void LoadModel();
		std::string m_path;
		Renderer::IModel* m_model;
		bool m_loaded;
		static std::map<std::string, ModelBuffers> m_mdel_buffer_instances;
		static std::map<std::string, ModelPoolData> m_model_pools;
		static std::map<std::string, ShaderStorage> m_shaders;
		// How many slots we will reserve 
		static const unsigned int m_buffer_size_step;
	};
}