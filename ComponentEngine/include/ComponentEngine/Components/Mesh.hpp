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
	struct MaterialDefintion
	{
		std::string diffuse_texture;
		std::string metalic_texture;
		std::string roughness_texture;
		std::string normal_texture;
		std::string cavity_texture;
		std::string ao_texture;
	};
	inline bool operator< (const MaterialDefintion &m1, const MaterialDefintion &m2)
	{
		if (m1.diffuse_texture == m2.diffuse_texture)
		{
			if (m1.metalic_texture == m2.metalic_texture)
			{
				if (m1.roughness_texture == m2.roughness_texture)
				{
					if (m1.normal_texture == m2.normal_texture)
					{
						if (m1.cavity_texture == m2.cavity_texture)
						{
							return m1.ao_texture < m2.ao_texture;
						}
						else
						{
							return m1.cavity_texture < m2.cavity_texture;
						}
					}
					else
					{
						return m1.normal_texture < m2.normal_texture;
					}
				}
				else
				{
					return m1.roughness_texture < m2.roughness_texture;
				}
			}
			else
			{
				return m1.metalic_texture < m2.metalic_texture;
			}
		}
		else
		{
			return m1.diffuse_texture < m2.diffuse_texture;
		}
	}


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
		void ChangePath(DropBoxInstance<FileForms>& p, std::string path);
		void InstanciateModel(std::vector<MaterialDefintion> definitions);

		void UpdateMaterials();
		
		void SetMaterial(int index, MaterialDefintion definition);


		DropBoxInstance<FileForms> m_file_path;





		struct MaterialFileForms
		{
			DropBoxInstance<FileForms> diffuse_texture;
			DropBoxInstance<FileForms> metalic_texture;
			DropBoxInstance<FileForms> roughness_texture;
			DropBoxInstance<FileForms> normal_texture;
			DropBoxInstance<FileForms> cavity_texture;
			DropBoxInstance<FileForms> ao_texture;
		};

		std::vector<MaterialFileForms> m_materials;

		unsigned int m_hit_group;

		std::string m_dir;
		VulkanModelPool* m_model_pool;
		VulkanModel* m_model;
		unsigned int m_vertex_count;
		// Index for the current mesh in the position array
		unsigned int m_mesh_index;
		bool m_loaded;
		std::array<int, 8> m_materials_offsets;
		// How many slots we will reserve 
		static const unsigned int m_buffer_size_step;

		enteez::Entity * m_entity;
		std::vector<MaterialDefintion> loading_definitions;
	};
	struct Pending
	{
		Mesh* mesh;
		std::vector<MaterialDefintion> definitions;
	};
	struct MeshInstance
	{
		std::vector<Pending> pending_models;
		VulkanModelPool* mesh_instance;
		std::array<int, 8> defaultMaterialMap;
		int materialCount;
		bool loading;
	};
}
