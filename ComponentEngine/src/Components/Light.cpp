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

	//m_light_pool = Engine::Singlton()->GetLightBufferPool();
	m_light_allocation = Engine::Singlton()->GetLightBufferPool()->Allocate();
}

ComponentEngine::Light::~Light()
{
	LightData* data = Engine::Singlton()->GetLightBufferPool()->Get<LightData>(m_light_allocation);
	data->alive = 0.0f;
	Engine::Singlton()->GetLightBufferPool()->UnAllocate(m_light_allocation);
}

void ComponentEngine::Light::Update(float frame_time)
{
	
	LightData* data = Engine::Singlton()->GetLightBufferPool()->Get<LightData>(m_light_allocation);
	data->position = m_entity->GetComponent<Transformation>().GetWorldPosition() + m_offset;
	data->intensity = m_intensity;
	data->color = m_color;
	data->alive = 1.0f;
}

void ComponentEngine::Light::EditorUpdate(float frame_time)
{
	Update(frame_time);
}

void ComponentEngine::Light::Display()
{
	ImGui::Text("Offset");
	ImGui::DragFloat3("##LightOffset", (float*)&m_offset, 0.1f);
	ImGui::Text("Intensity");
	ImGui::DragFloat("##LightIntensity", (float*)&m_intensity, 0.5f, 0.0f, 10000.0f);
	ImGui::Text("Color");
	ImGui::ColorEdit3("##LightColor", (float*)&m_color);
}

void ComponentEngine::Light::Load(std::ifstream & in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(Light, m_offset), PayloadSize());
}

void ComponentEngine::Light::Save(std::ofstream & out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(Light, m_offset), PayloadSize());
}

unsigned int ComponentEngine::Light::PayloadSize()
{
	return SizeOfOffsetRange(Light, m_offset, m_color);
}

bool ComponentEngine::Light::DynamiclySized()
{
	return false;
}

enteez::BaseComponentWrapper * ComponentEngine::Light::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<Light>* wrapper = entity.AddComponent<Light>(&entity);
	wrapper->SetName("Light");
	return wrapper;
}
