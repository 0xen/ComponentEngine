#include <ItemHover.hpp>

#include <ComponentEngine\Components\Transformation.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>

ComponentEngine::ItemHover::ItemHover(enteez::Entity * entity)
{
	m_entity = entity;
	m_delta_time = 0.0f;
	m_last_move_distance = 0.0f;
	m_move_distance = 0.5f;
	m_spin_speed = 90.0f;
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
	m_entity->GetComponent<Transformation>().Rotate(glm::vec3(0.0f, m_spin_speed * frame_time, 0.0f));
}

void ComponentEngine::ItemHover::Display()
{
	{
		ImGui::PushID(0);
		ImGui::Text("Move Distance");
		float change = m_move_distance;
		if (ImGui::InputFloat("", &change, 0.01f, 0.1f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			// Move the temp, but changed value into the main one
			m_move_distance = change;
		}
		ImGui::PushItemWidth(-(ImGui::GetWindowContentRegionWidth() - ImGui::CalcItemWidth()));
		ImGui::PopID();
	}
	{
		ImGui::PushID(1);
		ImGui::Text("Spin Speed");
		float change = m_spin_speed;
		if (ImGui::InputFloat("", &change, 1.0f, 10.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			// Move the temp, but changed value into the main one
			m_spin_speed = change;
		}
		ImGui::PushItemWidth(-(ImGui::GetWindowContentRegionWidth() - ImGui::CalcItemWidth()));
		ImGui::PopID();
	}
}//

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
