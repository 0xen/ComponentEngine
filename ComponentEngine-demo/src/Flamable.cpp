#include <Flamable.hpp>



#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>
#include <ComponentEngine\Components\MsgSend.hpp>


ComponentEngine::Flamable::Flamable(enteez::Entity * entity)
{
	m_entity = entity;
	m_onFire = false;
}

bool ComponentEngine::Flamable::OnFire()
{
	return m_onFire;
}

void ComponentEngine::Flamable::SetOnFire(bool fire)
{
	m_onFire = fire;
	Send(m_entity, ParticleSystemVisibility{ fire }, true);
}

void ComponentEngine::Flamable::Display()
{
	if (ImGui::Checkbox("On Fire", &m_onFire))
	{
		SetOnFire(m_onFire);
	}
}

void ComponentEngine::Flamable::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<Flamable>* wrapper = entity.AddComponent<Flamable>(&entity);
	wrapper->SetName("Flamable");
}

void ComponentEngine::Flamable::EntityHookXML(enteez::Entity & entity, pugi::xml_node & component_data)
{
	enteez::ComponentWrapper<Flamable>* wrapper = entity.AddComponent<Flamable>(&entity);
	wrapper->SetName("Flamable");

	Flamable& flamale = wrapper->Get();

	flamale.SetOnFire(component_data.attribute("onFire").as_bool(false));
}

void ComponentEngine::Flamable::ReciveMessage(enteez::Entity * sender, OnCollisionEnter & message)
{
	if (message.collider->HasComponent<Flamable>() && message.collider->GetComponent<Flamable>().OnFire() && !m_onFire)
	{
		SetOnFire(true);
	}
}
