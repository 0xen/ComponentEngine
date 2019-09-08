#include <ComponentEngine\Components\Light.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Transformation.hpp>

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
	m_dir = glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f));

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
	data->alive = 1;
	data->lightType = m_type;
	data->dir = m_dir;
}

// Called during updates when we are not in the play state
void ComponentEngine::Light::EditorUpdate(float frame_time)
{
	Update(frame_time);
}

// Called when we are in a ImGui UI draw state and the components info needs to be rendered
void ComponentEngine::Light::Display()
{
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
	ImGui::DragFloat("##LightIntensity", (float*)&m_intensity, 0.5f, 0.0f, 10000.0f);
	ImGui::Text("Color");
	ImGui::ColorEdit3("##LightColor", (float*)&m_color, ImGuiColorEditFlags_InputRGB);

}

// Load the component from a file
void ComponentEngine::Light::Load(std::ifstream & in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(Light, m_offset), PayloadSize());
}

// Save the component to a file
void ComponentEngine::Light::Save(std::ofstream & out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(Light, m_offset), PayloadSize());
}

// How much data should we save to a file
unsigned int ComponentEngine::Light::PayloadSize()
{
	return SizeOfOffsetRange(Light, m_offset, m_type);
}

// Is the size we save to a file dynamic?
bool ComponentEngine::Light::DynamiclySized()
{
	return false;
}

// Define a static constructor for the component
enteez::BaseComponentWrapper * ComponentEngine::Light::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<Light>* wrapper = entity.AddComponent<Light>(&entity);
	wrapper->SetName("Light");
	return wrapper;
}
