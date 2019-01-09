#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine/pugixml.hpp>

#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <ComponentEngine\UIManager.hpp>

#include <lodepng.h>

#include <assert.h>
#include <sstream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

const unsigned int Engine::IS_RUNNING_LOCK = 0;
const unsigned int Engine::TOGGLE_FRAME_LIMITING = 1;

Engine* Engine::m_engine = nullptr;

bool ComponentEngine::Engine::IsRunning()
{
	std::lock_guard<std::mutex> guard(m_locks[IS_RUNNING_LOCK]);
	return m_running;
}

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
	Stop();
	for (auto i : m_thread_data)
	{
		delete i->thread_instance;
	}
}

void ComponentEngine::Engine::Start()
{
	m_title = "Component Engine";
	m_width = 1080;
	m_height = 720;
	m_api = RenderingAPI::VulkanAPI;
	InitWindow();
	InitEnteeZ();
	InitRenderer();
	InitComponentHooks();
	m_main_thread = std::this_thread::get_id();
	m_running = true;

	ThreadData* data = new ThreadData();
	data->data_lock.lock();
	data->thread_instance = nullptr;
	data->name = "Renderer";
	data->delta_time = SDL_GetPerformanceCounter();
	data->frame_limited = true;
	m_thread_data.push_back(data);
	m_thread_linker[m_main_thread] = data;
	data->data_lock.unlock();
}

void ComponentEngine::Engine::AddThread(ThreadHandler* handler, const char* name)
{
	if (m_threading)
	{

		ThreadData* data = new ThreadData();
		data->data_lock.lock();
		handler->StartThread();
		data->thread_instance = handler;
		data->name = name;
		data->delta_time = SDL_GetPerformanceCounter();
		data->frame_limited = true;
		std::thread::id id = data->thread_instance->GetID();
		m_thread_linker[id] = data;
		m_thread_data.push_back(data);
		data->data_lock.unlock();
	}

}

void ComponentEngine::Engine::Stop()
{
	if (!IsRunning())
	{
		return;
	}
	{
		std::lock_guard<std::mutex> guard(m_locks[IS_RUNNING_LOCK]);
		m_running = false;
	}
	GetRendererMutex().lock();
	GetEntityManager().Clear();
	DeInitEnteeZ();
	DeInitRenderer();
	DeInitWindow();
	GetRendererMutex().unlock();
}

void ComponentEngine::Engine::Join()
{
	for (auto i : m_thread_data)
	{
		if (i->thread_instance != nullptr)
		{
			i->thread_instance->Join();
		}
	}
}

bool ComponentEngine::Engine::Running()
{
	NewThreadUpdatePass();
	if (m_main_thread == std::this_thread::get_id())
	{
		if (m_request_stop)
		{
			m_request_stop = false;
			Stop();
		}
		else if (m_request_toggle_threading)
		{
			m_request_toggle_threading = false;
			ToggleThreading();
		}
		std::lock_guard<std::mutex> guard(m_locks[IS_RUNNING_LOCK]);
		bool result = m_renderer != nullptr && m_renderer->IsRunning();

		//GetRendererMutex().lock();
		//GetRendererMutex().unlock();
		return result;
	}
	else
	{
		std::lock_guard<std::mutex> guard(m_locks[IS_RUNNING_LOCK]);
		return m_running;
	}
}

bool ComponentEngine::Engine::Running(int ups)
{
	Sync(ups);
	return Running();
}

void ComponentEngine::Engine::Update()
{
	GetRendererMutex().lock();

	if (m_main_thread == std::this_thread::get_id())
	{
		for (auto i : m_thread_data)
		{
			if (i->thread_instance != nullptr)
			{
				if (i->thread_instance->Joined())
				{
					i->thread_instance->Loop();
				}
			}
		}
	}

	UpdateWindow();
	GetRendererMutex().unlock();
}

