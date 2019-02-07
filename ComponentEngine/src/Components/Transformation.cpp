#include <ComponentEngine\Components\Transformation.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <imgui.h>

using namespace ComponentEngine;

void ComponentEngine::Transformation::MemoryPointTo(glm::mat4 * new_mat4, bool transfer_old_data)
{
	if (transfer_old_data)
		memcpy(new_mat4, m_mat4, sizeof(glm::mat4));
	if (m_origional)
	{
		m_origional = false;
		// Cleanup as this memory was reserved by us
		delete m_mat4;
	}
	m_mat4 = new_mat4;
	PushToPositionArray();
}

glm::mat4 & Transformation::Get()
{
	return *m_mat4;
}

void Transformation::Translate(glm::vec3 translation)
{
	m_local_mat4 = glm::translate(m_local_mat4, translation);
	PushToPositionArray();
}

void ComponentEngine::Transformation::SetWorldX(float x)
{
	(m_local_mat4)[3][0] = x;
	PushToPositionArray();
}

void ComponentEngine::Transformation::SetWorldY(float y)
{
	(m_local_mat4)[3][1] = y;
	PushToPositionArray();
}

void ComponentEngine::Transformation::SetWorldZ(float z)
{
	(m_local_mat4)[3][2] = z;
	PushToPositionArray();
}

void ComponentEngine::Transformation::MoveWorldX(float x)
{
	(m_local_mat4)[3][0] += x;
	PushToPositionArray();
}

void ComponentEngine::Transformation::MoveWorldY(float y)
{
	(m_local_mat4)[3][1] += y;
	PushToPositionArray();
}

void ComponentEngine::Transformation::MoveWorldZ(float z)
{
	(m_local_mat4)[3][2] += z;
	PushToPositionArray();
}

void ComponentEngine::Transformation::MoveLocalX(float x)
{
	glm::vec3 xFacing = m_local_mat4[0];
	xFacing = glm::normalize(xFacing) * x;
	(m_local_mat4)[3][0] += xFacing.x;
	(m_local_mat4)[3][1] += xFacing.y;
	(m_local_mat4)[3][2] += xFacing.z;
	PushToPositionArray();
}

void ComponentEngine::Transformation::MoveLocalY(float y)
{
	glm::vec3 yFacing = m_local_mat4[1];
	yFacing = glm::normalize(yFacing) * y;
	(m_local_mat4)[3][0] += yFacing.x;
	(m_local_mat4)[3][1] += yFacing.y;
	(m_local_mat4)[3][2] += yFacing.z;
	PushToPositionArray();
}

void ComponentEngine::Transformation::MoveLocalZ(float z)
{
	glm::vec3 zFacing = m_local_mat4[2];
	zFacing = glm::normalize(zFacing) * z;
	(m_local_mat4)[3][0] += zFacing.x;
	(m_local_mat4)[3][1] += zFacing.y;
	(m_local_mat4)[3][2] += zFacing.z;
	PushToPositionArray();
}

float ComponentEngine::Transformation::GetWorldX()
{
	return (m_local_mat4)[3][0];
}

float ComponentEngine::Transformation::GetWorldY()
{
	return (m_local_mat4)[3][1];
}

float ComponentEngine::Transformation::GetWorldZ()
{
	return (m_local_mat4)[3][2];
}

glm::vec3 ComponentEngine::Transformation::GetLocalPosition()
{
	return (m_local_mat4)[3];
}

glm::vec3 ComponentEngine::Transformation::GetWorldPosition()
{
	return (*m_mat4)[3];
}

void Transformation::Scale(glm::vec3 scale)
{
	m_local_mat4 = glm::scale(m_local_mat4, scale);
	PushToPositionArray();
}

void ComponentEngine::Transformation::RotateWorldX(float x)
{
	Rotate(glm::vec3(1.0f, 0.0f, 0.0f), x);
}

void ComponentEngine::Transformation::RotateWorldY(float y)
{
	Rotate(glm::vec3(0.0f, 1.0f, 0.0f), y);
}

void ComponentEngine::Transformation::RotateWorldZ(float z)
{
	Rotate(glm::vec3(0.0f, 0.0f, 1.0f), z);
}

void ComponentEngine::Transformation::Rotate(glm::vec3 axis, float angle)
{
	//m_rotation
	m_local_mat4 = glm::rotate(m_local_mat4, angle, axis);
	PushToPositionArray();
}

void ComponentEngine::Transformation::ReciveMessage(enteez::Entity * sender, TransformationPtrRedirect & message)
{
	MemoryPointTo(message.mat_ptr, true);
}

