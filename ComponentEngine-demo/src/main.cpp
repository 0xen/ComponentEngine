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

	ParticalSystemConfiguration ps_config;
	ps_config.data.emiter_location = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	ps_config.data.start_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	ps_config.data.end_color = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
	ps_config.data.frame_time = 0.0f;
	ps_config.data.partical_count = 5000;

	{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(-1.0f, 1.0f, -5.0f));
		ParticalSystem* ps = entity->AddComponent<ParticalSystem>(entity, engine);
		ps->GetConfig() = ps_config;
		ps->Build();
	}

	{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(1.0f, 1.0f, -5.0f));
		ParticalSystem* ps = entity->AddComponent<ParticalSystem>(entity, engine);
		ps->GetConfig() = ps_config;
		ps->Build();
	}

	{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(-1.0f, -1.0f, -5.0f));
		ParticalSystem* ps = entity->AddComponent<ParticalSystem>(entity, engine);
		ps->GetConfig() = ps_config;
		ps->Build();
	}

	{
		Entity* entity = em.CreateEntity();
		Transformation* transform = entity->AddComponent<Transformation>();
		transform->Translate(glm::vec3(1.0f, -1.0f, -5.0f));
		ParticalSystem* ps = entity->AddComponent<ParticalSystem>(entity, engine);
		ps->GetConfig() = ps_config;
		ps->Build();
	}

	int tick = 0;
	float rotation = 0.01f;
	while (engine->Running())
	{
		tick++;



		em.ForEach<ParticalSystem, Transformation>([rotation](enteez::Entity* entity, ParticalSystem& ps, Transformation& trans)
		{
			trans.Rotate(glm::vec3(0.0f, 0.0f, 1.0f), rotation);
			ps.Update();
		}, true);



		if (tick % 2 == 0)
		{
			em.ForEach<ParticalSystem>([](enteez::Entity* entity, ParticalSystem& ps)
			{
				ps.Rebuild();
			}, true);
		}


		engine->Update();
		engine->Render();

	}


	delete engine;
    return 0;
}
