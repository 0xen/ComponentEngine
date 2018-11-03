#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine/pugixml.hpp>

#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>

#include <assert.h>
#include <sstream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

const unsigned int Engine::IS_RUNNING_LOCK = 0;
const unsigned int Engine::THREAD_TIME_LOCK = 1;
Engine* Engine::m_engine = nullptr;

ComponentEngine::Engine::Engine()
{
}

Engine* ComponentEngine::Engine::Singlton()
{
	if (m_engine == nullptr)
	{
		m_engine = new Engine();
	}
	return m_engine;
}

ComponentEngine::Engine::~Engine()
{
	// Stop threads
	m_logic_thread->join();
	DeInitEnteeZ();
	DeInitRenderer();
	DeInitWindow();
}

void ComponentEngine::Engine::Start(void(*logic_function)())
{
	m_title = "Component Engine";
	m_width = 1080;
	m_height = 720;
	m_api = RenderingAPI::VulkanAPI;
	InitWindow();
	InitRenderer();
	InitEnteeZ();
	InitComponentHooks();
	m_logic_thread = new ThreadHandler(logic_function);
}

bool ComponentEngine::Engine::Running()
{
	std::lock_guard<std::mutex> guard(m_locks[IS_RUNNING_LOCK]);
	return m_renderer != nullptr && m_renderer->IsRunning();
}

void ComponentEngine::Engine::Update()
{
	UpdateWindow();
}

void ComponentEngine::Engine::Rebuild()
{
	GetRendererMutex().lock();
	m_renderer->Rebuild();
	GetRendererMutex().unlock();
}

void ComponentEngine::Engine::RenderFrame()
{
	//Update camera
	m_camera_component.view = glm::inverse(m_camera_component.view);
	m_camera_component.view = m_camera_entity->GetComponent<Transformation>().Get();
	m_camera_component.view = glm::inverse(m_camera_component.view);
	m_camera_buffer->SetData();

	// Update all renderer's via there Update function
	IRenderer::UpdateAll();
}

bool ComponentEngine::Engine::LoadScene(const char * path, bool merge_scenes)
{

	pugi::xml_document xml;
	pugi::xml_parse_result result = xml.load_file(path);

	pugi::xml_node game_node = xml.child("Game");
	if (!game_node)return false;
	pugi::xml_node scenes_node = game_node.child("Scenes");
	if (!scenes_node)return false;
	pugi::xml_node scene_node = scenes_node.child("Scene");
	if (!scene_node)return false;


	for (pugi::xml_node node : scene_node.children("GameObject"))
	{
		LoadXMLGameObject(node);
	}


	return true;
}

IGraphicsPipeline * ComponentEngine::Engine::GetDefaultGraphicsPipeline()
{
	return m_default_pipeline;
}

IRenderer * ComponentEngine::Engine::GetRenderer()
{
	return m_renderer;
}

Entity * ComponentEngine::Engine::GetCameraEntity()
{
	return m_camera_entity;
}

Transformation * ComponentEngine::Engine::GetCameraTransformation()
{
	return &m_camera_entity->GetComponent<Transformation>();
}

IDescriptorPool * ComponentEngine::Engine::GetCameraPool()
{
	return m_camera_pool;
}

IDescriptorSet * ComponentEngine::Engine::GetCameraDescriptorSet()
{
	return m_camera_descriptor_set;
}

float ComponentEngine::Engine::GetFrameTime()
{
	return m_frame_time;
}

float ComponentEngine::Engine::GetThreadTime()
{

	/* = m_now_delta_time;
	m_now_delta_time = SDL_GetPerformanceCounter();
	m_frame_time = static_cast<float>((m_now_delta_time - m_delta_time) / (float)SDL_GetPerformanceFrequency());
*/


	std::thread::id id = std::this_thread::get_id();
	Uint64 now = SDL_GetPerformanceCounter();
	Uint64 last;
	{
		std::lock_guard<std::mutex> guard(m_locks[THREAD_TIME_LOCK]);
		if (m_thread_time.find(id) == m_thread_time.end())
		{
			m_thread_time[id] = now;
		}
	}
	last = m_thread_time[id];
	m_thread_time[id] = now;
	float temp = static_cast<float>((now - last) / (float)SDL_GetPerformanceFrequency());
	return temp;
}

