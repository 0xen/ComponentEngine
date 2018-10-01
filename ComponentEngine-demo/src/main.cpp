#include <ComponentEngine\Engine.hpp>
#include <iostream>
#include <vector>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* engine;


int main(int argc, char **argv)
{
	engine = new Engine();

	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();

	Entity* entity = em.CreateEntity();
	entity->AddComponent<Transformation>();



	std::vector<DefaultMeshVertex> vertex_data = {
		DefaultMeshVertex(glm::vec3(1.0f,1.0f,0.0f)),
		DefaultMeshVertex(glm::vec3(1.0f,-1.0f,0.0f)),
		DefaultMeshVertex(glm::vec3(-1.0f,-1.0f,0.0f)),
		DefaultMeshVertex(glm::vec3(-1.0f,1.0f,0.0f))
	};

	std::vector<uint16_t> index_data{
		0,1,2,
		0,2,3
	};
	IVertexBuffer* vertex_buffer = renderer->CreateVertexBuffer(vertex_data.data(), sizeof(DefaultMeshVertex), vertex_data.size());
	IIndexBuffer* index_buffer = renderer->CreateIndexBuffer(index_data.data(), sizeof(uint16_t), index_data.size());

	vertex_buffer->SetData();
	index_buffer->SetData();



	IModelPool* model_pool = renderer->CreateModelPool(vertex_buffer, index_buffer);

	// Create a position buffer for the model pool
	glm::mat4* model_position_array = new glm::mat4[10];
	IUniformBuffer* model_position_buffer = renderer->CreateUniformBuffer(model_position_array, sizeof(glm::mat4), 1);

	// Attach the buffer to buffer index 0
	model_pool->AttachBuffer(0, model_position_buffer);


	IModel* model = model_pool->CreateModel();


	// Create the model position data
	glm::mat4 model_pos = glm::mat4(1.0f);
	model_pos = glm::translate(model_pos, glm::vec3(2, 0, -20));
	model_pos = glm::scale(model_pos, glm::vec3(1.0f, 1.0f, 1.0f));

	// Set the data to buffer index 0
	model->SetData(0, model_pos);

	// Update all model positions
	model_position_buffer->SetData();

	engine->GetDefaultGraphicsPipeline()->AttachModelPool(model_pool);


	while (engine->Running())
	{



		engine->Update();
		engine->Render();

	}



	delete model_position_buffer;
	delete engine;
    return 0;
}
