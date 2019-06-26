#include <ComponentEngine\Components\Camera.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Transformation.hpp>

#include <EnteeZ\EnteeZ.hpp>

std::vector<ComponentEngine::Camera*> ComponentEngine::Camera::m_global_cameras;

ComponentEngine::Camera::Camera()
{
	m_camera_data.view = glm::mat4(1.0f);
	m_camera_data.viewInverse = glm::inverse(m_camera_data.view);

	m_near_clip = 0.1f;
	m_far_clip = 200.0f;
	m_fov = 45.0f;


	m_camera_buffer = Engine::Singlton()->GetRenderer()->CreateUniformBuffer(&m_camera_data, BufferChain::Single, sizeof(Camera), 1,true);
	UpdateProjection();
	m_global_cameras.push_back(this);
}

ComponentEngine::Camera::Camera(enteez::Entity* entity)
{
	m_entity = entity;
	m_camera_data.view = glm::mat4(1.0f);
	m_camera_data.viewInverse = glm::inverse(m_camera_data.view);



	m_near_clip = 0.1f;
	m_far_clip = 200.0f;
	m_fov = 45.0f;


	m_camera_buffer = Engine::Singlton()->GetRenderer()->CreateUniformBuffer(&m_camera_data, BufferChain::Double, sizeof(Camera), 1,true);
	m_camera_buffer->SetData(BufferSlot::Primary);
	//UpdateProjection();

	SetMainCamera();
	m_global_cameras.push_back(this);
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

	m_camera_data.view = glm::inverse(m_camera_data.view);
	m_camera_data.view = m_entity->GetComponent<Transformation>().Get();
	m_camera_data.viewInverse = m_camera_data.view;
	m_camera_data.view = glm::inverse(m_camera_data.view);
	m_camera_buffer->SetData(BufferSlot::Secondery);

}

void ComponentEngine::Camera::BufferTransfer()
{
	m_camera_buffer->Transfer(BufferSlot::Primary, BufferSlot::Secondery);
}

void ComponentEngine::Camera::Display()
{
	bool cameraRunning = Engine::Singlton()->GetMainCamera() == this;
	ImGui::Text("Status: %s", cameraRunning ? "Active":"Inavtive");

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

IUniformBuffer* ComponentEngine::Camera::GetCameraBuffer()
{
	return m_camera_buffer;
}

void ComponentEngine::Camera::UpdateProjection()
{

	float aspectRatio = ((float)1080) / ((float)720);
	m_camera_data.proj = glm::perspective(
		glm::radians(m_fov),
		aspectRatio,
		m_near_clip,
		m_far_clip
	);
	m_camera_data.proj[1][1] *= -1;

	m_camera_data.projInverse = glm::inverse(m_camera_data.proj);

	// Need to flip the projection as GLM was made for OpenGL
	m_camera_data.proj[1][1] *= -1;
	{
		Engine::Singlton()->GetRendererMutex().lock();
		m_camera_buffer->SetData(BufferSlot::Primary);
		Engine::Singlton()->GetRendererMutex().unlock();
	}
}

ComponentEngine::Transformation* ComponentEngine::Camera::GetTransformation()
{
	assert(m_entity != nullptr && "Camera must be attached to scene!");
	return &m_entity->GetComponent<Transformation>();
}
