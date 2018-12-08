#include <ComponentEngine\Components\Mesh.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Common.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <ComponentEngine\DefaultMeshVertex.hpp>
#include <EnteeZ\EnteeZ.hpp>

#include <imgui.h>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <ComponentEngine\tiny_obj_loader.h>


using namespace ComponentEngine;

std::map<std::string, MeshInstance> Mesh::m_mesh_instance;
std::map<std::string, MaterialStorage> Mesh::m_materials;
const unsigned int Mesh::m_buffer_size_step = 100;

ComponentEngine::Mesh::Mesh(enteez::Entity* entity, std::string path) : /*MsgSend(entity),*/ m_entity(entity), m_path(path), m_dir(Common::GetDir(m_path))
{
	m_loaded = false;
	m_vertex_count = 0;
	LoadModel();
}

ComponentEngine::Mesh::~Mesh()
{
	// Stop rendering
	for (int i = 0; i < m_sub_mesh_count; i++)
	{
		m_sub_meshes[i]->ShouldRender(false);
	}
	delete[] m_sub_meshes;
}

void ComponentEngine::Mesh::EntityHook(enteez::Entity & entity, pugi::xml_node & component_data)
{

	pugi::xml_node mesh_node = component_data.child("Path");
	if (mesh_node)
	{
		std::string path = mesh_node.attribute("value").as_string();


		enteez::ComponentWrapper<Mesh>* mesh = entity.AddComponent<Mesh>(&entity, path);
		mesh->SetName("Mesh");
		if (!mesh->Get().Loaded())
		{
			std::cout << "Mesh: Unable to find mesh (" << path.c_str() << ")" << std::endl;
			entity.RemoveComponent<Mesh>();
		}
	}
}

std::string ComponentEngine::Mesh::GetPath()
{
	return m_path;
}

bool ComponentEngine::Mesh::Loaded()
{
	return m_loaded;
}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, RenderStatus& message)
{
	for (int i = 0; i < m_sub_mesh_count; i++)
	{
		m_sub_meshes[i]->ShouldRender(message.should_renderer);
	}
}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, OnComponentEnter<Transformation>& message)
{
	message.GetComponent().MemoryPointTo(&m_mesh_instance[m_path].model_position_array[m_mesh_index]);
}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, OnComponentExit<Transformation>& message)
{
	//auto it = m_mesh_instance[m_path].model_position_array.begin() + m_mesh_index;
	//m_mesh_instance[m_path].model_position_array.erase(it);
}

void ComponentEngine::Mesh::Update()
{

}

void ComponentEngine::Mesh::Display()
{
	ImGui::Text("Sub-Mesh Count: %d", m_sub_mesh_count);
	ImGui::Text("Vertex Count: %d", m_vertex_count);
}

void ComponentEngine::Mesh::UpdateBuffers()
{
	for (auto& it : m_mesh_instance)
	{
		it.second.model_position_buffer->SetData(BufferSlot::Primary);
	}
}

void ComponentEngine::Mesh::LoadModel()
{
	auto it = m_mesh_instance.find(m_path);


	if (it == m_mesh_instance.end())
	{
		// Needs changed how we do this
		std::string material_base_dir = m_dir + "../Resources/";
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, m_path.c_str(), m_dir.c_str());
		if (!err.empty())
		{
			std::cerr << err << std::endl;
		}
		if (!ret)
		{
			m_loaded = false;
			return;
		}

		// TEMP
		IGraphicsPipeline* last_created_pipeline = Engine::Singlton()->GetDefaultGraphicsPipeline();

		std::vector<IDescriptorSet*> materials_descriptor_set;
		for (auto& m : materials)
		{
			if (m_materials.find(m.name) == m_materials.end())
			{
				// For now use the same shader for everything, just create duplicate pipelines to emulate the functionality


				m_materials[m.name].m_texture_maps_pool = Engine::Singlton()->GetRenderer()->CreateDescriptorPool({
					Engine::Singlton()->GetRenderer()->CreateDescriptor(Renderer::DescriptorType::IMAGE_SAMPLER, Renderer::ShaderStage::FRAGMENT_SHADER, 0),
					});


				IDescriptorSet* texture_maps_descriptor_set = /*Engine::Singlton()->GetTextureMapsPool()*/m_materials[m.name].m_texture_maps_pool->CreateDescriptorSet();
				if (m.diffuse_texname.empty())
				{
					texture_maps_descriptor_set->AttachBuffer(0, Engine::Singlton()->GetTexture(material_base_dir + "default.png"));
				}
				else
				{
					texture_maps_descriptor_set->AttachBuffer(0, Engine::Singlton()->GetTexture(material_base_dir + m.diffuse_texname));
				}
				texture_maps_descriptor_set->UpdateSet();

				m_materials[m.name].m_texture_descriptor_set = texture_maps_descriptor_set;

			}
			
			materials_descriptor_set.push_back(m_materials[m.name].m_texture_descriptor_set);

		}
		
		/*
		
			

		*/

		// Create the mesh instance
		MeshInstance& mesh_instance = m_mesh_instance[m_path];

		// Create the model position buffers
		//mesh_instance.model_position_array = new glm::mat4[m_buffer_size_step];
		mesh_instance.model_position_array.resize(m_buffer_size_step);
		mesh_instance.model_position_buffer = Engine::Singlton()->GetRenderer()->CreateUniformBuffer(mesh_instance.model_position_array.data(), BufferChain::Single, sizeof(glm::mat4), m_buffer_size_step);
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

				material_mesh.vertexBuffer =
					Engine::Singlton()->GetRenderer()->CreateVertexBuffer(mesh_it->second.vertexData.data(), sizeof(MeshVertex), mesh_it->second.vertexData.size());
				material_mesh.vertexBuffer->SetData(BufferSlot::Primary);

				material_mesh.indexBuffer =
					Engine::Singlton()->GetRenderer()->CreateIndexBuffer(mesh_it->second.indexData.data(), sizeof(uint16_t), mesh_it->second.indexData.size());
				material_mesh.indexBuffer->SetData(BufferSlot::Primary);

				material_mesh.model_pool = Engine::Singlton()->GetRenderer()->CreateModelPool
					(material_mesh.vertexBuffer, material_mesh.indexBuffer);

				material_mesh.model_pool->AttachBuffer(0, mesh_instance.model_position_buffer);


				material_mesh.model_pool->AttachDescriptorSet(1, materials_descriptor_set[mesh_it->first]);
				
				last_created_pipeline->AttachModelPool(material_mesh.model_pool);

				++j;
			}

		}


	}

	MeshInstance& mesh_instance = m_mesh_instance[m_path];
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

			IModel* model = material_meshe.model_pool->CreateModel();

			m_vertex_count += material_meshe.model_pool->GetVertexBuffer()->GetElementCount(BufferSlot::Primary);

			model->ShouldRender(false);
			model->GetData<glm::mat4>(0) = glm::mat4(1.0f);
			m_sub_meshes[i++] = model;
		}
	}
	
	m_mesh_index = m_mesh_instance[m_path].used_instances;

	Send(m_entity, TransformationPtrRedirect(&mesh_instance.model_position_array[m_mesh_index]));

	m_mesh_instance[m_path].used_instances++;


	/*
	IModel* model = m_mesh_instance[m_path].model_pool->CreateModel();
	model->ShouldRender(false);
	model->GetData<glm::mat4>(0) = glm::mat4(0.0f);

	Send(TransformationPtrRedirect(&model->GetData<glm::mat4>(0)));

	m_mesh_instance[m_path].model_pool->Update();

	m_model = model;
	*/
	m_loaded = true;
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