void ComponentEngine::Engine::UpdateScene()
{
	Mesh::SetBufferData();
	GetRendererMutex().lock();
	Mesh::TransferToPrimaryBuffers();
	GetRendererMutex().unlock();
}

void ComponentEngine::Engine::UpdateUI()
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = GetLastThreadTime();
	UpdateImGUI();
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

	GetRendererMutex().lock();
	// Set data
	m_camera_buffer->SetData(BufferSlot::Primary);
	// Update all renderer's via there Update function
	IRenderer::UpdateAll();
	GetRendererMutex().unlock();
}


float ComponentEngine::Engine::Sync(int ups)
{
	std::thread::id id = std::this_thread::get_id();
	auto it = m_thread_linker.find(id);
	if (it == m_thread_linker.end())return 0.0f;
	ThreadData*& data = m_thread_linker[id];
	data->data_lock.lock();
	data->requested_ups = ups;

	
		//std::lock_guard<std::mutex> guard(m_locks[TOGGLE_FRAME_LIMITING]);
		if (!data->frame_limited)
		{
			data->data_lock.unlock();
			return data->loop_time;
		}
	

	float stop_time = GetThreadDeltaTime();
	data->delta_process_time += stop_time;
	data->delta_loop_time += stop_time;

	int pause_time = (int)(1000 / ups);
	pause_time -= (int)(data->process_time * 1000.0f);
	data->data_lock.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds(pause_time));
	data->data_lock.lock();
	float start_time = GetThreadDeltaTime();
	data->delta_loop_time += start_time;

	float loop_time = data->loop_time;
	data->data_lock.unlock();
	return loop_time;
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

IDescriptorPool * ComponentEngine::Engine::GetTextureMapsPool()
{
	return m_texture_maps_pool;
}

float ComponentEngine::Engine::GetThreadDeltaTime()
{
	std::thread::id id = std::this_thread::get_id();
	Uint64 now = SDL_GetPerformanceCounter();
	auto it = m_thread_linker.find(id);
	if (it == m_thread_linker.end())return 0.0f;
	ThreadData*& data = m_thread_linker[id];
	data->data_lock.lock();
	Uint64 last = data->delta_time;
	data->delta_time = now;
	float temp = static_cast<float>((float)(now - last) / SDL_GetPerformanceFrequency());
	data->data_lock.unlock();
	return temp;
}

float ComponentEngine::Engine::GetLastThreadTime()
{
	std::thread::id id = std::this_thread::get_id();
	auto it = m_thread_linker.find(id);
	if (it == m_thread_linker.end())return 0.0f;
	ThreadData*& data = m_thread_linker[id];
	data->data_lock.lock();
	float loop_time = data->loop_time;
	data->data_lock.unlock();
	return loop_time;
}

ITextureBuffer * ComponentEngine::Engine::GetTexture(std::string path)
{
	if (m_texture_storage.find(path) == m_texture_storage.end())
	{
		std::vector<unsigned char> image; //the raw pixels
		unsigned width;
		unsigned height;
		unsigned error = lodepng::decode(image, width, height, path);
		if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

		ITextureBuffer* texture = m_renderer->CreateTextureBuffer(image.data(), Renderer::DataFormat::R8G8B8A8_FLOAT, width, height);
		texture->SetData(BufferSlot::Primary);
		m_texture_storage[path] = texture;
	}
	return m_texture_storage[path];
}
/*
ordered_lock& ComponentEngine::Engine::GetLogicMutex()
{
	return m_logic_thread->ThreadLock();
}*/

ordered_lock& ComponentEngine::Engine::GetRendererMutex()
{
	return m_renderer_thread;
}

