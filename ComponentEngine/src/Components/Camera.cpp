#include <ComponentEngine\Components\Camera.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Transformation.hpp>
#include <ComponentEngine\Common.hpp>

#include <renderer\vulkan\VulkanUniformBuffer.hpp>

#include <EnteeZ\EnteeZ.hpp>

std::vector<ComponentEngine::Camera*> ComponentEngine::Camera::m_global_cameras;

ComponentEngine::Camera::Camera()
{
	m_camera_data.view = glm::mat4(1.0f);
	m_camera_data.viewInverse = m_camera_data.view;// glm::inverse(m_camera_data.view);

	m_requested_reset_viewport_buffer = false;
	m_near_clip = 0.1f;
	m_far_clip = 200.0f;
	m_fov = 45.0f;

	m_camera_data.maxRecursionDepthColor = glm::vec3(0.7f, 0.7f, 0.7f);
	m_camera_data.samplesPerFrame = 2;
	m_camera_data.mode = 0;
	m_camera_data.dofSampleCount = 10;
	m_camera_data.globalIlluminationBrightness = 0.8f;
	m_camera_data.globalIlluminationReflectionMissBrightness = 0.75f;
	m_camera_data.gpuRecursionCount = Engine::Singlton()->GetRaytracerRecursionDepth();
	m_camera_data.recursionCount = 50;
	m_camera_data.aperture = 0.002f;
	m_camera_data.focusDistance = 0.5f;
	m_camera_data.movmentTollarance = 0.0001f;

	m_camera_buffer = Engine::Singlton()->GetRenderer()->CreateUniformBuffer(&m_camera_data, BufferChain::Double, sizeof(Camera), 1,true);
	UpdateProjection();
	m_global_cameras.push_back(this);
	m_camera_buffer->Transfer(BufferSlot::Primary, BufferSlot::Secondery);
}

ComponentEngine::Camera::Camera(enteez::Entity* entity)
{
	m_entity = entity;
	m_camera_data.view = glm::mat4(1.0f);
	m_camera_data.viewInverse = m_camera_data.view;// glm::inverse(m_camera_data.view);


	m_requested_reset_viewport_buffer = false;
	m_near_clip = 0.1f;
	m_far_clip = 200.0f;
	m_fov = 45.0f;

	m_camera_data.maxRecursionDepthColor = glm::vec3(0.7f, 0.7f, 0.7f);
	m_camera_data.samplesPerFrame = 2;
	m_camera_data.mode = 0;
	m_camera_data.dofSampleCount = 10;
	m_camera_data.globalIlluminationBrightness = 0.8f;
	m_camera_data.globalIlluminationReflectionMissBrightness = 0.75f;
	m_camera_data.gpuRecursionCount = Engine::Singlton()->GetRaytracerRecursionDepth();
	m_camera_data.recursionCount = 50;
	m_camera_data.aperture = 0.002f;
	m_camera_data.focusDistance = 0.5f;
	m_camera_data.movmentTollarance = 0.0001f;

	m_camera_buffer = Engine::Singlton()->GetRenderer()->CreateUniformBuffer(&m_camera_data, BufferChain::Double, sizeof(Camera), 1,true);
	UpdateProjection();

	SetMainCamera();
	m_global_cameras.push_back(this);
	m_camera_buffer->Transfer(BufferSlot::Primary, BufferSlot::Secondery);
}

ComponentEngine::Camera::~Camera()
{
	Engine::Singlton()->GetRendererMutex().lock();
	delete m_camera_buffer;

	{
		auto it = std::find(m_global_cameras.begin(), m_global_cameras.end(), this);
		if (it != m_global_cameras.end())
		{
			m_global_cameras.erase(it);
		}
	}

	{
		if (m_global_cameras.size() > 0 && Engine::Singlton()->GetMainCamera() == this)
		{
			Engine::Singlton()->SetCamera(m_global_cameras[m_global_cameras.size() - 1]);
		}
	}

	Engine::Singlton()->GetRendererMutex().unlock();
}

void ComponentEngine::Camera::Update(float frame_time)
{

}

