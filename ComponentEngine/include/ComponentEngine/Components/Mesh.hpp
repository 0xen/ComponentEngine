#pragma once

#include <string>
#include <map>

#include <ComponentEngine\Components\MsgRecive.hpp>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\IO.hpp>
#include <ComponentEngine\UI\UIManager.hpp>
#include <ComponentEngine\tiny_obj_loader.h>

#include <glm/glm.hpp>

class ordered_lock;

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
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
	struct FileForms;

	class Transformation;

	class Mesh : public MsgRecive<RenderStatus>, public UI , public IO,
		public MsgRecive<OnComponentEnter<Transformation>>, public MsgRecive<OnComponentExit<Transformation>>
	{
	public:
		Mesh(enteez::Entity* entity);
		Mesh(enteez::Entity* entity, std::string path);
		~Mesh();
		void ChangePath(std::string path);
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
		std::string GetPath();
		bool Loaded();
		virtual void ReciveMessage(enteez::Entity* sender, RenderStatus& message);
		virtual void ReciveMessage(enteez::Entity* sender, OnComponentEnter<Transformation>& message);
		virtual void ReciveMessage(enteez::Entity* sender, OnComponentExit<Transformation>& message);
		virtual void Display();

		virtual void Load(std::ifstream& in);
		virtual void Save(std::ofstream& out);
		virtual unsigned int PayloadSize();
		virtual bool DynamiclySized();

		static void SetBufferData();
		static void TransferToPrimaryBuffers();
		static ordered_lock& GetModelPositionTransferLock();
		void LoadModel();
		friend class Engine;
	private:
		DropBoxInstance<FileForms> m_file_path;
		std::string m_dir;
		Renderer::IModel** m_sub_meshes;
		unsigned int m_sub_mesh_count;
		unsigned int m_vertex_count;
		// Index for the current mesh in the position array
		unsigned int m_mesh_index;
		bool m_loaded;

		// How many slots we will reserve 
		static const unsigned int m_buffer_size_step;

		static ordered_lock m_transformation_lock;

		enteez::Entity * m_entity;


	};
}