void ComponentEngine::Engine::RegisterComponentBase(std::string name, void(*default_initilizer)(enteez::Entity &entity), void(*xml_initilizer)(enteez::Entity &entity, pugi::xml_node &component_data))
{
	m_component_register[name].default_initilizer = default_initilizer;
	m_component_register[name].xml_initilizer = xml_initilizer;
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
	//SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_ShowWindow(m_window);

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	bool sucsess = SDL_GetWindowWMInfo(m_window, &info);
	assert(sucsess && "Error, unable to get window info");

	m_window_handle = new NativeWindowHandle(info.info.win.window, m_width, m_height);
	m_window_handle->clear_color = { 0.2f,0.2f,0.2f,1.0f };
}

void ComponentEngine::Engine::UpdateWindow()
{

	ImGuiIO& io = ImGui::GetIO();
	SDL_Event event;
	while (SDL_PollEvent(&event) > 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			if (Running())
			{
				Stop();
				return;
			}
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
				//Get new dimensions and repaint on window size change
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				GetRendererMutex().lock();
				m_window_handle->width = event.window.data1;
				m_window_handle->height = event.window.data2;
				m_imgui.m_screen_dim = glm::vec2(event.window.data1, event.window.data2);
				io.DisplaySize = ImVec2(event.window.data1, event.window.data2);
				m_imgui.m_screen_res_buffer->SetData(BufferSlot::Primary);
				Rebuild();
				GetRendererMutex().unlock();
				// Update Camera
				UpdateCameraProjection();
				m_camera_buffer->SetData(BufferSlot::Primary);
				break;
			}
			break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT) io.MouseDown[0] = event.button.state == SDL_PRESSED;
				if (event.button.button == SDL_BUTTON_RIGHT) io.MouseDown[1] = event.button.state == SDL_PRESSED;
				if (event.button.button == SDL_BUTTON_MIDDLE) io.MouseDown[2] = event.button.state == SDL_PRESSED;
				//io.MouseDown[0] = true;
				break;
			case SDL_MOUSEMOTION:
				io.MousePos = ImVec2(event.motion.x, event.motion.y);
				break;
			case SDL_TEXTINPUT:
			{
				io.AddInputCharactersUTF8(event.text.text);
				break;
			}
			case SDL_MOUSEWHEEL:
			{
				if (event.wheel.x > 0) io.MouseWheelH += 1;
				if (event.wheel.x < 0) io.MouseWheelH -= 1;
				if (event.wheel.y > 0) io.MouseWheel += 1;
				if (event.wheel.y < 0) io.MouseWheel -= 1;
				break;
			}
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				int key = event.key.keysym.scancode;
				IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
				io.KeysDown[key] = (event.type == SDL_KEYDOWN);
				io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
				io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
				io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
				io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
			}
			break;


		}
	}
}

void ComponentEngine::Engine::DeInitWindow()
{
	SDL_RestoreWindow(m_window);
	SDL_DestroyWindow(m_window);
	//SDL_ShowWindow
	delete m_window_handle;
}

