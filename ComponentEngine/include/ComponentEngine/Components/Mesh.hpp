#pragma once

#include <string>
#include <map>

#include <ComponentEngine\Components\MsgRecive.hpp>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\Logic.hpp>
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
	class IDescriptorPool;
	class IDescriptorSet;
	class IGraphicsPipeline;
	class IModelPool;
	class IModel;
}
namespace ComponentEngine
{
	class MeshVertex;


	struct MaterialStorage
	{
		~MaterialStorage();
		Renderer::IDescriptorPool* m_texture_maps_pool;
		Renderer::IDescriptorSet* m_texture_descriptor_set;
	};

	struct MaterialMeshBases
	{
		std::vector<MeshVertex> vertexData;
		std::vector<uint16_t> indexData;
	};

	// This is a part of the sub-mesh of X Material
	struct MaterialMesh
	{
		Renderer::IVertexBuffer* vertexBuffer;
		Renderer::IIndexBuffer* indexBuffer;
		Renderer::IModelPool* model_pool;
	};

	// a sub-mesh of the main mesh
	struct SubMesh
	{
		MaterialMesh* material_meshes;
		unsigned int material_meshes_count = 0;
	};

	// Instance of the main mesh
	struct MeshInstance
	{
		SubMesh* sub_meshes;
		unsigned int sub_meshes_count = 0;

		unsigned int total_pool_allocation = 0;

		glm::mat4* model_position_array;
		Renderer::IUniformBuffer* model_position_buffer;

		unsigned int used_instances = 0;

	};


	struct Mesh : public MsgSend, public Logic, public MsgRecive<RenderStatus>
	{
		Mesh(enteez::Entity* entity, std::string path);
		static void EntityHook(enteez::Entity& entity, pugi::xml_node& component_data);
		std::string GetPath();
		bool Loaded();
		virtual void ReciveMessage(enteez::Entity* sender, const RenderStatus& message);
		virtual void Update();
		static void UpdateBuffers();
	private:
		void LoadModel();
		std::string m_path;
		std::string m_dir;
		Renderer::IModel** m_sub_meshes;
		unsigned int m_sub_mesh_count;
		bool m_loaded;
		static std::map<std::string, MeshInstance> m_mesh_instance;
		static std::map<std::string, MaterialStorage> m_materials;
		// How many slots we will reserve 
		static const unsigned int m_buffer_size_step;
	};
}
