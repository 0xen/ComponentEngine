#include <ComponentEngine\Engine.hpp>

#include <assert.h>
#include <sstream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

ComponentEngine::Engine::Engine()
{
	m_title = "Component Engine";
	m_width = 1080;
	m_height = 720;
	m_api = RenderingAPI::VulkanAPI;
	InitWindow();
	InitRenderer();
	InitEnteeZ();
}

ComponentEngine::Engine::~Engine()
{
	DeInitEnteeZ();
	DeInitRenderer();
	DeInitWindow();
}

bool ComponentEngine::Engine::Running()
{
	return m_renderer != nullptr && m_renderer->IsRunning();
}

void ComponentEngine::Engine::Update()
{
	UpdateWindow();
}

void ComponentEngine::Engine::Render()
{
	// Update all renderer's via there Update function
	IRenderer::UpdateAll();
}

IGraphicsPipeline * ComponentEngine::Engine::GetDefaultGraphicsPipeline()
{
	return m_default_pipeline;
}

IRenderer * ComponentEngine::Engine::GetRenderer()
{
	return m_renderer;
}

float ComponentEngine::Engine::GetFrameTime()
{
	return m_frame_time;
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

	m_delta_time = m_now_delta_time;
	m_now_delta_time = SDL_GetPerformanceCounter();
	m_frame_time = (float)(m_now_delta_time - m_delta_time) / SDL_GetPerformanceFrequency();




	/*m_frame_time = 1000.0f / (SDL_GetPerformanceCounter() - m_now_delta_time);
	m_delta_time -= m_frame_time;
	m_now_delta_time = SDL_GetPerformanceCounter();
	m_delta_fps++;
	if (m_delta_time <= 0)
	{
		m_fps = m_delta_fps;
		m_delta_fps = 0;
		m_delta_time = 1000.0f;
		std::stringstream ss;
		ss << m_title << " FPS:" << m_fps;
		SDL_SetWindowTitle(m_window, ss.str().c_str());
	}*/

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
				m_window_handle->width = event.window.data1;
				m_window_handle->height = event.window.data2;
				m_renderer->Rebuild();
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

}

void ComponentEngine::Engine::DeInitEnteeZ()
{
}

void ComponentEngine::Engine::InitRenderer()
{
	// Create a instance of the renderer
	m_renderer = IRenderer::CreateRenderer(m_api);
	m_renderer->Start(m_window_handle);

	// If the rendering was not fully created, error out
	assert(m_renderer != nullptr && "Error, renderer instance could not be created");




	// Create camera
	m_camera.view = glm::mat4(1.0f);
	m_camera.view = glm::scale(m_camera.view, glm::vec3(1.0f, 1.0f, 1.0f));
	m_camera.view = glm::translate(m_camera.view, glm::vec3(0.0f, 0.0f, 0.0f));

	float aspectRatio = ((float)m_window_handle->width) / ((float)m_window_handle->height);
	m_camera.projection = glm::perspective(
		glm::radians(45.0f),
		aspectRatio,
		0.1f,
		200.0f
	);

	// Create camera buffer
	m_camera_buffer = m_renderer->CreateUniformBuffer(&m_camera, sizeof(Camera), 1);
	m_camera_buffer->SetData();

	// Create camera pool
	// This is a layout for the camera input data
	m_camera_pool = m_renderer->CreateDescriptorPool({
		m_renderer->CreateDescriptor(Renderer::DescriptorType::UNIFORM, Renderer::ShaderStage::VERTEX_SHADER, 0),
		});

	// Create camera descriptor set from the tempalte
	IDescriptorSet* camera_descriptor_set = m_camera_pool->CreateDescriptorSet();
	// Attach the buffer
	camera_descriptor_set->AttachBuffer(0, m_camera_buffer);
	camera_descriptor_set->UpdateSet();


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
	m_default_pipeline->AttachDescriptorSet(0, camera_descriptor_set);

	// Build and check default pipeline
	assert(m_default_pipeline->Build() && "Unable to build default pipeline");

}

void ComponentEngine::Engine::DeInitRenderer()
{
	delete m_default_pipeline;
	delete m_camera_buffer;
	delete m_renderer;
}
