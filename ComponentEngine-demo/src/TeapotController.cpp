#include <TeapotController.hpp>

#include <Flamable.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>
#include <ComponentEngine\Components\MsgSend.hpp>


ComponentEngine::TeapotController::TeapotController(enteez::Entity * entity)
{
	m_entity = entity;
}

void ComponentEngine::TeapotController::Display()
{

}

enteez::BaseComponentWrapper* ComponentEngine::TeapotController::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<TeapotController>* wrapper = entity.AddComponent<TeapotController>(&entity);
	wrapper->SetName("Teapot Controller");
	return wrapper;
}

void ComponentEngine::TeapotController::ReciveMessage(enteez::Entity * sender, OnCollisionEnter & message)
{
	if (message.collider->HasComponent<Flamable>() && message.collider->GetComponent<Flamable>().OnFire())
	{

		Send(m_entity, m_entity, ParticleSystemVisibility{ true }, true);
	}
}

void ComponentEngine::TeapotController::ReciveMessage(enteez::Entity * sender, OnCollisionExit & message)
{
	if (message.collider->HasComponent<Flamable>() && message.collider->GetComponent<Flamable>().OnFire())
	{
		Send(m_entity, m_entity, ParticleSystemVisibility{ false }, true);
	}
}