void ComponentEngine::Engine::InitEnteeZ()
{
	// Define what base classes each one of these components have
	RegisterBase<Transformation, MsgRecive<TransformationPtrRedirect>, UI>();
	RegisterBase<RendererComponent, UI>();
	RegisterBase<Mesh, MsgRecive<RenderStatus>, UI, MsgRecive<OnComponentEnter<Transformation>>, MsgRecive<OnComponentExit<Transformation>>>();
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
	m_camera_component.view = glm::mat4(1.0f);
	m_camera_component.view = glm::translate(m_camera_component.view, glm::vec3(0.0f, 0.0f, 0.0f));

	UpdateCameraProjection(); 


	m_camera_entity = this->GetEntityManager().CreateEntity("Camera");
	//m_camera_entity->AddComponent(&m_camera_component);
	ComponentWrapper<Indestructable>* indestructable_transformation = m_camera_entity->AddComponent<Indestructable>();
	indestructable_transformation->SetName("Indestructable");
	ComponentWrapper<Transformation>* camera_transformation = m_camera_entity->AddComponent<Transformation>(m_camera_entity);
	camera_transformation->SetName("Transformation");
	camera_transformation->Get().Translate(glm::vec3(0.0f, 0.0f, 0.0f));
	ComponentWrapper<Camera>* camera_component = m_camera_entity->AddComponent(&m_camera_component);
	camera_component->SetName("Camera");

	// Create camera buffer
	m_camera_buffer = m_renderer->CreateUniformBuffer(&m_camera_component, BufferChain::Single, sizeof(Camera), 1);
	m_camera_buffer->SetData(BufferSlot::Primary);

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
		{ ShaderStage::VERTEX_SHADER, "../../ComponentEngine-demo/Shaders/Textured/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../../ComponentEngine-demo/Shaders/Textured/frag.spv" }
		});

	// Tell the pipeline what data is should expect in the forum of Vertex input
	m_default_pipeline->AttachVertexBinding({
		VertexInputRate::INPUT_RATE_VERTEX,
		{
			{ 0, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,position) },
		{ 1, DataFormat::R32G32_FLOAT,offsetof(MeshVertex,uv) },
		{ 2, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,normal) },
		{ 3, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,color) },
		},
		sizeof(MeshVertex),
		0
		});

	m_default_pipeline->AttachVertexBinding({
		VertexInputRate::INPUT_RATE_INSTANCE, // Input Rate
		{
			{	// Vertex Bindings
				4, // Location
				DataFormat::MAT4_FLOAT, // Format
				0 // Offset from start of data structure
			}
		},
		sizeof(glm::mat4), // Total size
		1 // Binding
		});

	// Tell the pipeline what the input data will be payed out like
	m_default_pipeline->AttachDescriptorPool(m_camera_pool);
	// Attach the camera descriptor set to the pipeline
	m_default_pipeline->AttachDescriptorSet(0, m_camera_descriptor_set);

	m_texture_maps_pool = Engine::Singlton()->GetRenderer()->CreateDescriptorPool({
		Engine::Singlton()->GetRenderer()->CreateDescriptor(Renderer::DescriptorType::IMAGE_SAMPLER, Renderer::ShaderStage::FRAGMENT_SHADER, 0),
		});

	m_default_pipeline->AttachDescriptorPool(m_texture_maps_pool);

	m_default_pipeline->UseCulling(true);

	bool sucsess = m_default_pipeline->Build();
	// Build and check default pipeline
	assert(sucsess && "Unable to build default pipeline");
	InitImGUI();
}

void ComponentEngine::Engine::DeInitRenderer()
{
	DeInitImGUI();

	Mesh::CleanUp();

	for (auto it = m_texture_storage.begin(); it != m_texture_storage.end(); it++)
	{
		delete it->second;
	}
	m_texture_storage.clear();


	delete m_default_pipeline;
	m_default_pipeline = nullptr;
	delete m_camera_buffer;
	m_camera_buffer = nullptr;
	delete m_camera_pool;
	m_camera_pool = nullptr;
	delete m_texture_maps_pool;
	m_texture_maps_pool = nullptr;
	m_renderer->Stop();
	delete m_renderer;
	m_renderer = nullptr;
}

