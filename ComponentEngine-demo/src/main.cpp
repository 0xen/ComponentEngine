#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\ParticalSystem.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include <lodepng.h>
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <ComponentEngine\tiny_obj_loader.h>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* engine;

IGraphicsPipeline* textured_pipeline = nullptr;

class MeshVertex
{
public:
	MeshVertex() {};
	MeshVertex(glm::vec3 position, glm::vec2 uv, glm::vec3 normal, glm::vec3 color) : position(position), uv(uv), normal(normal), color(color) {}
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 color;
};



void LogicThread()
{
	engine->GetRendererMutex().lock();

	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();


	// Load the scene
	engine->LoadScene("../../ComponentEngine-demo/Scenes/GameInstance.xml");

	Transformation* camera = engine->GetCameraTransformation();

	camera->Translate(glm::vec3(0.0f, 0.0f, 10.0f));

	// Create texture pipeline

	IGraphicsPipeline* textured_pipeline = renderer->CreateGraphicsPipeline({
		{ ShaderStage::VERTEX_SHADER, "../../ComponentEngine-demo/Shaders/Textured/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../../ComponentEngine-demo/Shaders/Textured/frag.spv" }
		});

	textured_pipeline->AttachVertexBinding({
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

	textured_pipeline->AttachVertexBinding({
		VertexInputRate::INPUT_RATE_INSTANCE,
		{
			{ 4, DataFormat::MAT4_FLOAT,0 }
		},
		sizeof(glm::mat4),
		1
		});


	// Tell the pipeline what the input data will be payed out like
	textured_pipeline->AttachDescriptorPool(engine->GetCameraPool());
	// Attach the camera descriptor set to the pipeline
	textured_pipeline->AttachDescriptorSet(0, engine->GetCameraDescriptorSet());


	IDescriptorPool* texture_pool = renderer->CreateDescriptorPool({
		renderer->CreateDescriptor(Renderer::DescriptorType::IMAGE_SAMPLER, Renderer::ShaderStage::FRAGMENT_SHADER, 0),
		});
	textured_pipeline->AttachDescriptorPool(texture_pool);

	textured_pipeline->Build();


	std::vector<unsigned char> image; //the raw pixels
	unsigned width;
	unsigned height;
	unsigned error = lodepng::decode(image, width, height, "../../ComponentEngine-demo/Resources/Resources/cobble.png");
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	ITextureBuffer* texture = renderer->CreateTextureBuffer(image.data(), Renderer::DataFormat::R8G8B8A8_FLOAT, width, height);
	texture->SetData();

	IDescriptorSet* texture_descriptor_set1 = texture_pool->CreateDescriptorSet();
	texture_descriptor_set1->AttachBuffer(0, texture);
	texture_descriptor_set1->UpdateSet();


	// Test OBJ Loader



	std::string inputfile = "../../ComponentEngine-demo/Resources/Models/cube.obj";
	std::string material_base_dir = "../../ComponentEngine-demo/Resources/Resources/";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str(), material_base_dir.c_str());

	if (!err.empty())
	{ // `err` may contain warning message.
		std::cerr << err << std::endl;
	}

	if (!ret)
	{
		exit(1);
	}

	std::vector<MeshVertex> testVertexData;
	std::vector<uint16_t> testIndexData;

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
			testVertexData.push_back(vertex);
			testIndexData.push_back(testIndexData.size());
			//shape.mesh.material_ids[1];
			//materials[1].
		}
	}

	IVertexBuffer* vertexBuffer = renderer->CreateVertexBuffer(testVertexData.data(), sizeof(MeshVertex), testIndexData.size());
	vertexBuffer->SetData();

	IIndexBuffer* indexBuffer = renderer->CreateIndexBuffer(testIndexData.data(), sizeof(uint16_t), testIndexData.size());
	indexBuffer->SetData();

	IModelPool* model_pool = renderer->CreateModelPool(vertexBuffer, indexBuffer);


	model_pool->AttachDescriptorSet(1, texture_descriptor_set1);


	unsigned int model_array_size = 40000;
	glm::mat4* model_position_array = new glm::mat4[model_array_size];
	for (int i = 0; i < model_array_size; i++)
	{
		model_position_array[i] = glm::mat4(1.0f);
	}

	IUniformBuffer* model_position_buffer = renderer->CreateUniformBuffer(model_position_array, sizeof(glm::mat4), model_array_size);

	// Attach the buffer to buffer index 0
	model_pool->AttachBuffer(0, model_position_buffer);
	float padding_scale = 2.0f;
	int grid_width = 10;
	int grid_height = 10;
	float scale = 1.0f;
	for (int x = 0; x < grid_width; x++)
	{
		for (int y = 0; y < grid_height; y++)
		{
			Entity* entity = em.CreateEntity();
			IModel * model = entity->AddComponent(model_pool->CreateModel());
			Transformation* transform = entity->AddComponent<Transformation>(&model->GetData<glm::mat4>(0));
			transform->Translate(glm::vec3(
				-(grid_width * padding_scale / 2) + (x * padding_scale),
				-(grid_height * padding_scale / 2) + (y * padding_scale),
				0.0f
				));
			transform->Scale(glm::vec3(scale, scale, scale));
		}
	}

	textured_pipeline->AttachModelPool(model_pool);

	engine->GetRendererMutex().unlock();
	// Logic Updating
	while (engine->Running())
	{
		float thread_time = engine->GetThreadTime();
		camera->Translate(glm::vec3(0.0f, 0.0f, 1.0f*thread_time));
		em.ForEach<Transformation>([thread_time, camera](enteez::Entity* entity, Transformation& transformation)
		{
			if (&transformation != camera)
			{
				transformation.Rotate(glm::vec3(0.0f, 0.0f, 1.0f), 1.0f * thread_time);
			}
		}, true);

		engine->GetRendererMutex().lock();
		std::cout << "L:" << thread_time << std::endl;
		model_pool->Update();
		engine->GetRendererMutex().unlock();
	}

	delete texture;

}

#include <ComponentEngine\pugixml.hpp>


pugi::xml_node LoadScene(pugi::xml_node& xml)
{
	pugi::xml_node game_node = xml.child("Game");
	if (!game_node)return game_node;
	pugi::xml_node scenes_node = game_node.child("Scenes");
	if (!scenes_node)return scenes_node;
	pugi::xml_node scene_node = scenes_node.child("Scene");
	return scene_node;
}


void SetupScene(pugi::xml_node& xml)
{


	for (pugi::xml_node node : xml.children("GameObject"))
	{

	}
}



void TestXML()
{
	pugi::xml_document xml;
	pugi::xml_parse_result result = xml.load_file("../../ComponentEngine-demo/Scenes/GameInstance.xml");

	pugi::xml_node scene;
	if (scene = LoadScene(xml))
	{
		std::cout << "Yes" << std::endl;
		SetupScene(scene);
	}


}


int main(int argc, char **argv)
{


	//TestXML();






	//exit(0);


	engine = new Engine();

	engine->Start(LogicThread);

	// Rendering
	while (engine->Running())
	{

		engine->Update();
		engine->GetRendererMutex().lock();
		std::cout << "R:" << engine->GetFrameTime() << std::endl;
		engine->RenderFrame();
		engine->GetRendererMutex().unlock();
	}
	delete engine;

    return 0;
}
