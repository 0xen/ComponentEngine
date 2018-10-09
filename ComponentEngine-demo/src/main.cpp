#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\ParticalSystem.hpp>
#include <iostream>
#include <vector>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* engine;

class ProcessTask
{
public:
	virtual void Update() = 0;
};

class Model : public ProcessTask
{
public:
	Model(Entity* entity, IModelPool* model_pool) : m_entity(entity), m_model(model_pool->CreateModel())
	{
	}
	~Model()
	{
		delete m_model;
	}
	virtual void Update()
	{
		if (m_entity->HasComponent<Transformation>())
		{
			// Set the data to buffer index 0
			m_model->SetData(0, m_entity->GetComponent<Transformation>().Get());
		}
	}
private:
	Entity * m_entity;
	IModel * m_model;
};



int main(int argc, char **argv)
{
	engine = new Engine();
	engine->RegisterBase<Model, ProcessTask>();

	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();

	//{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(0.0f, 0.0f, -5.0f));
		//entity->AddComponent<Model>(entity, model_pool);



		ParticalSystem* ps = entity->AddComponent<ParticalSystem>(engine);
		ParticalSystemConfiguration& ps_config = ps->GetConfig();
		ps_config.data.emiter_location = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		ps_config.data.start_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		ps_config.data.end_color = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
		ps_config.data.frame_time = 0.0f;
		ps_config.data.partical_count = 10000;

		ps->Build();

	//}


	for (auto e : em.GetEntitys())
	{
		e->ForEach<ProcessTask>([](enteez::Entity* entity, ProcessTask& process_task)
		{
			process_task.Update();
		});
	}

	while (engine->Running())
	{

		
		ps->Update();


		engine->Update();
		engine->Render();

	}



	//delete model_position_buffer;
	delete engine;
    return 0;
}
