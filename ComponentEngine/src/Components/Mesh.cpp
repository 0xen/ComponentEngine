#include <ComponentEngine\Components\Mesh.hpp>

#include <ComponentEngine\Engine.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <ComponentEngine\DefaultMeshVertex.hpp>
#include <EnteeZ\EnteeZ.hpp>


using namespace ComponentEngine;

std::map<std::string, ModelBuffers> Mesh::m_mdel_buffer_instances;
std::map<std::string, ModelPoolData> Mesh::m_model_pools;
std::map<std::string, ShaderStorage> Mesh::m_shaders;
const unsigned int Mesh::m_buffer_size_step = 100;

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

	if (it == m_mdel_buffer_instances.end())
	{
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
		if (!ret)
		{
			m_loaded = false;
			return;
		}

		// TEMP
		IGraphicsPipeline* last_created_pipeline;

		for (auto& m : materials)
		{

			if (m_shaders.find(m.name) == m_shaders.end())
			{
				// For now use the same shader for everything, just create duplicate pipelines to emulate the functionality


				IGraphicsPipeline* pipeline = Engine::Singlton()->GetRenderer()->CreateGraphicsPipeline({
					{ ShaderStage::VERTEX_SHADER, "../../ComponentEngine-demo/Shaders/Textured/vert.spv" },
					{ ShaderStage::FRAGMENT_SHADER, "../../ComponentEngine-demo/Shaders/Textured/frag.spv" }
					});

				last_created_pipeline = pipeline;

				pipeline->AttachVertexBinding({
					VertexInputRate::INPUT_RATE_VERTEX,
					{
						{ 0, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,position) },
					{ 1, DataFormat::R32G32_FLOAT,offsetof(MeshVertex,uv) },
					{ 2, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,normal) },
					{ 3, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,color) },
					},
					sizeof(MeshVertex),
					0
					});

				pipeline->AttachVertexBinding({
					VertexInputRate::INPUT_RATE_INSTANCE,
					{
						{ 4, DataFormat::MAT4_FLOAT,0 }
					},
					sizeof(glm::mat4),
					1
					});


				// Tell the pipeline what the input data will be payed out like
				pipeline->AttachDescriptorPool(Engine::Singlton()->GetCameraPool());
				// Attach the camera descriptor set to the pipeline
				pipeline->AttachDescriptorSet(0, Engine::Singlton()->GetCameraDescriptorSet());


				IDescriptorPool* texture_pool = Engine::Singlton()->GetRenderer()->CreateDescriptorPool({
					Engine::Singlton()->GetRenderer()->CreateDescriptor(Renderer::DescriptorType::IMAGE_SAMPLER, Renderer::ShaderStage::FRAGMENT_SHADER, 0),
					});
				pipeline->AttachDescriptorPool(texture_pool);

				pipeline->Build();


				m_shaders[m.name].pipeline = pipeline;


			}


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

		m_mdel_buffer_instances[m_path].vertexBuffer =
			Engine::Singlton()->GetRenderer()->CreateVertexBuffer(vertexData.data(), sizeof(MeshVertex), vertexData.size());
		m_mdel_buffer_instances[m_path].vertexBuffer->SetData();

		m_mdel_buffer_instances[m_path].indexBuffer =
			Engine::Singlton()->GetRenderer()->CreateIndexBuffer(indexData.data(), sizeof(uint16_t), indexData.size());
		m_mdel_buffer_instances[m_path].indexBuffer->SetData();

		IModelPool* model_pool = Engine::Singlton()->GetRenderer()->CreateModelPool
		(m_mdel_buffer_instances[m_path].vertexBuffer, m_mdel_buffer_instances[m_path].indexBuffer);


		glm::mat4* model_position_array = new glm::mat4[m_buffer_size_step];

		IUniformBuffer* model_position_buffer = Engine::Singlton()->GetRenderer()->CreateUniformBuffer(model_position_array, sizeof(glm::mat4), m_buffer_size_step);


		model_pool->AttachBuffer(0, model_position_buffer);

		last_created_pipeline->AttachModelPool(model_pool);

		// Populate the model pool map
		m_model_pools[m_path].model_pool = model_pool;
		m_model_pools[m_path].model_position_buffer = model_position_buffer;
		m_model_pools[m_path].model_position_array = model_position_array;
	}
	
	IModel* model = m_model_pools[m_path].model_pool->CreateModel();
	model->GetData<glm::mat4>(0) = glm::mat4(1.0f);
	m_model_pools[m_path].model_pool->Update();

	m_model = model;

	m_loaded = true;
}

ComponentEngine::ModelBuffers::~ModelBuffers()
{
	delete vertexBuffer;
	delete indexBuffer;
}

ComponentEngine::ShaderStorage::~ShaderStorage()
{
	delete pipeline;
}