void ComponentEngine::Transformation::Display()
{
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(m_local_mat4, scale, rotation, translation, skew, perspective);
	rotation = glm::conjugate(rotation);

	ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() - 10);
	{
		ImGui::PushID(0);
		ImGui::Text("Position");
		glm::vec3 change = translation;
		if (ImGui::InputFloat3("", (float*)&change, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			m_local_mat4 = glm::mat4(1.0f);
			m_local_mat4 = glm::translate(m_local_mat4, change);
			m_local_mat4 *= glm::inverse(glm::toMat4(rotation));
			m_local_mat4 = glm::scale(m_local_mat4, scale);
			PushToPositionArray();
		}
		ImGui::PopID();
	}
	{
		ImGui::PushID(1);
		ImGui::Text("Rotation");

		glm::vec3 euler = glm::eulerAngles(rotation);
		glm::vec3 change = glm::vec3(glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z));
		if (ImGui::InputFloat3("", (float*)&change, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			m_local_mat4 = glm::mat4(1.0f);
			m_local_mat4 = glm::translate(m_local_mat4, translation);
			m_local_mat4 *= glm::inverse(glm::toMat4(glm::quat(glm::vec3(glm::radians(change.x), glm::radians(change.y), glm::radians(change.z)))));
			m_local_mat4 = glm::scale(m_local_mat4, scale);
			PushToPositionArray();
		}
		ImGui::PopID();
	}
	{
		ImGui::PushID(2);
		ImGui::Text("Scale");
		glm::vec3 change = scale;
		if (ImGui::InputFloat3("", (float*)&change, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			m_local_mat4 = glm::mat4(1.0f);
			m_local_mat4 = glm::translate(m_local_mat4, translation);
			m_local_mat4 *= glm::inverse(glm::toMat4(rotation));
			m_local_mat4 = glm::scale(m_local_mat4, change);
			PushToPositionArray();
		}
		ImGui::PushItemWidth(-(ImGui::GetWindowContentRegionWidth() - ImGui::CalcItemWidth()));
		ImGui::PopID();
	}
}

void ComponentEngine::Transformation::Rotate(glm::vec3 angles)
{
	Rotate(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(angles.x));
	Rotate(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(angles.y));
	Rotate(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(angles.z));
}

void ComponentEngine::Transformation::SetParent(enteez::Entity* parent)
{
	if (m_parent != nullptr) m_parent->GetComponent<Transformation>().RemoveChild(this);
	m_parent = parent;
	if (m_parent != nullptr) m_parent->GetComponent<Transformation>().AddChild(this);
	PushToPositionArray();
}

enteez::Entity* ComponentEngine::Transformation::GetParent()
{
	return m_parent;
}

std::vector<Transformation*> ComponentEngine::Transformation::GetChildren()
{
	return m_children;
}

bool ComponentEngine::Transformation::HasChildren()
{
	return m_children.size()>0;
}

enteez::Entity * ComponentEngine::Transformation::GetEntity()
{
	return m_entity;
}

void ComponentEngine::Transformation::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<Transformation>* trans_wrapper = entity.AddComponent<Transformation>(&entity);
	trans_wrapper->SetName("Transformation");
}

void ComponentEngine::Transformation::EntityHookXML(enteez::Entity & entity, pugi::xml_node & component_data)
{


	std::vector<Transformation*> children;
	if(entity.HasComponent<Transformation>())
	{
		Transformation& trans = entity.GetComponent<Transformation>();
		children = trans.GetChildren();
	}

	enteez::ComponentWrapper<Transformation>* trans_wrapper = entity.AddComponent<Transformation>(&entity);
	trans_wrapper->SetName("Transformation");
	Transformation& trans = trans_wrapper->Get();
	pugi::xml_node position_node = component_data.child("Position");
	if (position_node)
	{
		trans.Translate(glm::vec3(
			position_node.attribute("x").as_float(),
			position_node.attribute("y").as_float(),
			position_node.attribute("z").as_float()
		));
	}
	pugi::xml_node rotation_node = component_data.child("Rotation");
	if (rotation_node)
	{
		trans.Rotate(glm::vec3(
			rotation_node.attribute("x").as_float(),
			rotation_node.attribute("y").as_float(),
			rotation_node.attribute("z").as_float()
		));
	}
	pugi::xml_node scale_node = component_data.child("Scale");
	if (scale_node)
	{
		trans.Scale(glm::vec3(
			scale_node.attribute("x").as_float(),
			scale_node.attribute("y").as_float(),
			scale_node.attribute("z").as_float()
		));
	}

	// Add children if it has a transformation previously
	for (Transformation* child : children)
	{
		trans.AddChild(child);
	}
	// Update parent and children transformations
	trans.PushToPositionArray();
}

void ComponentEngine::Transformation::AddChild(Transformation * trans)
{
	m_children.push_back(trans);
}

void ComponentEngine::Transformation::RemoveChild(Transformation * trans)
{
	auto it = std::find(m_children.begin(), m_children.end(), trans);
	if (it != m_children.end())
	{
		m_children.erase(it);
	}
}

void ComponentEngine::Transformation::PushToPositionArray()
{
	*m_mat4 = m_local_mat4;
	if (m_parent != nullptr && m_parent->HasComponent<Transformation>())
		(*m_mat4) = m_parent->GetComponent<Transformation>().Get() * m_local_mat4;

	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->PushToPositionArray();
	}
}

