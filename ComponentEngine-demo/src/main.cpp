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

	Entity* entity = em.CreateEntity();
	entity->AddComponent<Transformation>();


	while (engine->Running())
	{



		engine->Update();
		engine->Render();

	}

	delete engine;
    return 0;
}
