#include <WaterSourceController.hpp>

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

enteez::BaseComponentWrapper* ComponentEngine::WaterSourceController::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<WaterSourceController>* wrapper = entity.AddComponent<WaterSourceController>(&entity);
	wrapper->SetName("Water Source Controller");
	return wrapper;
}

void ComponentEngine::WaterSourceController::ReciveMessage(enteez::Entity * sender, OnCollisionEnter & message)
{
	if (message.collider->HasComponent<Flamable>() && message.collider->GetComponent<Flamable>().OnFire())
	{
		message.collider->GetComponent<Flamable>().SetOnFire(false);
	}
}
