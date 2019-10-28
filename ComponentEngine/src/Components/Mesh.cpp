#include <ComponentEngine\Components\Mesh.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Common.hpp>

#include <ComponentEngine\ThreadHandler.hpp>
#include <ComponentEngine\DefaultMeshVertex.hpp>
#include <EnteeZ\EnteeZ.hpp>

#include <renderer\vulkan\VulkanTextureBuffer.hpp>
#include <renderer\vulkan\VulkanAcceleration.hpp>
#include <renderer\vulkan\VulkanModelPool.hpp>
#include <renderer\vulkan\VulkanModel.hpp>
#include <renderer\vulkan\VulkanRenderPass.hpp>

#include <imgui.h>

#include <lodepng.h>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <ComponentEngine\tiny_obj_loader.h>


using namespace ComponentEngine;

const unsigned int Mesh::m_buffer_size_step = 100;

ordered_lock ComponentEngine::Mesh::m_transformation_lock;

std::map<std::string, VulkanModelPool*> ComponentEngine::Mesh::m_mesh_instances;

ComponentEngine::Mesh::Mesh(enteez::Entity* entity) : m_entity(entity)
{
	m_loaded = false;
	m_vertex_count = 0;
	m_hit_group = 0;
}

ComponentEngine::Mesh::Mesh(enteez::Entity* entity, std::string path) : /*MsgSend(entity),*/ m_entity(entity)
{
	m_loaded = false;
	m_vertex_count = 0;
	m_hit_group = 0;
	ChangePath(path);
	LoadModel();

}

ComponentEngine::Mesh::~Mesh()
{
	if (m_loaded)
	{
		UnloadModel();
	}
}

void ComponentEngine::Mesh::ChangePath(std::string path)
{
	m_file_path = DropBoxInstance<FileForms>("Mesh");
	m_file_path.data.GenerateFileForm(path);
	m_file_path.SetMessage(m_file_path.data.shortForm);
	m_dir = Common::GetDir(m_file_path.data.longForm);
}

enteez::BaseComponentWrapper* ComponentEngine::Mesh::EntityHookDefault(enteez::Entity& entity)
{

	if (!entity.HasComponent<Mesh>())
	{
		enteez::ComponentWrapper<Mesh>* mesh = entity.AddComponent<Mesh>(&entity);
		mesh->SetName("Mesh");
		Send(mesh->Get().m_entity, mesh->Get().m_entity, OnComponentEnter<Mesh>(&mesh->Get()));
		return mesh;
	}
	return nullptr;///This is a bit shit and needs to be resolved. Need to dynamicly return the base component type
}

std::string ComponentEngine::Mesh::GetPath()
{
	return m_file_path.data.longForm;
}

bool ComponentEngine::Mesh::Loaded()
{
	return m_loaded;
}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, RenderStatus& message)
{
	if (!m_loaded)return;
	m_model->ShouldRender(message.should_renderer);
}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, OnComponentEnter<Transformation>& message)
{

}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, OnComponentExit<Transformation>& message)
{

}