void ComponentEngine::Engine::InitComponentHooks()
{
	
	RegisterComponentBase("Transformation", Transformation::EntityHookDefault, Transformation::EntityHookXML);
	RegisterComponentBase("Mesh",nullptr, Mesh::EntityHook);
	RegisterComponentBase("Renderer", RendererComponent::EntityHookDefault, RendererComponent::EntityHookXML);
	

	RegisterComponentBase("Indestructable", nullptr, nullptr);
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

void ComponentEngine::Engine::LoadXMLGameObject(pugi::xml_node & xml_entity, Entity* parent)
{
	std::string name = xml_entity.attribute("name").as_string();
	enteez::Entity* entity = GetEntityManager().CreateEntity(name.size() > 0 ? name : "Object");

	// For now manualy add transformation by default
	Transformation::EntityHookDefault(*entity);


	for (pugi::xml_node node : xml_entity.children("Component"))
	{
		AttachXMLComponent(node, entity);
	}
	for (pugi::xml_node node : xml_entity.children("GameObject"))
	{
		LoadXMLGameObject(node, entity);
	}
	// If this object has a parent, add it to the object
	if (parent != nullptr)
	{
		entity->GetComponent<Transformation>().SetParent(parent);
	}
}

void ComponentEngine::Engine::AttachXMLComponent(pugi::xml_node & xml_component, enteez::Entity * entity)
{
	std::string name = xml_component.attribute("name").as_string();
	auto& it = m_component_register.find(name);
	if (it != m_component_register.end())
	{
		m_component_register[name].xml_initilizer(*entity, xml_component);
	}
	else
	{
		std::cout << "Warning! Could not find component (" << name.c_str() << ") Initializer!" << std::endl;
	}
}

void ComponentEngine::Engine::InitImGUI()
{

	m_ui = new UIManager(this);

	// Init ImGUI
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(m_window_handle->width, m_window_handle->height);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);


	ImGuiStyle& style = ImGui::GetStyle();

	// light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
	style.Alpha = 1.0f;
	style.FrameRounding = 3.0f;
	style.WindowBorderSize = 0.0f;

	style.WindowRounding = 3.0f;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);


	io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
	io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
	io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
	io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
	io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
	io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
	io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
	io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
	io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
	io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
	io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
	io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
	io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

	// Init Screen Dim
	m_imgui.m_screen_dim = glm::vec2(1080, 720);
	m_imgui.m_screen_res_buffer = m_renderer->CreateUniformBuffer(&m_imgui.m_screen_dim, BufferChain::Single, sizeof(glm::vec2), 1, true);
	m_imgui.m_screen_res_buffer->SetData(BufferSlot::Primary);
	m_imgui.m_screen_res_pool = m_renderer->CreateDescriptorPool({
		m_renderer->CreateDescriptor(Renderer::DescriptorType::UNIFORM, Renderer::ShaderStage::VERTEX_SHADER, 0),
		});
	m_imgui.m_screen_res_set = m_imgui.m_screen_res_pool->CreateDescriptorSet();
	m_imgui.m_screen_res_set->AttachBuffer(0, m_imgui.m_screen_res_buffer);
	m_imgui.m_screen_res_set->UpdateSet();

	// Create font texture
	unsigned char* font_data;
	int font_width, font_height;
	io.Fonts->GetTexDataAsRGBA32(&font_data, &font_width, &font_height);
	m_imgui.m_font_texture = m_renderer->CreateTextureBuffer(font_data, Renderer::DataFormat::R8G8B8A8_FLOAT, font_width, font_height);

	io.Fonts->TexID = (ImTextureID)m_imgui.m_font_texture->GetTextureID();

	m_imgui.m_font_texture_pool = m_renderer->CreateDescriptorPool({
		m_renderer->CreateDescriptor(Renderer::DescriptorType::IMAGE_SAMPLER, Renderer::ShaderStage::FRAGMENT_SHADER, 0),
		});
	m_imgui.m_texture_descriptor_set = m_imgui.m_font_texture_pool->CreateDescriptorSet();
	m_imgui.m_texture_descriptor_set->AttachBuffer(0, m_imgui.m_font_texture);
	m_imgui.m_texture_descriptor_set->UpdateSet();

	// Setup ImGUI Pipeline
	m_imgui.m_imgui_pipeline = m_renderer->CreateGraphicsPipeline({
		{ ShaderStage::VERTEX_SHADER, "../../ImGUI/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../../ImGUI/frag.spv" }
		});
	m_imgui.m_imgui_pipeline->UseDepth(false);

	m_imgui.m_imgui_pipeline->AttachVertexBinding({
		VertexInputRate::INPUT_RATE_VERTEX,
		{
			{ 0, DataFormat::R32G32_FLOAT,offsetof(ImDrawVert,pos) },
			{ 1, DataFormat::R32G32_FLOAT,offsetof(ImDrawVert,uv) },
			{ 2, DataFormat::R8G8B8A8_UNORM,offsetof(ImDrawVert,col) },
		},
		sizeof(ImDrawVert),
		0
		});

	// Attach screen buffer
	m_imgui.m_imgui_pipeline->AttachDescriptorPool(m_imgui.m_screen_res_pool);
	m_imgui.m_imgui_pipeline->AttachDescriptorSet(0, m_imgui.m_screen_res_set);

	// Attach font buffer
	m_imgui.m_imgui_pipeline->AttachDescriptorPool(m_imgui.m_font_texture_pool);
	m_imgui.m_imgui_pipeline->AttachDescriptorSet(1, m_imgui.m_texture_descriptor_set);
	

	// Build Pipeline
	m_imgui.m_imgui_pipeline->Build();

	const int temp_vert_max = 30000;
	const int temp_in_max = 40000;

	m_imgui.m_vertex_data = new ImDrawVert[temp_vert_max];
	m_imgui.m_vertex_buffer = m_renderer->CreateVertexBuffer(m_imgui.m_vertex_data, sizeof(ImDrawVert), temp_vert_max);

	m_imgui.m_index_data = new ImDrawIdx[temp_in_max];
	m_imgui.m_index_buffer = m_renderer->CreateIndexBuffer(m_imgui.m_index_data, sizeof(ImDrawIdx), temp_in_max);

	// Setup model instance
	m_imgui.model_pool = m_renderer->CreateModelPool(m_imgui.m_vertex_buffer, m_imgui.m_index_buffer);
	m_imgui.model = m_imgui.model_pool->CreateModel();
	m_imgui.model_pool->SetVertexDrawCount(0);

	m_imgui.m_imgui_pipeline->AttachModelPool(m_imgui.model_pool);
}

