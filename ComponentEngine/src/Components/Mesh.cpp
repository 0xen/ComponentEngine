#include <ComponentEngine\Components\Mesh.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Common.hpp>

#include <ComponentEngine\ThreadHandler.hpp>
#include <ComponentEngine\DefaultMeshVertex.hpp>
#include <EnteeZ\EnteeZ.hpp>

#include <imgui.h>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <ComponentEngine\tiny_obj_loader.h>


using namespace ComponentEngine;

const unsigned int Mesh::m_buffer_size_step = 100;

unsigned int Mesh::m_used_vertex = 0;
unsigned int Mesh::m_used_index = 0;

ordered_lock ComponentEngine::Mesh::m_transformation_lock;

ComponentEngine::Mesh::Mesh(enteez::Entity* entity) : m_entity(entity)
{
	m_loaded = false;
	m_vertex_count = 0;


}

ComponentEngine::Mesh::Mesh(enteez::Entity* entity, std::string path) : /*MsgSend(entity),*/ m_entity(entity)
{
	m_loaded = false;
	m_vertex_count = 0;
	ChangePath(path);
	LoadModel();

}

ComponentEngine::Mesh::~Mesh()
{

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
	for (int i = 0; i < m_sub_mesh_count; i++)
	{
		m_sub_meshes[i]->ShouldRender(message.should_renderer);
	}
}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, OnComponentEnter<Transformation>& message)
{

}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, OnComponentExit<Transformation>& message)
{
	//auto it = m_mesh_instance[m_path].model_position_array.begin() + m_mesh_index;
	//m_mesh_instance[m_path].model_position_array.erase(it);
}

void ComponentEngine::Mesh::Display()
{

	if (m_loaded)
	{
		//MeshInstance& meshInstance = m_mesh_instance[m_file_path.data.longForm];
		//ImGui::Text("Mesh Instance Count: %d", meshInstance.used_instances);
		ImGui::Text("Sub-Mesh Count: %d", m_sub_mesh_count);
		ImGui::Text("Vertex Count: %d", m_vertex_count);
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
	std::string path = Common::ReadString(in);
	if (path.size()> 0)
	{
		ChangePath(path);
		LoadModel();
	}
}

void ComponentEngine::Mesh::Save(std::ofstream & out)
{
	Common::Write(out, m_file_path.data.longForm);
}

unsigned int ComponentEngine::Mesh::PayloadSize()
{
	return Common::StreamStringSize(m_file_path.data.longForm);
}

bool ComponentEngine::Mesh::DynamiclySized()
{
	return true;
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

}