void ComponentEngine::Camera::SetBufferData()
{
	m_camera_data.view = m_entity->GetComponent<Transformation>().Get();
	m_camera_data.viewInverse = m_camera_data.view;
	//m_camera_data.viewInverse = glm::inverse(m_camera_data.view);
	m_camera_buffer->SetData(BufferSlot::Secondery);


	std::unique_lock<std::mutex> lock(m_reset_viewport_buffers);
	if (m_requested_reset_viewport_buffer)
	{
		m_requested_reset_viewport_buffer = false;
		Engine::Singlton()->ResetViewportBuffers();
	}
}

void ComponentEngine::Camera::BufferTransfer()
{
	m_camera_buffer->Transfer(BufferSlot::Primary, BufferSlot::Secondery);
}

void ComponentEngine::Camera::Display()
{
	bool cameraRunning = Engine::Singlton()->GetMainCamera() == this;
	ImGui::Text("Status: %s", cameraRunning ? "Active":"Inavtive");

	Engine::Singlton()->GetLogicMutex().lock();

	if (!cameraRunning)
	{
		if (ImGui::Button("Activate Camera"))
		{
			this->SetMainCamera();
		}
	}

	ImGui::Text("Near Clip");
	if (ImGui::DragFloat("##cameraNearClip", &m_near_clip, 0.05f, 0.1f, 200.0f))
	{
		UpdateProjection();
	}

	ImGui::Text("Far Clip");
	if (ImGui::DragFloat("##cameraFarClip", &m_far_clip, 0.05f, 0.1f, 200.0f))
	{
		UpdateProjection();
	}

	ImGui::Text("FOV");
	if (ImGui::DragFloat("##cameraFOV", &m_fov, 0.05f, 1.0f, 100.0f))
	{
		UpdateProjection();
	}

	ImGui::Text("Global Illumination Brightness");
	if (ImGui::DragFloat("##cameraglobalIlluminationBrightness", &m_camera_data.globalIlluminationBrightness, 0.05f, 0.0f, 1.0f))
	{
		UpdateProjection();
	}

	ImGui::Text("Global Illumination Brightness Miss");
	if (ImGui::DragFloat("##cameraglobalIlluminationBrightnessMiss", &m_camera_data.globalIlluminationReflectionMissBrightness, 0.05f, 0.0f, 1.0f))
	{
		UpdateProjection();
	}

	DisplayRaytraceConfig();

	Engine::Singlton()->GetLogicMutex().unlock();
}

void ComponentEngine::Camera::DisplayRaytraceConfig()
{

	Engine::Singlton()->GetLogicMutex().lock();


	int mode = m_camera_data.mode;
	const char* modes[] = { "Single Pass","DOF" };
	ImGui::Text("Mode");
	if (ImGui::Combo("##Mode", &mode, modes, 2))
	{
		m_camera_data.mode = mode;
		SendDataToGPU();
	}

	ImGui::Text("Sample Per Frame");
	int sampleCount = m_camera_data.samplesPerFrame;
	if (ImGui::DragInt("##samplePerFrame", &sampleCount, 0.05f, 1, 50))
	{
		m_camera_data.samplesPerFrame = sampleCount;
		SendDataToGPU();
	}

	ImGui::Text("DOF Total Samples");
	int totalSamples = m_camera_data.dofSampleCount;
	if (ImGui::DragInt("##totalSamples", &totalSamples, 0.05f, 1, 250))
	{
		m_camera_data.dofSampleCount = totalSamples;
		SendDataToGPU();
	}

	ImGui::Text("Transparency Recursion Count");
	int RecursionCount = m_camera_data.recursionCount;
	if (ImGui::DragInt("##cameraRecursionCount", &RecursionCount, 0.1f, 1, 100))
	{
		m_camera_data.recursionCount = RecursionCount;
		SendDataToGPU();
	}

	ImGui::Text("Aperture");
	if (ImGui::DragFloat("##cameraAperture", &m_camera_data.aperture, 0.001f, 0.001f, 1.0f, "%.5f"))
	{
		SendDataToGPU();
	}

	ImGui::Text("Focus Distance");
	if (ImGui::DragFloat("##cameraFocusDistance", &m_camera_data.focusDistance, 0.005f, 0.001f, 100.0f, "%.5f"))
	{
		SendDataToGPU();
	}

	ImGui::Text("Movement Tolerance");
	if (ImGui::DragFloat("##cameraMovmentTolerance", &m_camera_data.movmentTollarance, 0.0001f, 0.001f, 2.0f, "%.5f"))
	{
		SendDataToGPU();
	}

	ImGui::Text("Max Recursion Depth Color");
	if (ImGui::ColorEdit3("##cameraMaxRecursionDepthColor", (float*)&m_camera_data.maxRecursionDepthColor))
	{
		SendDataToGPU();
	}
	Engine::Singlton()->GetLogicMutex().unlock();
}

