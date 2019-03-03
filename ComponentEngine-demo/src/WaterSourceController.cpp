#include <WaterSourceController.hpp>



#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <Flamable.hpp>


ComponentEngine::WaterSourceController::WaterSourceController(enteez::Entity * entity)
{
	m_entity = entity;
}

void ComponentEngine::WaterSourceController::Display()
{

}

void ComponentEngine::WaterSourceController::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<WaterSourceController>* wrapper = entity.AddComponent<WaterSourceController>(&entity);
	wrapper->SetName("Water Source Controller");
}

void ComponentEngine::WaterSourceController::EntityHookXML(enteez::Entity & entity, pugi::xml_node & component_data)
{
	enteez::ComponentWrapper<WaterSourceController>* wrapper = entity.AddComponent<WaterSourceController>(&entity);
	wrapper->SetName("WaterSourceController");
}

void ComponentEngine::WaterSourceController::ReciveMessage(enteez::Entity * sender, OnCollisionEnter & message)
{
	if (message.collider->HasComponent<Flamable>() && message.collider->GetComponent<Flamable>().OnFire())
	{
		message.collider->GetComponent<Flamable>().SetOnFire(false);
	}
}
