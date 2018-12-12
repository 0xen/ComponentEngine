#include <ItemHover.hpp>

#include <ComponentEngine\Components\Transformation.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>

ComponentEngine::ItemHover::ItemHover(enteez::Entity * entity)
{
	m_entity = entity;
	m_delta_time = 0.0f;
	m_last_move_distance = 0.0f;
	m_move_distance = 0.5f;
}

void ComponentEngine::ItemHover::Update(float frame_time)
{
	// Get the current Y position from the transformation component
	float current_y = m_entity->GetComponent<Transformation>().GetWorldY();
	// Calculate the center pivot point
	float center_y_pos = current_y - m_last_move_distance;
	// Calculate the new frame time
	m_delta_time += frame_time;
	// Store the current move we will be making in last move
	m_last_move_distance = (sin(m_delta_time) * m_move_distance);
	// Set the object in its new position
	m_entity->GetComponent<Transformation>().SetWorldY(center_y_pos + m_last_move_distance);

	// Rotate the object around the world Y
	m_entity->GetComponent<Transformation>().Rotate(glm::vec3(0.0f, 90.0f * frame_time, 0.0f));
}

void ComponentEngine::ItemHover::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<ItemHover>* wrapper = entity.AddComponent<ItemHover>(&entity);
	wrapper->SetName("ItemHover");
}

void ComponentEngine::ItemHover::EntityHookXML(enteez::Entity & entity, pugi::xml_node & component_data)
{
	enteez::ComponentWrapper<ItemHover>* wrapper = entity.AddComponent<ItemHover>(&entity);
	wrapper->SetName("ItemHover");
}
