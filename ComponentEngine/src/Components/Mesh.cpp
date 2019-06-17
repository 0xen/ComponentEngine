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

std::map<std::string, MeshInstance> Mesh::m_mesh_instance;
std::map<std::string, MaterialStorage> Mesh::m_materials;
const unsigned int Mesh::m_buffer_size_step = 100;

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
	UnloadModel();
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
	if (!m_loaded)return;
	message.GetComponent().MemoryPointTo(&m_mesh_instance[m_file_path.data.longForm].model_position_array, m_mesh_index);
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
		MeshInstance& meshInstance = m_mesh_instance[m_file_path.data.longForm];
		ImGui::Text("Mesh Instance Count: %d", meshInstance.used_instances);
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
			if (m_loaded)UnloadModel();

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
	GetModelPositionTransferLock().lock();
	for (auto& it : m_mesh_instance)
	{
		it.second.model_position_buffer->SetData(BufferSlot::Secondery);
	}
	GetModelPositionTransferLock().unlock();
}

void ComponentEngine::Mesh::TransferToPrimaryBuffers()
{
	GetModelPositionTransferLock().lock();
	for (auto& it : m_mesh_instance)
	{
		it.second.model_position_buffer->Transfer(BufferSlot::Primary, BufferSlot::Secondery);
	}
	GetModelPositionTransferLock().unlock();
}

ordered_lock& ComponentEngine::Mesh::GetModelPositionTransferLock()
{
	return m_transformation_lock;
}