void ComponentEngine::Mesh::Display()
{

	if (m_loaded)
	{
		ImGui::Text("Vertex Count: %d", m_vertex_count);

		std::vector<HitShaderPipeline>& hitgroups = Engine::Singlton()->GetHitShaderPipelines();

		ImGui::Text("Hitgroup");

		Engine* engine = Engine::Singlton();
		VulkanAcceleration* as = engine->GetTopLevelAS();

		m_hit_group = as->GetModelPoolHitGroupOffset(m_model_pool);
		std::string hitgroup_name = hitgroups[m_hit_group].name;

		if (ImGui::BeginCombo("##HitgroupSelection", hitgroup_name.c_str()))
		{
			for (int i = 0 ; i < hitgroups.size(); i ++)
			{
				HitShaderPipeline& hitgroup = hitgroups[i];
				if (!hitgroup.primaryHitgroup) continue;
				bool is_selected = (hitgroup_name == hitgroup.name);
				if (ImGui::Selectable(hitgroup.name.c_str(), is_selected))
				{
					as->SetModelPoolHitGroupOffset(m_model_pool, i);
					engine->GetRenderPass()->Rebuild();
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

	}

	DropBoxInstance<FileForms> tempFilePath = m_file_path;
	if (UIManager::DropBox("Mesh File", "File", tempFilePath))
	{
		if (tempFilePath.data.extension == ".obj" && m_file_path.data.longForm != tempFilePath.data.longForm)
		{
			Engine& engine = *Engine::Singlton();
			engine.GetLogicMutex().lock();
			engine.GetRendererMutex().lock();
			tempFilePath.SetMessage(tempFilePath.data.shortForm);
			//if (m_loaded)UnloadModel();

			ChangePath(tempFilePath.data.longForm);
			//m_file_path = tempFilePath;
			//std::cout << m_file_path.data.longForm << std::endl;
			LoadModel();
			engine.GetRendererMutex().unlock();
			engine.GetLogicMutex().unlock();
		}
	}


}

void ComponentEngine::Mesh::Load(std::ifstream & in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(Mesh, m_hit_group), SizeOfOffsetRange(Mesh, m_hit_group, m_hit_group));
	std::string path = Common::ReadString(in);
	if (path.size()> 0)
	{
		ChangePath(path);
		LoadModel();
	}
}

void ComponentEngine::Mesh::Save(std::ofstream & out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(Mesh, m_hit_group), SizeOfOffsetRange(Mesh, m_hit_group, m_hit_group));

	Common::Write(out, m_file_path.data.longForm);
}

unsigned int ComponentEngine::Mesh::PayloadSize()
{
	return SizeOfOffsetRange(Mesh, m_hit_group, m_hit_group) + Common::StreamStringSize(m_file_path.data.longForm);
}

bool ComponentEngine::Mesh::DynamiclySized()
{
	return true;
}

void ComponentEngine::Mesh::Update(float frame_time)
{
	if (m_loaded)m_model->SetData(0, m_entity->GetComponent<Transformation>().Get());
}

void ComponentEngine::Mesh::EditorUpdate(float frame_time)
{
	if(m_loaded)m_model->SetData(0, m_entity->GetComponent<Transformation>().Get());
}

void ComponentEngine::Mesh::SetBufferData()
{

}

void ComponentEngine::Mesh::TransferToPrimaryBuffers()
{

}

ordered_lock& ComponentEngine::Mesh::GetModelPositionTransferLock()
{
	return m_transformation_lock;
}

void ComponentEngine::Mesh::LoadModel()
{
	Engine* engine = Engine::Singlton();

	if (m_loaded)
	{
		UnloadModel();
	}
	m_loaded = false;

	VulkanAcceleration* as = engine->GetTopLevelAS();

	if (m_mesh_instances.find(m_file_path.data.longForm) == m_mesh_instances.end())
	{
		VulkanVertexBuffer* vertexBuffer = engine->GetGlobalVertexBufer();
		VulkanIndexBuffer* indexBuffer = engine->GetGlobalIndexBuffer();

		std::vector<uint32_t>& all_indexs = engine->GetGlobalIndexArray();
		std::vector<MeshVertex>& all_vertexs = engine->GetGlobalVertexArray();
		std::vector<MatrialObj>& materials = engine->GetGlobalMaterialArray();
		std::vector<VkDescriptorImageInfo>& texture_descriptors = engine->GetTextureDescriptors();
		std::vector<VulkanTextureBuffer*>& textures = engine->GetTextures();
		VulkanBufferPool* position_buffer_pool = engine->GetPositionBufferPool();

		unsigned int& used_vertex = engine->GetUsedVertex();
		unsigned int& used_index = engine->GetUsedIndex();
		unsigned int& used_materials = engine->GetUsedMaterials();

		{
			std::ifstream file(m_file_path.data.longForm, std::ios::ate | std::ios::binary);
			if (!file.is_open())
			{
				return;
			}
			file.close();
		}

		ObjLoader<MeshVertex> loader;
		loader.loadModel(m_file_path.data.longForm);

		uint32_t m_nbVertices = static_cast<uint32_t>(loader.m_vertices.size());
		uint32_t m_nbIndices = static_cast<uint32_t>(loader.m_indices.size());

		unsigned int vertexStart = used_vertex;
		unsigned int indexStart = used_index;

		for (uint32_t& index : loader.m_indices)
		{
			all_indexs[used_index] = index;
			used_index++;
		}

		for (MeshVertex& vertex : loader.m_vertices)
		{
			vertex.matID += used_materials;
			all_vertexs[used_vertex] = vertex;
			used_vertex++;
		}

		unsigned int material_count = 0;
		unsigned int offset = texture_descriptors.size();
		for (auto& material : loader.m_materials)
		{
			if (material.textureID >= 0)
				material.textureID += offset;
			else
				material.textureID = 0; // Set to default white texture

			if (material.metalicTextureID >= 0)
				material.metalicTextureID += offset;
			else
				material.metalicTextureID = 1; // Set to default black texture

			if (material.roughnessTextureID >= 0)
				material.roughnessTextureID += offset;
			else
				material.roughnessTextureID = 0; // Set to default white texture

			if (material.normalTextureID >= 0)
				material.normalTextureID += offset;
			else
				material.normalTextureID = 0; // Set to default white texture

			materials[used_materials] = material;

			used_materials++;
		}

		for (auto& texturePath : loader.m_textures)
		{
			std::stringstream ss;
			ss << m_dir << texturePath;

			VulkanTextureBuffer* texture = engine->LoadTexture(ss.str());

			textures.push_back(texture);

			texture_descriptors.push_back(texture->GetDescriptorImageInfo(BufferSlot::Primary));

		}

		m_mesh_instances[m_file_path.data.longForm] = engine->GetRenderer()->CreateModelPool(vertexBuffer, vertexStart, m_nbVertices, indexBuffer, indexStart, m_nbIndices, ModelPoolUsage::SingleMesh);

		m_mesh_instances[m_file_path.data.longForm]->AttachBufferPool(0, position_buffer_pool);



		as->AttachModelPool(m_mesh_instances[m_file_path.data.longForm], m_hit_group);
	}

	
	
	m_model_pool = m_mesh_instances[m_file_path.data.longForm];

	// Create a instance of the model
	m_model = m_model_pool->CreateModel();
	
	m_vertex_count = m_model_pool->GetVertexSize();

	m_model->SetData(0, m_entity->GetComponent<Transformation>().Get());


	engine->UpdateAccelerationDependancys();

	m_loaded = true;
}

void ComponentEngine::Mesh::UnloadModel()
{
	if (m_loaded)
	{
		m_model->Remove();
		Engine::Singlton()->UpdateAccelerationDependancys();
		m_loaded = false;
	}
}
