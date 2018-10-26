#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\ParticalSystem.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* engine;


std::mutex mtx;


class ResourceBase
{
protected:
	void* m_resource;
};

template <typename T>
class Resource : public ResourceBase
{
public:
	Resource(T* data) : m_resource(data) {}
	T& Get()
	{
		return *static_cast<T*>(m_resource);
	}

};



class ThreadingManager
{
public:

private:

};

ThreadHandler* render_thread = nullptr;
ThreadHandler* logic_thread = nullptr;

int main_data = 0;
int seccondery_data = 0;

/*void RenderThread()
{
	while (true)
	{
		if (render_thread == nullptr)continue;
		std::lock_guard<std::mutex> render_lock(render_thread->ThreadLock());
		std::cout << main_data << std::endl;
	}
}


void LogicThread()
{
	while (true)
	{
		if (render_thread == nullptr || logic_thread == nullptr)continue;
		// On Update
		std::lock_guard<std::mutex> thread_lock(logic_thread->ThreadLock());
		seccondery_data++;
		if (seccondery_data % 10 == 0) // Need to update renderer
		{
			int temp;
			{
				std::lock_guard<std::mutex> render_lock(render_thread->ThreadLock());
				temp = main_data;
				main_data = seccondery_data;
			}
			// Do something...like deletion of old main data
		}
	}
}*/

void LogicThread()
{
	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();

	engine->GetCameraTransformation()->Translate(glm::vec3(0.0f, 0.0f, 10.0f));

	std::vector<DefaultMeshVertex> vertexData = {
		DefaultMeshVertex(glm::vec4(1.0f,1.0f,0.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f)),
		DefaultMeshVertex(glm::vec4(1.0f,-1.0f,0.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f)),
		DefaultMeshVertex(glm::vec4(-1.0f,-1.0f,0.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f)),
		DefaultMeshVertex(glm::vec4(-1.0f,1.0f,0.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f))
	};
	IVertexBuffer* vertexBuffer = renderer->CreateVertexBuffer(vertexData.data(), sizeof(DefaultMeshVertex), vertexData.size());
	vertexBuffer->SetData();

	std::vector<uint16_t> indexData{
		0,1,2,
		0,2,3
	};
	IIndexBuffer* indexBuffer = renderer->CreateIndexBuffer(indexData.data(), sizeof(uint16_t), indexData.size());
	indexBuffer->SetData();

	IModelPool* model_pool = renderer->CreateModelPool(vertexBuffer, indexBuffer);

	unsigned int model_array_size = 100;
	glm::mat4* model_position_array = new glm::mat4[model_array_size];
	for (int i = 0; i < model_array_size; i++)
	{
		model_position_array[i] = glm::mat4(1.0f);
	}
	IUniformBuffer* model_position_buffer = renderer->CreateUniformBuffer(model_position_array, sizeof(glm::mat4), model_array_size);

	// Attach the buffer to buffer index 0
	model_pool->AttachBuffer(0, model_position_buffer);
	float padding_scale = 1.3f;
	int width = 10;
	int height = 10;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			Entity* entity = em.CreateEntity();
			IModel * model = entity->AddComponent(model_pool->CreateModel());
			Transformation* transform = entity->AddComponent<Transformation>(&model->GetData<glm::mat4>(0));
			transform->Translate(glm::vec3(
				-(width * padding_scale / 2) + (x * padding_scale),
				-(height * padding_scale / 2) + (y * padding_scale),
				0.0f
				));
			transform->Scale(glm::vec3(0.3f, 0.3f, 0.3f));
		}
	}


	engine->GetDefaultGraphicsPipeline()->AttachModelPool(model_pool);

	// Logic Updating
	while (engine->Running())
	{


		engine->GetRendererMutex().lock();
		em.ForEach<Transformation>([](enteez::Entity* entity, Transformation& transformation)
		{
			if (entity != engine->GetCameraEntity())
			{
				transformation.Rotate(glm::vec3(0.0f, 0.0f, 1.0f), 1.0f * engine->GetFrameTime());
			}
		}, true);
		model_pool->Update();
		engine->GetRendererMutex().unlock();
	}

}

int main(int argc, char **argv)
{

	engine = new Engine();

	engine->Start(LogicThread);


	// Rendering
	while (engine->Running())
	{

		engine->Update();
		engine->GetRendererMutex().lock();
		engine->RenderFrame();
		engine->GetRendererMutex().unlock();
	}
	delete engine;

    return 0;
}
