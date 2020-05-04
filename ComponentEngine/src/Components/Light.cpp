#include <ComponentEngine\Components\Light.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Transformation.hpp>
#include <ComponentEngine\Components\Mesh.hpp>

#include <renderer\vulkan\VulkanUniformBuffer.hpp>
#include <renderer\vulkan\VulkanBufferPool.hpp>
#include <renderer\vulkan\VulkanBuffer.hpp>

#include <EnteeZ\EnteeZ.hpp>

ComponentEngine::Light::Light(enteez::Entity * entity)
{
	m_entity = entity;
	m_offset = glm::vec3(0.0f);
	m_intensity = 100.0f;
	m_color = glm::vec3(1.0f);
	m_type = 0;
	m_alive = 1;
	m_dir = glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f));
	shadowRangeStartOffset = 0;
	shadowRangeEndOffset = 0;

	//m_light_pool = Engine::Singlton()->GetLightBufferPool();
	m_light_allocation = Engine::Singlton()->GetLightBufferPool()->Allocate();
}

ComponentEngine::Light::~Light()
{
	LightData* data = Engine::Singlton()->GetLightBufferPool()->Get<LightData>(m_light_allocation);
	data->alive = 0;
	Engine::Singlton()->GetLightBufferPool()->UnAllocate(m_light_allocation);
}

// Called during logic updates
void ComponentEngine::Light::Update(float frame_time)
{
	LightData* data = Engine::Singlton()->GetLightBufferPool()->Get<LightData>(m_light_allocation);
	data->position = m_entity->GetComponent<Transformation>().GetWorldPosition() + m_offset;
	data->intensity = m_intensity;
	data->color = m_color;
	data->alive = m_alive;
	data->lightType = m_type;
	data->dir = m_dir;
	data->modelID = -1;
	data->shadowRangeStartOffset = shadowRangeStartOffset;
	data->shadowRangeEndOffset = shadowRangeEndOffset;
	if (m_entity->HasComponent<Mesh>())
	{
		data->modelID = m_entity->GetComponent<Mesh>().GetUUID();
	}
}

// Called during updates when we are not in the play state
void ComponentEngine::Light::EditorUpdate(float frame_time)
{
	Update(frame_time);
}

// Called when we are in a ImGui UI draw state and the components info needs to be rendered
void ComponentEngine::Light::Display()
{
	ImGui::PushID(this);
	ImGui::Text("Light Type");
	ImGui::Combo("##LightType", &m_type, m_light_types, 2, 2);

	switch (m_type)
	{
	case 0: // Point Light
		ImGui::Text("Offset");
		ImGui::DragFloat3("##LightOffset", (float*)&m_offset, 0.1f);
		break;
	case 1: // Directional Light
		ImGui::Text("Direction");
		ImGui::DragFloat3("##Direction", (float*)&m_dir, 0.1f);
		break;
	}

	ImGui::Text("Intensity");
	if (ImGui::DragFloat("##LightIntensity", (float*)&m_intensity, 0.5f, 0.0f, 10000.0f))
	{
		Engine::Singlton()->ResetViewportBuffers();
	}
	ImGui::Text("ShadowRange Start Offset");
	if (ImGui::DragFloat("##ShadowRangeStart", (float*)&shadowRangeStartOffset, 0.1f, -1000.0f, 1000.0f))
	{
		Engine::Singlton()->ResetViewportBuffers();
	}
	ImGui::Text("ShadowRange End Offset");
	if (ImGui::DragFloat("##ShadowRangeEnd", (float*)&shadowRangeEndOffset, 0.1f, -1000.0f, 1000.0f))
	{
		Engine::Singlton()->ResetViewportBuffers();
	}
	ImGui::Text("Color");
	if (ImGui::ColorEdit3("##LightColor", (float*)&m_color, ImGuiColorEditFlags_InputRGB))
	{
		Engine::Singlton()->ResetViewportBuffers();
	}
	ImGui::PopID();
}

void ComponentEngine::Light::Load(pugi::xml_node& node)
{
	{
		pugi::xml_node offset = node.child("Offset");
		m_offset.x = offset.attribute("X").as_float(m_offset.x);
		m_offset.y = offset.attribute("Y").as_float(m_offset.y);
		m_offset.z = offset.attribute("Z").as_float(m_offset.z);
	}
	m_intensity = node.attribute("Intensity").as_float(m_intensity);
	shadowRangeStartOffset = node.attribute("ShadowRangeStartOffset").as_float(shadowRangeStartOffset);
	shadowRangeEndOffset = node.attribute("ShadowRangeEndOffset").as_float(shadowRangeEndOffset);
	{
		pugi::xml_node color = node.child("Color");
		m_color.x = color.attribute("R").as_float(m_color.x);
		m_color.y = color.attribute("G").as_float(m_color.y);
		m_color.z = color.attribute("B").as_float(m_color.z);
	}
	{
		pugi::xml_node direction = node.child("Direction");
		m_dir.x = direction.attribute("X").as_float(m_dir.x);
		m_dir.y = direction.attribute("Y").as_float(m_dir.y);
		m_dir.z = direction.attribute("Z").as_float(m_dir.z);
	}
	m_type = node.attribute("Type").as_int(m_type);
	m_alive = node.attribute("Alive").as_int(m_alive);
}

void ComponentEngine::Light::Save(pugi::xml_node& node)
{
	{
		pugi::xml_node offset = node.append_child("Offset");
		offset.append_attribute("X").set_value(m_offset.x);
		offset.append_attribute("Y").set_value(m_offset.y);
		offset.append_attribute("Z").set_value(m_offset.z);
	}
	node.append_attribute("Intensity").set_value(m_intensity);
	node.append_attribute("ShadowRangeStartOffset").set_value(shadowRangeStartOffset);
	node.append_attribute("ShadowRangeEndOffset").set_value(shadowRangeEndOffset);
	{
		pugi::xml_node color = node.append_child("Color");
		color.append_attribute("R").set_value(m_color.x);
		color.append_attribute("G").set_value(m_color.y);
		color.append_attribute("B").set_value(m_color.z);
	}
	{
		pugi::xml_node direction = node.append_child("Direction");
		direction.append_attribute("X").set_value(m_dir.x);
		direction.append_attribute("Y").set_value(m_dir.y);
		direction.append_attribute("Z").set_value(m_dir.z);
	}
	node.append_attribute("Type").set_value(m_type);
	node.append_attribute("Alive").set_value(m_alive);
}

// Define a static constructor for the component
enteez::BaseComponentWrapper * ComponentEngine::Light::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<Light>* wrapper = entity.AddComponent<Light>(&entity);
	wrapper->SetName("Light");
	return wrapper;
}
