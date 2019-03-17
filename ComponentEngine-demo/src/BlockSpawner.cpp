#include <BlockSpawner.hpp>


#include <ComponentEngine/Engine.hpp>
#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>

#include <ComponentEngine/Components/Renderer.hpp>
#include <ComponentEngine/Components/Mesh.hpp>
#include <ComponentEngine/Components/BoxCollision.hpp>
#include <ComponentEngine/Components/Rigidbody.hpp>
#include <ComponentEngine/Components/ParticalSystem.hpp>
#include <TimedDestruction.hpp>


ComponentEngine::BlockSpawner::BlockSpawner(enteez::Entity * entity)
{
	m_entity = entity;
	m_timeBetween = 1.0f;
	m_delta = m_timeBetween;
}

void ComponentEngine::BlockSpawner::Update(float frame_time)
{
	m_delta += frame_time;
	if (m_delta > m_timeBetween)
	{
		m_delta -= m_timeBetween;

		Engine* engine = Engine::Singlton();
		engine->GetLogicMutex().lock();
		engine->GetRendererMutex().lock();

		Entity* blockEntity = engine->GetEntityManager().CreateEntity("Block");

		{ // Transformarion
			Transformation::EntityHookDefault(*blockEntity);
			Transformation& trans = blockEntity->GetComponent<Transformation>();
			trans.SetParent(m_entity);
		}
		{// Renderer
			RendererComponent::EntityHookDefault(*blockEntity);
		}

		int randomN = (rand() + (int)(frame_time * 100)) % 5;

		{// Mesh
			Mesh::EntityHookDefault(*blockEntity);
			Mesh& mesh = blockEntity->GetComponent<Mesh>();
			static const std::string paths[5] = {
			"../Resources/Models/Blocks/LetterBlockA.obj",
			"../Resources/Models/Blocks/LetterBlockB.obj",
			"../Resources/Models/Blocks/LetterBlockC.obj",
			"../Resources/Models/Blocks/LetterBlockD.obj",
			"../Resources/Models/Blocks/LetterBlockE.obj",
			};
			mesh.ChangePath(paths[randomN]);
			mesh.LoadModel();
			if (!mesh.Loaded())
			{
				engine->Log("Failed to load block in block spawner", ConsoleState::Error);
			}
		}
		{ // BoxCollision
			BoxCollision::EntityHookDefault(*blockEntity);
		}
		{ // Rigidbody
			Rigidbody::EntityHookDefault(*blockEntity);
			static const float mass[5] = {
				1.0f,
				20.0f,
				50.0f,
				100.0f,
				1000.0f
			};
			blockEntity->GetComponent<Rigidbody>().SetMass(mass[randomN]);
		}
		{ // TimedDestruction
			TimedDestruction::EntityHookDefault(*blockEntity);
		}

		
		engine->GetRendererMutex().unlock();
		engine->GetLogicMutex().unlock();

	}
}

void ComponentEngine::BlockSpawner::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<BlockSpawner>* wrapper = entity.AddComponent<BlockSpawner>(&entity);
	wrapper->SetName("BlockSpawner");
}

void ComponentEngine::BlockSpawner::EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data)
{
	enteez::ComponentWrapper<BlockSpawner>* wrapper = entity.AddComponent<BlockSpawner>(&entity);
	wrapper->SetName("BlockSpawner");

	BlockSpawner& spawner = wrapper->Get();

}