float ComponentEngine::Engine::GetFPS()
{
	return m_fps;
}

ordered_lock& ComponentEngine::Engine::GetLogicMutex()
{
	return m_logic_thread->ThreadLock();
}

ordered_lock& ComponentEngine::Engine::GetRendererMutex()
{
	return m_renderer_thread;
}

void ComponentEngine::Engine::RegisterComponentInitilizer(const char * name, void(*fp)(enteez::Entity& entity, pugi::xml_node& component_data))
{
	m_component_initilizers[name] = fp;
}

Uint32 ComponentEngine::Engine::GetWindowFlags(RenderingAPI api)
{
	switch (api)
	{
	case VulkanAPI:
		return SDL_WINDOW_VULKAN;
		break;
	}
	return 0;
}

void ComponentEngine::Engine::InitWindow()
{
	m_window = SDL_CreateWindow(
		m_title,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		m_width, m_height,
		GetWindowFlags(m_api) | SDL_WINDOW_RESIZABLE
	);
	SDL_ShowWindow(m_window);

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	assert(SDL_GetWindowWMInfo(m_window, &info) && "Error, unable to get window info");

	m_window_handle = new NativeWindowHandle(info.info.win.window, m_width, m_height);
	m_window_handle->clear_color = { 0.2f,0.2f,0.2f,1.0f };
}

void ComponentEngine::Engine::UpdateWindow()
{

	m_frame_time = GetThreadTime();

	m_fps_update -= m_frame_time;
	
	m_delta_fps++;
	if (m_fps_update <= 0)
	{
		m_fps_update = 1.0f;
		m_fps = m_delta_fps;
		m_delta_fps = 0;
		std::stringstream ss;
		ss << m_title << " FPS:" << m_fps;
		SDL_SetWindowTitle(m_window, ss.str().c_str());
	}

	SDL_Event event;
	while (SDL_PollEvent(&event) > 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			if (Running())
				m_renderer->Stop();
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
				//Get new dimensions and repaint on window size change
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				GetRendererMutex().lock();
				m_window_handle->width = event.window.data1;
				m_window_handle->height = event.window.data2;
				Rebuild();
				GetRendererMutex().unlock();

				// Update Camera
				UpdateCameraProjection();
				m_camera_buffer->SetData();
				break;
			}
			break;
		}
	}
}

void ComponentEngine::Engine::DeInitWindow()
{
	delete m_window_handle;
}

void ComponentEngine::Engine::InitEnteeZ()
{
	// Define what base classes each one of these components have
	RegisterBase<RendererComponent, MsgSend>();
	RegisterBase<Mesh, MsgRecive<RenderStatus>>();
}

void ComponentEngine::Engine::DeInitEnteeZ()
{
	this->GetEntityManager().DestroyEntity(m_camera_entity);
}