void ComponentEngine::Camera::Load(std::ifstream & in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(Camera, m_camera_data.recursionCount), PayloadSize());
}

void ComponentEngine::Camera::Save(std::ofstream & out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(Camera, m_camera_data.recursionCount), PayloadSize());
}

unsigned int ComponentEngine::Camera::PayloadSize()
{
	return SizeOfOffsetRange(Camera, m_camera_data.recursionCount, m_camera_data.focusDistance);
}

bool ComponentEngine::Camera::DynamiclySized()
{
	return false;
}

enteez::BaseComponentWrapper* ComponentEngine::Camera::EntityHookDefault(enteez::Entity& entity)
{
	enteez::ComponentWrapper<Camera>* mesh = entity.AddComponent<Camera>(&entity);
	mesh->SetName("Camera");
	return mesh;
}

void ComponentEngine::Camera::SetMainCamera()
{
	Engine::Singlton()->SetCamera(this);
}

VulkanUniformBuffer* ComponentEngine::Camera::GetCameraBuffer()
{
	return m_camera_buffer;
}

void ComponentEngine::Camera::UpdateProjection()
{
	Engine::Singlton()->GetLogicMutex().lock();

	float aspectRatio = ((float)1080) / ((float)720);
	m_camera_data.proj = glm::perspective(
		glm::radians(m_fov),
		aspectRatio,
		m_near_clip,
		m_far_clip
	);
	m_camera_data.proj[1][1] *= -1;
	m_camera_data.projInverse = glm::inverse(m_camera_data.proj);

	//m_camera_data.projInverse = glm::inverse(m_camera_data.proj);

	// Need to flip the projection as GLM was made for OpenGL
	//m_camera_data.proj[1][1] *= -1;*/


	//m_camera_data.projInverse = m_camera_data.proj;
	//m_camera_data.projInverse[1][1] *= -1;
	//m_camera_data.projInverse = glm::inverse(m_camera_data.projInverse);

	SendDataToGPU();
	Engine::Singlton()->GetLogicMutex().unlock();
}

ComponentEngine::Transformation* ComponentEngine::Camera::GetTransformation()
{
	assert(m_entity != nullptr && "Camera must be attached to scene!");
	return &m_entity->GetComponent<Transformation>();
}

float ComponentEngine::Camera::GetFOV()
{
	return m_fov;
}

void ComponentEngine::Camera::SetFOV(float f)
{
	m_fov = f;
}

float ComponentEngine::Camera::GetAperture()
{
	return m_camera_data.aperture;
}

void ComponentEngine::Camera::SetAperture(float a)
{
	m_camera_data.aperture = a;
}

float ComponentEngine::Camera::GetFocusDistance()
{
	return m_camera_data.focusDistance;
}

void ComponentEngine::Camera::SetFocusDistance(float f)
{
	m_camera_data.focusDistance = f;
}

void ComponentEngine::Camera::SendDataToGPU()
{
	//Engine::Singlton()->GetRendererMutex().lock();
	m_camera_buffer->SetData(BufferSlot::Secondery);

	std::unique_lock<std::mutex> lock(m_reset_viewport_buffers);
	m_requested_reset_viewport_buffer = true;
	//Engine::Singlton()->GetRendererMutex().unlock();
}