void ComponentEngine::Engine::UpdateImGUI()
{
	m_ui->Render();
	ImDrawData* imDrawData = ImGui::GetDrawData();

	if (imDrawData == nullptr)return;

	if (m_imgui.m_vertex_buffer->GetElementCount(BufferSlot::Primary) < imDrawData->TotalVtxCount ||
		m_imgui.m_index_buffer->GetElementCount(BufferSlot::Primary) < imDrawData->TotalIdxCount)
	{
		throw "Dynamic GUI buffers not handled";
	}
	ImDrawVert* temp_vertex_data = m_imgui.m_vertex_data;
	ImDrawIdx* temp_index_data = m_imgui.m_index_data;
	unsigned int index_count = 0;
	unsigned int vertex_count = 0;
	for (int n = 0; n < imDrawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];
		memcpy(temp_vertex_data, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		// Loop through and manually add a offset to the index's so they can all be rendered in one render pass
		for (int i = 0; i < cmd_list->IdxBuffer.Size; i++)
		{
			temp_index_data[i] = cmd_list->IdxBuffer.Data[i] + vertex_count;
		}

		temp_vertex_data += cmd_list->VtxBuffer.Size;
		temp_index_data += cmd_list->IdxBuffer.Size;

		vertex_count += cmd_list->VtxBuffer.Size;
		index_count += cmd_list->IdxBuffer.Size;
	}

	// Submit the payload to the GPU

	m_imgui.model_pool->SetVertexDrawCount(index_count);


	GetRendererMutex().lock();
	if (IsRunning())
	{
		m_imgui.m_vertex_buffer->SetData(BufferSlot::Primary);
		m_imgui.m_index_buffer->SetData(BufferSlot::Primary);
	}
	GetRendererMutex().unlock();
}