void ComponentEngine::Engine::InitRenderer()
{
	// Create a instance of the renderer
	m_renderer = IRenderer::CreateRenderer(m_api);
	m_renderer->Start(m_window_handle);

	// If the rendering was not fully created, error out
	assert(m_renderer != nullptr && "Error, renderer instance could not be created");

	// Create camera
	m_camera_component.view = glm::mat4(1.0f);
	m_camera_component.view = glm::translate(m_camera_component.view, glm::vec3(0.0f, 0.0f, 0.0f));

	UpdateCameraProjection(); 


	m_camera_entity = this->GetEntityManager().CreateEntity();
	//m_camera_entity->AddComponent(&m_camera_component);
	Transformation* camera_transformation = m_camera_entity->AddComponent<Transformation>();
	camera_transformation->Translate(glm::vec3(0.0f, 0.0f, 0.0f));
	m_camera_entity->AddComponent(&m_camera_component);





	// Create camera buffer
	m_camera_buffer = m_renderer->CreateUniformBuffer(&m_camera_component, sizeof(Camera), 1);
	m_camera_buffer->SetData();

	// Create camera pool
	// This is a layout for the camera input data
	m_camera_pool = m_renderer->CreateDescriptorPool({
		m_renderer->CreateDescriptor(Renderer::DescriptorType::UNIFORM, Renderer::ShaderStage::VERTEX_SHADER, 0),
		});

	// Create camera descriptor set from the tempalte
	m_camera_descriptor_set = m_camera_pool->CreateDescriptorSet();
	// Attach the buffer
	m_camera_descriptor_set->AttachBuffer(0, m_camera_buffer);
	m_camera_descriptor_set->UpdateSet();


	// Create default pipeline
	m_default_pipeline = m_renderer->CreateGraphicsPipeline({
		{ ShaderStage::VERTEX_SHADER, "../../ComponentEngine-demo/Shaders/Default/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../../ComponentEngine-demo/Shaders/Default/frag.spv" }
		});

	// Tell the pipeline what data is should expect in the forum of Vertex input
	m_default_pipeline->AttachVertexBinding({
		VertexInputRate::INPUT_RATE_VERTEX,
		{
			{ 0, DataFormat::R32G32B32A32_FLOAT,offsetof(DefaultMeshVertex,position) },
			{ 1, DataFormat::R32G32B32A32_FLOAT,offsetof(DefaultMeshVertex,color) }
		},
		sizeof(DefaultMeshVertex),
		0
		});

	m_default_pipeline->AttachVertexBinding({
		VertexInputRate::INPUT_RATE_INSTANCE,
		{
			{ 2, DataFormat::MAT4_FLOAT,0 }
		},
		sizeof(glm::mat4),
		1
		});

	// Tell the pipeline what the input data will be payed out like
	m_default_pipeline->AttachDescriptorPool(m_camera_pool);
	// Attach the camera descriptor set to the pipeline
	m_default_pipeline->AttachDescriptorSet(0, m_camera_descriptor_set);

	// Build and check default pipeline
	assert(m_default_pipeline->Build() && "Unable to build default pipeline");

}

void ComponentEngine::Engine::DeInitRenderer()
{
	delete m_default_pipeline;
	delete m_camera_buffer;
	delete m_renderer;
}

void ComponentEngine::Engine::InitComponentHooks()
{
	RegisterComponentInitilizer("Transformation", Transformation::EntityHook);
	RegisterComponentInitilizer("Mesh", Mesh::EntityHook);
	RegisterComponentInitilizer("Renderer", RendererComponent::EntityHook);
}

void ComponentEngine::Engine::UpdateCameraProjection()
{
	float aspectRatio = ((float)m_window_handle->width) / ((float)m_window_handle->height);
	m_camera_component.projection = glm::perspective(
		glm::radians(45.0f),
		aspectRatio,
		0.1f,
		200.0f
	);
	// Need to flip the projection as GLM was made for OpenGL
	m_camera_component.projection[1][1] *= -1;



}

void ComponentEngine::Engine::LoadXMLGameObject(pugi::xml_node & xml_entity)
{
	enteez::Entity* entity = GetEntityManager().CreateEntity();
	for (pugi::xml_node node : xml_entity.children("Component"))
	{
		AttachXMLComponent(node, entity);
	}
}

void ComponentEngine::Engine::AttachXMLComponent(pugi::xml_node & xml_component, enteez::Entity * entity)
{
	std::string name = xml_component.attribute("name").as_string();
	auto& it = m_component_initilizers.find(name);
	if (it != m_component_initilizers.end())
	{
		m_component_initilizers[name](*entity, xml_component);
	}
	else
	{
		std::cout << "Warning! Could not find component (" << name.c_str() << ") Initializer!" << std::endl;
	}
}