void ComponentEngine::Mesh::LoadModel()
{
	auto it = m_mesh_instance.find(m_file_path.data.longForm);


	if (it == m_mesh_instance.end())
	{
		// Needs changed how we do this
		//std::string material_base_dir = m_dir + "../Resources/";
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, m_file_path.data.longForm.c_str(), m_dir.c_str());
		if (!err.empty())
		{
			std::cerr << err << std::endl;
		}
		if (!ret)
		{
			m_loaded = false;
			return;
		}


		// Create the mesh instance
		MeshInstance& mesh_instance = m_mesh_instance[m_file_path.data.longForm];

		// Create the model position buffers
		mesh_instance.model_position_array = new glm::mat4[m_buffer_size_step];
		//mesh_instance.model_position_array.resize(m_buffer_size_step);
		mesh_instance.model_position_buffer = Engine::Singlton()->GetRenderer()->CreateUniformBuffer(mesh_instance.model_position_array, BufferChain::Double, sizeof(glm::mat4), m_buffer_size_step);
		// Create the sub meshes
		mesh_instance.sub_meshes_count = shapes.size();
		mesh_instance.sub_meshes = new SubMesh[mesh_instance.sub_meshes_count];

		// Load the models into the mesh instance
		for (int i = 0 ; i < mesh_instance.sub_meshes_count; i ++)
		{
			const auto& shape = shapes[i];
			SubMesh& sub_mesh = mesh_instance.sub_meshes[i];

			size_t index_offset = 0;

			std::map<unsigned int, MaterialMeshBases> mesh_bases;

			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
			{
				int fv = shape.mesh.num_face_vertices[f];

				int mat_id = shape.mesh.material_ids[f];

				glm::vec3 diffuse_color = glm::vec3(materials[mat_id].diffuse[0], materials[mat_id].diffuse[1], materials[mat_id].diffuse[2]);
				
				
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
					MeshVertex vertex;
					vertex.position = {
						attrib.vertices[3 * idx.vertex_index + 0],
						attrib.vertices[3 * idx.vertex_index + 1],
						attrib.vertices[3 * idx.vertex_index + 2]
					};

					vertex.color = diffuse_color;

					if (idx.normal_index >= 0)
					{
						vertex.normal = {
							attrib.normals[3 * idx.normal_index + 0],
							attrib.normals[3 * idx.normal_index + 1],
							attrib.normals[3 * idx.normal_index + 2]
						};
					}
					if (idx.texcoord_index >= 0)
					{
						vertex.uv = {
							attrib.texcoords[2 * idx.texcoord_index + 0],
							attrib.texcoords[2 * idx.texcoord_index + 1]
						};
					}

					mesh_bases[mat_id].vertexData.push_back(vertex);
					mesh_bases[mat_id].indexData.push_back(mesh_bases[mat_id].indexData.size());
				}
				index_offset += fv;
			}


			// Creation of the model pools
			sub_mesh.material_meshes_count = mesh_bases.size();
			sub_mesh.material_meshes = new MaterialMesh[sub_mesh.material_meshes_count];

			// Add to the total of pools
			mesh_instance.total_pool_allocation += sub_mesh.material_meshes_count;

			int j = 0;
			for (auto& mesh_it = mesh_bases.begin(); mesh_it != mesh_bases.end(); ++mesh_it)
			{
				MaterialMesh& material_mesh = sub_mesh.material_meshes[j];


				unsigned int material_id = mesh_it->first;

				tinyobj::material_t& material = materials[material_id];

				PipelinePack& pipeline = Engine::Singlton()->GetPipelineContaining(material.name);

				bool nonDefaultPipeline = pipeline.pipeline != Engine::Singlton()->GetDefaultGraphicsPipeline();


				material_mesh.vertexBuffer =
					Engine::Singlton()->GetRenderer()->CreateVertexBuffer(mesh_it->second.vertexData.data(), sizeof(MeshVertex), mesh_it->second.vertexData.size());
				material_mesh.vertexBuffer->SetData(BufferSlot::Primary);

				material_mesh.indexBuffer =
					Engine::Singlton()->GetRenderer()->CreateIndexBuffer(mesh_it->second.indexData.data(), sizeof(uint16_t), mesh_it->second.indexData.size());
				material_mesh.indexBuffer->SetData(BufferSlot::Primary);

				material_mesh.model_pool = Engine::Singlton()->GetRenderer()->CreateModelPool
					(material_mesh.vertexBuffer, material_mesh.indexBuffer);

				material_mesh.model_pool->AttachBuffer(0, mesh_instance.model_position_buffer);


				if (pipeline.modelCreatePointer != nullptr)
				{
					pipeline.modelCreatePointer(pipeline.pipeline, material_mesh.model_pool, m_dir.c_str(), material);
				}
				
				pipeline.pipeline->AttachModelPool(material_mesh.model_pool);

				++j;
			}

		}
		std::stringstream ss;
		ss << "Loaded model: " << m_file_path.data.longForm;
		Engine::Singlton()->Log(ss.str(), Info);

	}

	MeshInstance& mesh_instance = m_mesh_instance[m_file_path.data.longForm];
	// Store the amount of sub-meshes
	m_sub_mesh_count = mesh_instance.total_pool_allocation;
	// Create a imodel pointer array to store all sub-mesh materials
	m_sub_meshes = new Renderer::IModel*[m_sub_mesh_count];

	int i = 0;
	for (int j = 0; j < mesh_instance.sub_meshes_count; j++)
	{
		SubMesh& sub_mesh = mesh_instance.sub_meshes[j];
		for (int k = 0; k < sub_mesh.material_meshes_count; k++)
		{
			MaterialMesh& material_meshe = sub_mesh.material_meshes[k];

			unsigned int currentLargestEntityIndex = material_meshe.model_pool->GetLargestIndex();

			if (currentLargestEntityIndex >= mesh_instance.model_position_buffer->GetElementCount(BufferSlot::Primary))
			{ 
				unsigned int newBufferSize = mesh_instance.model_position_buffer->GetElementCount(BufferSlot::Primary) * 4;
				glm::mat4* newMat = new glm::mat4[newBufferSize];
				memcpy(newMat, mesh_instance.model_position_array, sizeof(glm::mat4) * mesh_instance.model_position_buffer->GetElementCount(BufferSlot::Primary));

				delete mesh_instance.model_position_array;

				mesh_instance.model_position_array = newMat;
				//mesh_instance.model_position_array.resize(m_buffer_size_step);
				mesh_instance.model_position_buffer->Resize(BufferSlot::Primary, newMat, newBufferSize);
				mesh_instance.model_position_buffer->Resize(BufferSlot::Secondery, newMat, newBufferSize);



				
			}

			IModel* model = material_meshe.model_pool->CreateModel();

			m_mesh_index = model->GetModelPoolIndex();

			m_vertex_count += material_meshe.model_pool->GetVertexBuffer()->GetElementCount(BufferSlot::Primary);

			model->ShouldRender(false);
			model->GetData<glm::mat4>(0) = glm::mat4(1.0f);
			m_sub_meshes[i++] = model;
		}
	}
	mesh_instance.model_position_array[m_mesh_index] = glm::mat4(1.0f);
	Send(m_entity, m_entity, TransformationPtrRedirect(&mesh_instance.model_position_array, m_mesh_index));

	// Transfer over the buffer index data to the GPU so that a invalid matrix is not there
	GetModelPositionTransferLock().lock();
	mesh_instance.model_position_buffer->SetData(BufferSlot::Primary, m_mesh_index, 1);
	GetModelPositionTransferLock().unlock();


	m_mesh_instance[m_file_path.data.longForm].used_instances++;


	m_loaded = true;

	Send(m_entity, m_entity, OnComponentEnter<Mesh>(this));
}

void ComponentEngine::Mesh::UnloadModel()
{
	if (!m_loaded)return;
	for (int i = 0; i < m_sub_mesh_count; i++)
	{
		m_sub_meshes[i]->Remove();
	}
	delete[] m_sub_meshes;
	m_sub_mesh_count = 0;
	m_vertex_count = 0;
	m_mesh_instance[m_file_path.data.longForm].used_instances--;
	m_loaded = false;
}

void ComponentEngine::Mesh::CleanUp()
{
	m_materials.clear();
	m_mesh_instance.clear();
}

ComponentEngine::MaterialStorage::~MaterialStorage()
{
	delete m_texture_maps_pool;
	delete m_texture_descriptor_set;
}

ComponentEngine::MeshInstance::~MeshInstance()
{
	delete model_position_buffer;
	for (int i = 0; i < sub_meshes_count; i++)
	{
		SubMesh& sub_mesh = sub_meshes[i];
		for (int j = 0; j < sub_mesh.material_meshes_count; j++)
		{
			MaterialMesh& material_mesh = sub_mesh.material_meshes[j];
			delete material_mesh.vertexBuffer;
			delete material_mesh.indexBuffer;
			delete material_mesh.model_pool;
		}
	}
}
