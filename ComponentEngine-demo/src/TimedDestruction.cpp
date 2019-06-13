#include <TimedDestruction.hpp>


#include <ComponentEngine/Engine.hpp>
#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>
#include <ComponentEngine\Components\MsgSend.hpp>


void ComponentEngine::TimedDestruction::DestroyEntity(enteez::Entity* entity)
{

	Transformation& transformation = entity->GetComponent<Transformation>();
	if (transformation.HasChildren())
	{
		for (auto t : transformation.GetChildren())
		{
			DestroyEntity(t->GetEntity());
		}
	}
	entity->Destroy();
}

ComponentEngine::TimedDestruction::TimedDestruction(enteez::Entity * entity)
{
	m_entity = entity;
	m_timetillDealth = 11.0f; 
	m_delta = 0.0f;
}

void ComponentEngine::TimedDestruction::Update(float frame_time)
{
	m_delta += frame_time;
	if (m_delta > m_timetillDealth)
	{
		Engine::Singlton()->GetThreadManager()->AddTask([&](float frameTime) {

			Engine::Singlton()->GetLogicMutex().lock();
			Engine::Singlton()->GetRendererMutex().lock();

			DestroyEntity(m_entity);

			Engine::Singlton()->GetRendererMutex().unlock();
			Engine::Singlton()->GetLogicMutex().unlock();

		});
	}
}

enteez::BaseComponentWrapper* ComponentEngine::TimedDestruction::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<TimedDestruction>* wrapper = entity.AddComponent<TimedDestruction>(&entity);
	wrapper->SetName("TimedDestruction");
	return wrapper;
}

void ComponentEngine::TimedDestruction::EntityHookXML(enteez::Entity & entity, pugi::xml_node & component_data)
{
	enteez::ComponentWrapper<TimedDestruction>* wrapper = entity.AddComponent<TimedDestruction>(&entity);
	wrapper->SetName("TimedDestruction");

	TimedDestruction& flamale = wrapper->Get();

}
