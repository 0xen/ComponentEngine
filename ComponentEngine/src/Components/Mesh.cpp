#include <ComponentEngine\Components\Mesh.hpp>
#include <Renderer/IVertexBuffer.hpp>
#include <Renderer/IVertexBuffer.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <ComponentEngine\DefaultMeshVertex.hpp>
#include <EnteeZ\EnteeZ.hpp>

using namespace ComponentEngine;

std::map<std::string, ModelBuffers> Mesh::m_mdel_buffer_instances;

ComponentEngine::Mesh::Mesh(std::string path) : m_path(path)
{
	m_loaded = false;
	LoadModel();
}

void ComponentEngine::Mesh::EntityHook(enteez::Entity & entity, pugi::xml_node & component_data)
{

	pugi::xml_node mesh_node = component_data.child("Path");
	if (mesh_node)
	{
		std::string path = mesh_node.attribute("value").as_string();
		Mesh* mesh = entity.AddComponent<Mesh>(path);
		if (!mesh->Loaded())
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

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, const RenderStatus& message)
{
	std::cout << "Revived Message! " << message.should_renderer << std::endl;
}

void ComponentEngine::Mesh::LoadModel()
{
	auto it = m_mdel_buffer_instances.find(m_path);

	
	m_mdel_buffer_instances[m_path].used_instances++;

	if (it != m_mdel_buffer_instances.end())
	{
		m_loaded = true;
		return;
	}
	// Needs changed how we do this
	std::string material_base_dir = "../../ComponentEngine-demo/Resources/Resources/";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, m_path.c_str(), material_base_dir.c_str());
	if (!err.empty())
	{
		std::cerr << err << std::endl;
	}
	if(!ret)
	{
		m_loaded = false;
		return;
	}

	std::vector<MeshVertex> vertexData;
	std::vector<uint16_t> indexData;

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			MeshVertex vertex;
			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			if (index.normal_index >= 0)
			{
				vertex.normal = {
					attrib.vertices[3 * index.normal_index + 0],
					attrib.vertices[3 * index.normal_index + 1],
					attrib.vertices[3 * index.normal_index + 2]
				};
			}
			if (index.texcoord_index >= 0)
			{
				vertex.uv = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}
			vertexData.push_back(vertex);
			indexData.push_back(indexData.size());
			//shape.mesh.material_ids[1];
			//materials[1].
		}
	}
	

	m_loaded = true;
}

ComponentEngine::ModelBuffers::~ModelBuffers()
{
	delete vertexBuffer;
	delete indexBuffer;
}
