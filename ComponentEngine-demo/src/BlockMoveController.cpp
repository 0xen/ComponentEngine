#include <BlockMoveController.hpp>



#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\Transformation.hpp>
#include <Flamable.hpp>


ComponentEngine::BlockMoveController::BlockMoveController(enteez::Entity * entity)
{
	m_entity = entity;
	m_speed = 1.5f;
}

void ComponentEngine::BlockMoveController::Update(float frame_time)
{
	if (m_entity->HasComponent<Transformation>() && m_entity->HasComponent<Flamable>())
	{
		Transformation& trans = m_entity->GetComponent<Transformation>();
		Flamable& framable = m_entity->GetComponent<Flamable>();
		trans.MoveWorldX((framable.OnFire() ? -m_speed : m_speed) * frame_time);
	}
}

void ComponentEngine::BlockMoveController::Display()
{
}


enteez::BaseComponentWrapper* ComponentEngine::BlockMoveController::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<BlockMoveController>* wrapper = entity.AddComponent<BlockMoveController>(&entity);
	wrapper->SetName("Block Move Controller");
	return wrapper;
}

void ComponentEngine::BlockMoveController::EntityHookXML(enteez::Entity & entity, pugi::xml_node & component_data)
{
	enteez::ComponentWrapper<BlockMoveController>* wrapper = entity.AddComponent<BlockMoveController>(&entity);
	wrapper->SetName("Block Move Controller");

}