void ComponentEngine::Engine::DeInitImGUI()
{
	ImGui::SaveIniSettingsToDisk("imgui.ini");
	delete m_imgui.m_imgui_pipeline;
	delete m_imgui.m_screen_res_buffer;
	delete m_imgui.m_screen_res_pool;
	delete m_imgui.m_screen_res_set;
	delete m_imgui.m_font_texture;
	delete m_imgui.m_font_texture_pool;
	delete m_imgui.m_texture_descriptor_set;
	delete m_imgui.m_vertex_data;
	delete m_imgui.m_index_data;
	delete m_imgui.m_vertex_buffer;
	delete m_imgui.m_index_buffer;
	delete m_imgui.model_pool;
	delete m_imgui.model;
	delete m_ui;


}


void ComponentEngine::Engine::NewThreadUpdatePass()
{
	std::thread::id id = std::this_thread::get_id();

	auto it = m_thread_linker.find(id);
	if (it == m_thread_linker.end())return;

	ThreadData*& data = m_thread_linker[id];
	float thread_last = GetThreadDeltaTime(); // Get the time from the last call to Sync or NewThreadUpdatePass
	data->process_time = thread_last + data->delta_process_time; // Set the final time for data.process_time for this loop
	data->loop_time = thread_last + data->delta_loop_time; // Set the final time for data.delta_loop_time for this loop



	data->process_time_average_storage.push_back(data->process_time);
	data->loop_time_average_storage.push_back(data->loop_time);

	if (data->process_time_average_storage.size() >= 10)
	{
		data->process_time_average = 0.0f;
		data->loop_time_average = 0.0f;
		for (int i = 0; i < data->process_time_average_storage.size(); i++)
		{
			data->process_time_average += data->process_time_average_storage[i];
			data->loop_time_average += data->loop_time_average_storage[i];
		}
		data->process_time_average /= data->process_time_average_storage.size();
		data->loop_time_average /= data->loop_time_average_storage.size();
		data->process_time_average_storage.clear();
		data->loop_time_average_storage.clear();
	}
	// Reset the process time delta and delta loop time for this loop
	data->delta_process_time = 0.0f;
	data->delta_loop_time = 0.0f;
}


void ComponentEngine::Engine::RequestStop()
{
	m_request_stop = true;
}

void ComponentEngine::Engine::RequestToggleThreading()
{
	m_request_toggle_threading = true;
}

void ComponentEngine::Engine::ToggleFrameLimiting()
{
	std::lock_guard<std::mutex> guard(m_locks[TOGGLE_FRAME_LIMITING]);
	std::thread::id id = std::this_thread::get_id();
	auto it = m_thread_linker.find(id);
	if (it == m_thread_linker.end())return;
	ThreadData*& data = m_thread_linker[id];
	data->frame_limited = !data->frame_limited;
}

bool ComponentEngine::Engine::Threading()
{
	return m_threading;
}

void ComponentEngine::Engine::ToggleThreading()
{
	m_threading = !m_threading;

	for (int i = m_thread_data.size() - 1; i >= 0 ; i--)
	{
		auto it = m_thread_data.begin() + i;

		ThreadData* data = *it;
		data->ResetTimers();
		if (data->thread_instance != nullptr)
		{

			if (!m_threading)
			{
				// Thread has to be stored this way otherwise it plays up and points to the wrong thing
				std::thread::id id = data->thread_instance->GetID();

				data->thread_instance->Join();

				data->data_lock.lock();

				auto find = m_thread_linker.find(id);
				if (find != m_thread_linker.end())
				{
					m_thread_linker.erase(find);
				}
				data->data_lock.unlock();
			}
			else
			{
				data->data_lock.lock();

				data->thread_instance->StartThread();
				// Thread has to be stored this way otherwise it plays up and points to the wrong thing
				std::thread::id id = data->thread_instance->GetID();

				m_thread_linker[id] = data;
				data->data_lock.unlock();
			}




		}

	}

}

void ComponentEngine::Engine::ThreadData::ResetTimers()
{
	delta_process_time = 0.0f;
	process_time = 0.0f;
	process_time_average = 0.0f;
	delta_loop_time = 0.0f;
	loop_time = 0.0f;
	loop_time_average = 0.0f;
	delta_time = SDL_GetPerformanceCounter();
}
