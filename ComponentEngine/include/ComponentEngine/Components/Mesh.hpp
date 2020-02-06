#pragma once

#include <string>
#include <vector>
#include <map>

#include <ComponentEngine\Components\MsgRecive.hpp>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\IO.hpp>
#include <ComponentEngine\Components\Logic.hpp>
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
	namespace Vulkan
	{
		class VulkanModel;
		class VulkanModelPool;
	}
}

using namespace Renderer;
using namespace Renderer::Vulkan;

namespace ComponentEngine
{
	class MeshVertex;
	struct FileForms;

	class Transformation;

	class Mesh : public MsgRecive<RenderStatus>, public UI , public IO, public Logic,
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

		virtual void Update(float frame_time);
		virtual void EditorUpdate(float frame_time);
		void LoadModel();
		void UnloadModel();

		int GetUUID();

		friend class Engine;
	private:
		void InstanciateModel();

		// Key: File path, Value: Model instance
		static std::map<std::string, VulkanModelPool*> m_mesh_instances;
		static std::vector<std::string> m_meshes_loading;
		static std::map<std::string, std::vector<Mesh*>> m_pending_models;

		DropBoxInstance<FileForms> m_file_path;

		unsigned int m_hit_group;

		std::string m_dir;
		VulkanModelPool* m_model_pool;
		VulkanModel* m_model;
		unsigned int m_vertex_count;
		// Index for the current mesh in the position array
		unsigned int m_mesh_index;
		bool m_loaded;

		// How many slots we will reserve 
		static const unsigned int m_buffer_size_step;

		enteez::Entity * m_entity;
	};
}
