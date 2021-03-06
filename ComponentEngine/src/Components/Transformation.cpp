#include <ComponentEngine\Components\Transformation.hpp>

#include <ComponentEngine\Components\Mesh.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <ComponentEngine/Engine.hpp>
#include <imgui.h>

using namespace ComponentEngine;

glm::mat4 & Transformation::Get()
{
	return *m_mat4;
}

void ComponentEngine::Transformation::Load(pugi::xml_node& node)
{
	// Loop through the 4 colums
	for (int row = 0; row < 4; row++)
	{
		std::stringstream rs;
		rs << "r" << row;
		// Get the row node
		pugi::xml_node r = node.child(rs.str().c_str());
		// Loop through appending the colum data into the row
		for (int colum = 0; colum < 4; colum++)
		{
			std::stringstream cs;
			cs << "c" << colum;
			pugi::xml_attribute rc1 = r.attribute(cs.str().c_str());
			m_local_mat4[row][colum] = rc1.as_float(0.0f);
		}
	}
}

void ComponentEngine::Transformation::Save(pugi::xml_node& node)
{
	// Loop through the 4 colums
	for(int row = 0 ; row < 4; row++)
	{
		std::stringstream rs;
		rs << "r" << row;
		// Create the row node
		pugi::xml_node r = node.append_child(rs.str().c_str());
		// Loop through appending the colum data into the row
		for (int colum = 0; colum < 4; colum++)
		{
			std::stringstream cs;
			cs << "c" << colum;
			pugi::xml_attribute rc1 = r.append_attribute(cs.str().c_str());
			rc1.set_value(m_local_mat4[row][colum]);
		}
	}
}


/*void ComponentEngine::Transformation::Load(std::ifstream& in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(Transformation, m_local_mat4), PayloadSize());
	PushToPositionArray();
}

void ComponentEngine::Transformation::Save(std::ofstream& out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(Transformation, m_local_mat4), PayloadSize());
}*/

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

void ComponentEngine::Transformation::SetWorld(float x, float y, float z)
{
	(m_local_mat4)[3][0] = x;
	(m_local_mat4)[3][1] = y;
	(m_local_mat4)[3][2] = z;
	PushToPositionArray();
}

void ComponentEngine::Transformation::MoveWorldX(float x)
{
	(m_local_mat4)[3][0] += x;
	PushToPositionArray(true);
}

void ComponentEngine::Transformation::MoveWorldY(float y)
{
	(m_local_mat4)[3][1] += y;
	PushToPositionArray(true);
}

void ComponentEngine::Transformation::MoveWorldZ(float z)
{
	(m_local_mat4)[3][2] += z;
	PushToPositionArray(true);
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

void ComponentEngine::Transformation::SetLocalMat4(glm::mat4 mat4, bool updatePhysics)
{
	m_local_mat4 = mat4;
	PushToPositionArray(updatePhysics);
}

void ComponentEngine::Transformation::SetWorldMat4(glm::mat4 mat4, bool updatePhysics)
{

	m_local_mat4 = mat4 / GetParentWorldMat4();

	*m_mat4 = mat4;
	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->PushToPositionArray();
	}
	if (updatePhysics)
		Send(m_entity, m_entity, TransformationChange{ *m_mat4 });

}

glm::mat4& ComponentEngine::Transformation::GetLocalMat4()
{
	return m_local_mat4;
}

glm::mat4 & ComponentEngine::Transformation::GetMat4()
{
	return *m_mat4;
}

glm::mat4 ComponentEngine::Transformation::GetParentWorldMat4()
{
	if (m_parent != nullptr && m_parent->HasComponent<Transformation>())
		return m_parent->GetComponent<Transformation>().GetMat4();
	return glm::mat4(1.0f);
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

ComponentEngine::Transformation::~Transformation()
{
	Send(m_entity, m_entity, OnComponentExit<Transformation>(this));
	if (m_parent != nullptr && Engine::Singlton()->GetEntityManager().ValidEntity(m_parent)) m_parent->GetComponent<Transformation>().RemoveChild(this);
	delete m_mat4;
}

void ComponentEngine::Transformation::Display()
{
	if (DisplayTransform(m_local_mat4))
	{
		PushToPositionArray();
	}
}

bool ComponentEngine::Transformation::DisplayTransform(glm::mat4 & mat4)
{
	bool changed = false;
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(mat4, scale, rotation, translation, skew, perspective);
	rotation = glm::conjugate(rotation);

	ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() - 10);
	{
		ImGui::PushID(0);
		ImGui::Text("Position");
		glm::vec3 change = translation;
		if (ImGui::InputFloat3("", (float*)&change, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			mat4 = glm::mat4(1.0f);
			mat4 = glm::translate(mat4, change);
			mat4 *= glm::inverse(glm::toMat4(rotation));
			mat4 = glm::scale(mat4, scale);
			changed = true;
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
			for (int i = 0; i < 3; i++)
			{
				if (glm::degrees(euler[i]) != change[i])
				{
					mat4 = glm::mat4(1.0f);
					mat4 = glm::translate(mat4, translation);

					glm::vec3 rotAxis(0, 0, 0);
					rotAxis[i] = 1.0f;

					glm::quat appliedRot = glm::angleAxis(glm::radians(change[i] - euler[i]), rotAxis);

					mat4 *= glm::inverse(glm::toMat4(rotation*appliedRot));
					mat4 = glm::scale(mat4, scale);
					changed = true;
				}
			}
		}
		ImGui::PopID();
	}
	{
		ImGui::PushID(2);
		ImGui::Text("Scale");
		glm::vec3 change = scale;
		if (ImGui::InputFloat3("", (float*)&change, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			mat4 = glm::mat4(1.0f);
			mat4 = glm::translate(mat4, translation);
			mat4 *= glm::inverse(glm::toMat4(rotation));
			mat4 = glm::scale(mat4, change);
			changed = true;
		}
		ImGui::PushItemWidth(-(ImGui::GetWindowContentRegionWidth() - ImGui::CalcItemWidth()));
		ImGui::PopID();
	}

	return changed;
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

enteez::BaseComponentWrapper* ComponentEngine::Transformation::EntityHookDefault(enteez::Entity& entity)
{
	std::vector<Transformation*> children;
	if (entity.HasComponent<Transformation>())
	{
		Transformation& trans = entity.GetComponent<Transformation>();
		children = trans.GetChildren();
	}

	enteez::ComponentWrapper<Transformation>* trans_wrapper = entity.AddComponent<Transformation>(&entity);
	trans_wrapper->SetName("Transformation");
	Transformation& trans = trans_wrapper->Get();



	// Add children if it has a transformation previously
	for (Transformation* child : children)
	{
		trans.AddChild(child);
	}
	// Update parent and children transformations
	trans.PushToPositionArray();
	return trans_wrapper;
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

void ComponentEngine::Transformation::PushToPositionArray(bool updatePhysics)
{
	glm::mat4* matrix = m_mat4;

	*matrix = m_local_mat4;
	if (m_parent != nullptr && m_parent->HasComponent<Transformation>())
		(*matrix) = m_parent->GetComponent<Transformation>().Get() * m_local_mat4;

	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->PushToPositionArray();
	}
	if (updatePhysics)
		Send(m_entity, m_entity, TransformationChange{ *matrix });
}