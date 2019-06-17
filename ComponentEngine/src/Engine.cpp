#include <ComponentEngine\Engine.hpp>

#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <ComponentEngine\Components\ParticalSystem.hpp>
#include <ComponentEngine\Components\Rigidbody.hpp>
#include <ComponentEngine\Components\ICollisionShape.hpp>
#include <ComponentEngine\Components\BoxCollision.hpp>
#include <ComponentEngine\Components\SphereCollision.hpp>


#include <ComponentEngine\UI\UIManager.hpp>
#include <ComponentEngine\UI\Console.hpp>
#include <ComponentEngine\UI\Threading.hpp>
#include <ComponentEngine\UI\ComponentHierarchy.hpp>
#include <ComponentEngine\UI\Explorer.hpp>
#include <ComponentEngine\UI\SceneHierarchy.hpp>
#include <ComponentEngine\UI\MenuElement.hpp>
#include <ComponentEngine\UI\EditorState.hpp>

#include <lodepng.h>

#include <assert.h>
#include <sstream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

Engine* Engine::m_engine = nullptr;

bool ComponentEngine::Engine::IsRunning()
{
	std::lock_guard<std::mutex> guard(GetLock(IS_RUNNING));
	return m_running == EngineStates::Running;
}

ComponentEngine::Engine::Engine()
{
	// Reset key codes
	for (int i = 0; i < 256; i++)
	{
		m_keys[i] = false;
	}

	m_flags = 0;
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
	delete m_threadManager;
}

void ComponentEngine::Engine::Start()
{
	m_title = "Component Engine";
	m_width = 1080;
	m_height = 720;
	InitWindow();
	InitEnteeZ();
	InitRenderer();
	InitImGUI();
	InitPhysicsWorld();
	InitComponentHooks();
	m_main_thread = std::this_thread::get_id();
	m_running = EngineStates::Running;

	m_threadManager = new ThreadManager(ThreadMode::Threading);

	// Render a frame so you know it has not crashed xD
	RenderFrame();

	bool editor = !(m_flags & EngineFlags::ReleaseBuild) == EngineFlags::ReleaseBuild;
	if (editor)
	{
		m_play_state = PlayState::Editor;
		// Add UI task
		m_threadManager->AddTask([&](float frameTime) {
			UpdateUI(frameTime);
		}, 30, "UI");

		// Add Update Scene task
		m_threadManager->AddTask([&, this](float frameTime) {
			EntityManager& em = GetEntityManager();
			m_logic_lock.lock();
			if (m_play_state == PlayState::Play)
			{
				for (auto e : em.GetEntitys())
				{
					e->ForEach<Logic>([&](enteez::Entity * entity, Logic & logic)
					{
						logic.Update(frameTime);
					});
				}
			}
			else
			{
				for (auto e : em.GetEntitys())
				{
					e->ForEach<Logic>([&](enteez::Entity * entity, Logic & logic)
					{
						logic.EditorUpdate(frameTime);
					});
				}
			}

			m_logic_lock.unlock();
		}, 60, "Scene Update");

		// Update physics world
		m_threadManager->AddTask([&](float frameTime) {

			m_logic_lock.lock();
			bool playing = m_play_state == PlayState::Play;
			m_logic_lock.unlock();
			if (playing)
			{
				m_physicsWorld->Update(frameTime);
			}
		}, 60, "PhysicsWorld");
	}
	else
	{
		m_play_state = PlayState::Play;
		// Add Update Scene task
		m_threadManager->AddTask([&, this](float frameTime) {
			EntityManager& em = GetEntityManager();
			m_logic_lock.lock();
			for (auto e : em.GetEntitys())
			{
				e->ForEach<Logic>([&](enteez::Entity * entity, Logic & logic)
				{
					logic.Update(frameTime);
				});
			}
			m_logic_lock.unlock();
		}, 60, "Scene Update");

		// Update physics world
		m_threadManager->AddTask([&](float frameTime) {
			m_physicsWorld->Update(frameTime);
		}, 60, "PhysicsWorld");
	}

	// Add Render task
	m_threadManager->AddTask([&](float frameTime) {
		RenderFrame();
	}, 60, "Render");

	// Add Update Scene Buffers task
	m_threadManager->AddTask([&](float frameTime) {
		UpdateScene();
	}, 60, "Scene Buffer Swapping");
	

	Log("Starting Engine", Info);
}

void ComponentEngine::Engine::Stop()
{
	if (m_running != EngineStates::Stoping)
	{
		return;
	}
	m_threadManager->ChangeMode(ThreadMode::Joined);
	{
		std::lock_guard<std::mutex> guard(GetLock(IS_RUNNING));
		m_running = EngineStates::Stopped;
	}



	GetRendererMutex().lock();


	GetEntityManager().Clear();
	DeInitEnteeZ();
	DeInitPhysicsWorld();
	DeInitRenderer();
	DeInitWindow();
	GetRendererMutex().unlock();
	Log("Stopping Engine", Info);
}

void ComponentEngine::Engine::Join()
{
	m_threadManager->ChangeMode(Joined);
}

bool ComponentEngine::Engine::Running()
{
	//NewThreadUpdatePass();
	if (m_main_thread == std::this_thread::get_id())
	{
		if (m_request_stop)
		{
			m_running = EngineStates::Stoping;
			/*m_request_stop = false;
			Stop();*/
			return false;
		}
		else if (m_request_toggle_threading)
		{
			m_request_toggle_threading = false;
			ToggleThreading();
		}
		std::lock_guard<std::mutex> guard(GetLock(IS_RUNNING));
		bool result = m_renderer != nullptr && m_renderer->IsRunning();

		//GetRendererMutex().lock();
		//GetRendererMutex().unlock();
		return result;
	}
	else
	{
		std::lock_guard<std::mutex> guard(GetLock(IS_RUNNING));
		return m_running == EngineStates::Stopped;;
	}
}

void ComponentEngine::Engine::Update()
{
	m_threadManager->Update();
	GetRendererMutex().lock();
	UpdateWindow();
	GetRendererMutex().unlock();
}

void ComponentEngine::Engine::UpdateScene()
{
	EntityManager& em = GetEntityManager();

	m_logic_lock.lock();
	Mesh::SetBufferData();
	for (auto e : em.GetEntitys())
	{
		e->ForEach<TransferBuffers>([&](enteez::Entity * entity, TransferBuffers & buffer)
		{
			buffer.SetBufferData();
		});
	}

	GetRendererMutex().lock();
	Mesh::TransferToPrimaryBuffers();
	for (auto e : em.GetEntitys())
	{
		e->ForEach<TransferBuffers>([&](enteez::Entity * entity, TransferBuffers & buffer)
		{
			buffer.BufferTransfer();
		});
	}
	GetRendererMutex().unlock();
	m_logic_lock.unlock();
}

void ComponentEngine::Engine::UpdateUI(float delta)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = delta;
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
	if (m_main_camera == nullptr)return;
	GetRendererMutex().lock();
	// Update all renderer's via there Update function
	m_renderer->Update();
	GetRendererMutex().unlock();
}



void ComponentEngine::Engine::Log(std::string data, ConsoleState state)
{
	std::stringstream ss;
	ss << "(%i) " << data;

	std::lock_guard<std::mutex> guard(GetLock(CONSOLE));

	if (m_console.size() > 0 && m_console[m_console.size() - 1].message == ss.str() && m_console[m_console.size() - 1].state == state)
	{
		m_console[m_console.size() - 1].count++;
	}
	else
	{
		ConsoleMessage message;
		message.message = ss.str();
		message.count = 1;
		message.state = state;
		m_console.push_back(message);
	}
}

bool ComponentEngine::Engine::KeyDown(int key)
{
	std::lock_guard<std::mutex> guard(m_locks[READ_KEY_PRESS]);
	return m_keys[key];
}

bool ComponentEngine::Engine::MouseKeyDown(int key)
{
	ImGuiIO& io = ImGui::GetIO();
	std::lock_guard<std::mutex> guard(m_locks[READ_MOUSE_DATA]);
	return io.MouseDown[key];
}

glm::vec2 ComponentEngine::Engine::GetLastMouseMovment()
{
	std::lock_guard<std::mutex> guard(m_locks[READ_MOUSE_DATA]);
	return m_mousePosDelta;
}

bool ComponentEngine::Engine::LoadScene(const char * path)
{

	// clear scene
	{
		m_logic_lock.lock();
		GetRendererMutex().lock();

		GetEntityManager().Clear();
		//UpdateScene();

		GetRendererMutex().unlock();
		m_logic_lock.unlock();
	}

	std::ifstream in(path);

	EntityManager& em = GetEntityManager();

	m_logic_lock.lock();
	GetRendererMutex().lock();

	
	if (!in.is_open()) 
	{
		Log("Uable to load scene: Could not open", ConsoleState::Error);
		// Return locks as we do not need them anymore
		GetRendererMutex().unlock();
		m_logic_lock.unlock();
		return false;
	}

	{ // Validation check

		// Load String
		unsigned int stringLength;
		Common::Read(in, &stringLength, sizeof(unsigned int));

		// -1 for \0
		if (stringLength - 1 != EngineName.size())
		{
			Log("Uable to load scene: Invalid Header", ConsoleState::Error);
			// Return locks as we do not need them anymore
			GetRendererMutex().unlock();
			m_logic_lock.unlock();
			return false;
		}

		// Load string content
		char* data = new char[stringLength];
		Common::Read(in, data, sizeof(char) * stringLength);
		std::string str;
		str.assign(data);

		if (str != EngineName)
		{
			Log("Uable to load scene: Invalid Header", ConsoleState::Error);
			// Return locks as we do not need them anymore
			GetRendererMutex().unlock();
			m_logic_lock.unlock();
			return false;
		}
	}

	SetScenePath(path);

	
	unsigned int topLevelEntityCount;
	Common::Read(in, &topLevelEntityCount, sizeof(unsigned int));


	std::function<void(Entity* parent)> createEntity = [&](Entity* parent)
	{
		std::string entityName = Common::ReadString(in);

		Entity* entity = em.CreateEntity(entityName);
		unsigned int componentCount;

		Common::Read(in, &componentCount, sizeof(unsigned int));

		for (int j = 0; j < componentCount; j++)
		{
			std::string componentName = Common::ReadString(in);

			auto it = m_component_register.find(componentName);
			if (it != m_component_register.end())
			{
				BaseComponentWrapper* wrapper = it->second(*entity);
				IO* io = nullptr;
				if (em.BaseClassInstance(*wrapper, io))
				{
					io->Load(in);
				}
			}
			else
			{
				std::stringstream ss;
				ss << "Unable to find component '" << componentName << "' for entity '" << entityName << "'";
				Log(ss.str(), ComponentEngine::Warning);
			}
		}

		if (!entity->HasComponent<Transformation>())
		{
			Transformation::EntityHookDefault(*entity);
		}

		entity->GetComponent<Transformation>().SetParent(parent);

		unsigned int childrenCount;
		Common::Read(in, &childrenCount, sizeof(unsigned int));

		for (int i = 0; i < childrenCount; i++)
		{
			createEntity(entity);
		}
	};


	for (int i = 0; i < topLevelEntityCount; i++)
	{
		createEntity(nullptr);
	}
	GetRendererMutex().unlock();
	m_logic_lock.unlock();

	return true;
}

void ComponentEngine::Engine::SaveScene()
{

	std::ofstream out(m_currentScene);
	EntityManager& em = GetEntityManager();

	{ // Add validation to file
		Common::Write(out, EngineName);
	}


	std::function<void(Entity*)> saveEntity = [&](Entity* entity)
	{
		// Output entitys name
		Common::Write(out, entity->GetName());

		// Output the amount of components the entity has
		unsigned int componentCount = entity->GetComponentCount();
		Common::Write(out, &componentCount, sizeof(unsigned int));

		entity->ForEach([&](BaseComponentWrapper& wrapper) {
			// Output Components name 
			Common::Write(out, wrapper.GetName());

			IO* io = nullptr;
			if (em.BaseClassInstance(wrapper, io))
			{
				io->Save(out);
			}
		});

		if (entity->HasComponent<Transformation>())
		{
			Transformation& trans = entity->GetComponent<Transformation>();

			// Output the amount of children the entity has
			unsigned int childrenCount = trans.GetChildren().size();
			Common::Write(out, &childrenCount, sizeof(unsigned int));

			for (Transformation* child : trans.GetChildren())
			{
				saveEntity(child->GetEntity());
			}
		}
		else
		{
			// Output the amount of children the entity has
			unsigned int childrenCount = 0;
			Common::Write(out, &childrenCount, sizeof(unsigned int));
		}

	};

	std::vector<Entity*> topLevelEntities;
	for (auto entity : em.GetEntitys())
	{
		if (entity->GetComponent<Transformation>().GetParent() == nullptr)
		{
			topLevelEntities.push_back(entity);
		}
	}

	unsigned int entityCount = topLevelEntities.size();
	Common::Write(out, &entityCount, sizeof(unsigned int));

	for (auto entity : topLevelEntities)
	{
		saveEntity(entity);
	}

	out.flush();
	out.close();
}

void ComponentEngine::Engine::AddPipeline(std::string name, PipelinePack pipeline)
{
	m_pipelines[name] = pipeline;
}

PipelinePack& ComponentEngine::Engine::GetPipeline(std::string name)
{
	if (m_pipelines.find(name) != m_pipelines.end())return m_pipelines[name];
	return m_pipelines["Default"];
}

PipelinePack & ComponentEngine::Engine::GetPipelineContaining(std::string name)
{
	for (auto p : m_pipelines)
	{
		if (Common::Contains(name, p.first))
		{
			return m_pipelines[p.first];
		}
	}
	return m_pipelines["Default"];
}

IGraphicsPipeline * ComponentEngine::Engine::GetDefaultGraphicsPipeline()
{
	return m_default_pipeline;
}

IRenderer * ComponentEngine::Engine::GetRenderer()
{
	return m_renderer;
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

VertexBase ComponentEngine::Engine::GetDefaultVertexModelBinding()
{
	return VertexBase{
			VertexInputRate::INPUT_RATE_VERTEX,
			{
				{ 0, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,position) },
				{ 1, DataFormat::R32G32_FLOAT,offsetof(MeshVertex,uv) },
				{ 2, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,normal) },
				{ 3, DataFormat::R32G32B32_FLOAT,offsetof(MeshVertex,color) },
			},
			sizeof(MeshVertex),
			0
	};
}

VertexBase ComponentEngine::Engine::GetDefaultVertexModelPositionBinding()
{
	return VertexBase{
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
	};
}
 
TextureStorage& ComponentEngine::Engine::GetTexture(std::string path)
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
		m_texture_storage[path].texture = texture;


		m_texture_storage[path].texture_maps_pool = GetRenderer()->CreateDescriptorPool({
			GetRenderer()->CreateDescriptor(Renderer::DescriptorType::IMAGE_SAMPLER, Renderer::ShaderStage::FRAGMENT_SHADER, 0),
		});


		IDescriptorSet* texture_maps_descriptor_set = m_texture_storage[path].texture_maps_pool->CreateDescriptorSet();

		texture_maps_descriptor_set->AttachBuffer(0, texture);

		texture_maps_descriptor_set->UpdateSet();

		m_texture_storage[path].texture_descriptor_set = texture_maps_descriptor_set;


		{
			std::stringstream ss;
			ss << "Loaded texture" << path;
			Log(ss.str(), Info);
		}
	}
	return m_texture_storage[path];
}

std::string ComponentEngine::Engine::GetCurrentScene()
{
	return m_currentScene;
}

std::string ComponentEngine::Engine::GetCurrentSceneDirectory()
{
	return m_currentSceneDirectory;
}

ordered_lock& ComponentEngine::Engine::GetLogicMutex()
{
	return m_logic_lock;
}

ordered_lock& ComponentEngine::Engine::GetRendererMutex()
{
	return m_renderer_thread;
}

ThreadManager * ComponentEngine::Engine::GetThreadManager()
{
	return m_threadManager;
}

void ComponentEngine::Engine::SetCamera(Camera* camera)
{
	GetRendererMutex().lock();
	m_main_camera = camera;
	m_camera_descriptor_set->AttachBuffer(0, camera->GetCameraBuffer());
	m_camera_descriptor_set->UpdateSet();
	camera->UpdateProjection();
	Rebuild();
	GetRendererMutex().unlock();
}

bool ComponentEngine::Engine::HasCamera()
{
	return m_main_camera != m_default_camera;
}

Camera* ComponentEngine::Engine::GetMainCamera()
{
	return m_main_camera;
}

Camera* ComponentEngine::Engine::GetDefaultCamera()
{
	return m_default_camera;;
}

NativeWindowHandle* ComponentEngine::Engine::GetWindowHandle()
{
	return m_window_handle;
}

void ComponentEngine::Engine::RegisterComponentBase(std::string name, BaseComponentWrapper*(*default_initilizer)(enteez::Entity &entity))
{
	m_component_register[name] = default_initilizer;
}

void ComponentEngine::Engine::GrabMouse(bool grab)
{
	SDL_SetRelativeMouseMode((SDL_bool)grab);
	SDL_GetMouseState(&m_lockedPosX, &m_lockedPosY);
}

void ComponentEngine::Engine::SetPlayState(PlayState play_state)
{
	m_logic_lock.lock();
	m_play_state = play_state;
	m_logic_lock.unlock();
}

PlayState ComponentEngine::Engine::GetPlayState()
{
	return m_play_state;
}

PhysicsWorld * ComponentEngine::Engine::GetPhysicsWorld()
{
	return m_physicsWorld;
}

std::mutex & ComponentEngine::Engine::GetLock(EngineLock lock)
{
	return m_locks[(int)lock];
}

std::vector<ConsoleMessage>& ComponentEngine::Engine::GetConsoleMessages()
{
	return m_console;
}

std::map<std::string, BaseComponentWrapper*(*)(enteez::Entity& entity)> ComponentEngine::Engine::GetComponentRegister()
{
	return m_component_register;
}

void ComponentEngine::Engine::SetFlag(int flags)
{
	m_flags |= flags;
}

UIManager * ComponentEngine::Engine::GetUIManager()
{
	return m_ui;
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
		GetWindowFlags(VulkanAPI) | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
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
	m_mousePosDelta = glm::vec2();
	while (SDL_PollEvent(&event) > 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			if (Running())
			{
				RequestStop();
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
				if(m_main_camera!=nullptr) m_main_camera->UpdateProjection();
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
			{
				if (!SDL_GetRelativeMouseMode())
				{
					std::lock_guard<std::mutex> guard(m_locks[READ_MOUSE_DATA]);
					io.MousePos = ImVec2(event.motion.x, event.motion.y);
				}
				m_lastMousePos = glm::vec2(event.motion.x, event.motion.y);
				m_mousePosDelta = glm::vec2(event.motion.xrel, event.motion.yrel);
			}
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
				{
					std::lock_guard<std::mutex> guard(m_locks[READ_KEY_PRESS]);
					// Set both our local key press array and imgui key press array
					io.KeysDown[key] = m_keys[key] = (event.type == SDL_KEYDOWN);
				}
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
	RegisterBase<Transformation, MsgRecive<TransformationPtrRedirect>, UI, IO>();
	RegisterBase<RendererComponent, UI, MsgRecive<OnComponentEnter<Mesh>>>();
	RegisterBase<Mesh, MsgRecive<RenderStatus>, UI, IO, MsgRecive<OnComponentEnter<Transformation>>, MsgRecive<OnComponentExit<Transformation>>>();
	RegisterBase<ParticleSystem, Logic, UI, MsgRecive<ParticleSystemVisibility>, IO>();
	RegisterBase<Camera, Logic, UI, TransferBuffers>(); 
	RegisterBase<Rigidbody,
		MsgRecive<TransformationChange>, MsgRecive<OnComponentEnter<ICollisionShape>>,
		MsgRecive<CollisionRecording>, MsgRecive<CollisionEvent>,
		MsgRecive<OnComponentChange<ICollisionShape>>, MsgRecive<OnComponentExit<ICollisionShape>>,
		MsgRecive<OnComponentExit<Rigidbody>>, Logic, UI
	>();
	RegisterBase<BoxCollision, ICollisionShape, Logic, UI>();
	RegisterBase<SphereCollision, ICollisionShape, Logic, UI>();
}

void ComponentEngine::Engine::DeInitEnteeZ()
{

}

void ComponentEngine::Engine::InitRenderer()
{
	// Create a instance of the renderer
	m_renderer = new VulkanRenderer();
	m_renderer->Start(m_window_handle);

	// If the rendering was not fully created, error out
	assert(m_renderer != nullptr && "Error, renderer instance could not be created");

	//m_camera_entity = this->GetEntityManager().CreateEntity("Camera");
	//m_camera_entity->AddComponent(&m_camera_component);

	m_default_camera = new Camera();

	// Create camera pool
	// This is a layout for the camera input data
	m_camera_pool = m_renderer->CreateDescriptorPool({
		m_renderer->CreateDescriptor(Renderer::DescriptorType::UNIFORM, Renderer::ShaderStage::VERTEX_SHADER, 0),
		});


	// Create camera descriptor set from the tempalte
	m_camera_descriptor_set = m_camera_pool->CreateDescriptorSet();
	// 
	m_camera_descriptor_set->UpdateSet();


	// Create default pipeline
	m_default_pipeline = m_renderer->CreateGraphicsPipeline({
		{ ShaderStage::VERTEX_SHADER, "../Shaders/Default/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../Shaders/Default/frag.spv" }
		});

	// Tell the pipeline what data is should expect in the forum of Vertex input
	m_default_pipeline->AttachVertexBinding(GetDefaultVertexModelBinding());

	m_default_pipeline->AttachVertexBinding(GetDefaultVertexModelPositionBinding());

	// Tell the pipeline what the input data will be payed out like
	m_default_pipeline->AttachDescriptorPool(m_camera_pool);
	// Attach the camera descriptor set to the pipeline
	m_default_pipeline->AttachDescriptorSet(0, m_camera_descriptor_set);

	m_texture_maps_pool = Engine::Singlton()->GetRenderer()->CreateDescriptorPool({
		Engine::Singlton()->GetRenderer()->CreateDescriptor(Renderer::DescriptorType::IMAGE_SAMPLER, Renderer::ShaderStage::FRAGMENT_SHADER, 0),
		});

	//m_default_pipeline->AttachDescriptorPool(m_texture_maps_pool);

	m_default_pipeline->UseCulling(true);

	bool sucsess = m_default_pipeline->Build();
	m_pipelines["Default"] = PipelinePack{ m_default_pipeline };
	// Build and check default pipeline
	assert(sucsess && "Unable to build default pipeline");
	SetCamera(m_default_camera);

}

void ComponentEngine::Engine::DeInitRenderer()
{
	DeInitImGUI();

	Mesh::CleanUp();

	for (auto it = m_texture_storage.begin(); it != m_texture_storage.end(); it++)
	{
		delete it->second.texture_maps_pool;
		delete it->second.texture_descriptor_set;
		delete it->second.texture;
	}
	m_texture_storage.clear();


	delete m_default_camera;
	m_default_camera = nullptr;
	//delete m_default_pipeline;
	//m_default_pipeline = nullptr;

	for (auto& pipeline : m_pipelines)
	{
		delete pipeline.second.pipeline;
	}

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
	
	RegisterComponentBase("Transformation", Transformation::EntityHookDefault);
	RegisterComponentBase("Mesh", Mesh::EntityHookDefault);
	RegisterComponentBase("Renderer", RendererComponent::EntityHookDefault);
	RegisterComponentBase("Particle System", ParticleSystem::EntityHookDefault);
	RegisterComponentBase("Camera", Camera::EntityHookDefault);
	RegisterComponentBase("Rigidbody", Rigidbody::EntityHookDefault);
	RegisterComponentBase("Box Collision", BoxCollision::EntityHookDefault);
	RegisterComponentBase("Sphere Collision", SphereCollision::EntityHookDefault);
}

void ComponentEngine::Engine::InitPhysicsWorld()
{
	m_physicsWorld = new PhysicsWorld(this);
}

void ComponentEngine::Engine::DeInitPhysicsWorld()
{
	delete m_physicsWorld;
}

void ComponentEngine::Engine::InitImGUI()
{

	m_ui = new UIManager(this);


	bool editor = !(m_flags & EngineFlags::ReleaseBuild) == EngineFlags::ReleaseBuild;
	if (editor)
	{
		m_ui->AddElement(new Console("Console", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar, PlayState::Editor));
		m_ui->AddElement(new ThreadingWindow("Threading", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar, PlayState::Editor));
		m_ui->AddElement(new ComponentHierarchy("ComponentHierarchy", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar, PlayState::Editor));
		m_ui->AddElement(new Explorer("Explorer", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar, PlayState::Editor));
		m_ui->AddElement(new SceneHierarchy("SceneHierarchy", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar, PlayState::Editor));
		m_ui->AddElement(new EditorState("EditorState", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking, PlayState::Editor | PlayState::Play));

		static std::string filenameAutofill = "Scene.bin";
		static std::string defaultSavePath = "../Scene.bin";
		static unsigned int maxFileLength = 200;
		static char* saveAsStream = new char[maxFileLength + 1];

		m_ui->AddMenuElement(new MenuElement("File", 
		{
			new MenuElement("Add",[&] {
				m_engine->GetThreadManager()->AddTask([&](float frameTime) {
					EntityManager& em = GetEntityManager();
					enteez::Entity* entity = em.CreateEntity("New Entity");

					Transformation::EntityHookDefault(*entity);

					Transformation& transformation = entity->GetComponent<Transformation>();
					transformation.SetParent(m_ui->GetCurrentSceneFocus().entity);
				});
			}),
			new MenuElement("Reload Scene",[&] {
				m_engine->GetThreadManager()->AddTask([&](float frameTime) {

					m_logic_lock.lock();
					GetRendererMutex().lock();

					GetEntityManager().Clear();
					// Load in the scene
					LoadScene(m_engine->GetCurrentScene().c_str());
					UpdateScene();

					GetRendererMutex().unlock();
					m_logic_lock.unlock();

				});
			}),

			new MenuElement(MenuElementFlags::Spacer),

			new MenuElement("Save",[&] {
				m_logic_lock.lock();
				GetRendererMutex().lock();


				if (m_currentScene.size() == 0)
				{
					ImGui::OpenPopup("Save As##SaveAsFilePopup");


					for (int i = 0; i < filenameAutofill.size(); i++)
					{
						saveAsStream[i] = filenameAutofill.at(i);
					}

					for (int i = filenameAutofill.size(); i <= maxFileLength; i++)
					{
						saveAsStream[i] = '\0';
					}
				}
				else
				{
					m_engine->GetThreadManager()->AddTask([&](float frameTime) {
						SaveScene();
					});
				}

				GetRendererMutex().unlock();
				m_logic_lock.unlock();

			}),
			new MenuElement("Save As",[&] {

				ImGui::OpenPopup("Save As##SaveAsFilePopup");




				for (int i = 0; i < filenameAutofill.size(); i++)
				{
					saveAsStream[i] = filenameAutofill.at(i);
				}

				for (int i = filenameAutofill.size(); i <= maxFileLength; i++)
				{
					saveAsStream[i] = '\0';
				}

			},[&] {

				ImVec2 windowSize = ImVec2(400, 300);
				ImGui::SetNextWindowSize(windowSize);
				if (ImGui::BeginPopupModal("Save As##SaveAsFilePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
				{
					static FileForms saveFileForm;
					static Folder loadFolder;
					static std::string savePath = defaultSavePath;

					if (loadFolder.path.longForm != "../")
					{
						loadFolder = Folder();
						loadFolder.path.longForm = "../";
						loadFolder.path.shortForm = "/";
						Explorer::LoadFolder(loadFolder);
					}
					loadFolder.topLevel = true;
					

					ImGui::Text("Path: %s", savePath.c_str());

					{ // Scroll Window
						ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
						ImGui::BeginChild("Child1", ImVec2(windowSize.x, windowSize.y - 95), false, window_flags);

						Explorer::RendererFolder(loadFolder, [&](const char* path)
						{
							saveFileForm.GenerateFileForm(path);
							savePath = saveFileForm.longForm;
						});

						ImGui::EndChild();
					}

					ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_AlwaysInsertMode;
					bool keyPress = ImGui::InputText("##data", saveAsStream, maxFileLength, flags);
					
					if (keyPress)
					{
						std::stringstream ss;
						if (saveFileForm.folder.size() > 0)
						{
							ss << saveFileForm.folder << "/" << saveAsStream;
						}
						else
						{
							ss << "../" << saveAsStream;
						}

						savePath = ss.str();
					}


					{ // OK Buton

						if (savePath.size() == 0)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.40f, 0.61f, 0.3f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.40f, 0.48f, 0.71f, 0.3f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.46f, 0.54f, 0.80f, 0.3f));
							ImGui::Button("OK", ImVec2(windowSize.x / 2 - 15, 0));
							ImGui::PopStyleColor(3);
						}
						else
						{
							if (ImGui::Button("OK", ImVec2(windowSize.x / 2 - 15, 0)))
							{
								m_engine->GetThreadManager()->AddTask([&](float frameTime) {
									SetScenePath(savePath.c_str());
									SaveScene();
									Explorer::LoadFolder(loadFolder);
								});
								ImGui::CloseCurrentPopup();
							}
						}
					}

					ImGui::SetItemDefaultFocus();
					ImGui::SameLine();

					{ // Close
						if (ImGui::Button("Cancel", ImVec2(windowSize.x / 2 - 15, 0))) { ImGui::CloseCurrentPopup(); }
					}
					ImGui::EndPopup();
				}

			}),

			new MenuElement("Load",[&] { // On click

				ImGui::OpenPopup("Load##LoadFilePopup");

			},[&] { // Post Render
				ImVec2 windowSize = ImVec2(400,300);
				ImGui::SetNextWindowSize(windowSize);
				if (ImGui::BeginPopupModal("Load##LoadFilePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
				{
					static Folder loadFolder;
					static std::string loadPath = "";

					if (loadFolder.path.longForm != "../")
					{
						loadFolder = Folder();
						loadFolder.path.longForm = "../";
						loadFolder.path.shortForm = "/";
						Explorer::LoadFolder(loadFolder);
					}
					loadFolder.topLevel = true;


					ImGui::Text("Path: %s", loadPath.c_str());

					{ // Scroll Window
						ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
						ImGui::BeginChild("Child1", ImVec2(windowSize.x, windowSize.y - 75), false, window_flags);

						Explorer::RendererFolder(loadFolder, [&](const char* path)
						{
							loadPath = path;
						});

						ImGui::EndChild();
					}

					
					if (loadPath.size() == 0)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.40f, 0.61f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.40f, 0.48f, 0.71f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.46f, 0.54f, 0.80f, 0.3f));
						ImGui::Button("OK", ImVec2(windowSize.x/2 - 15, 0));
						ImGui::PopStyleColor(3);
					}
					else
					{
						if (ImGui::Button("OK", ImVec2(windowSize.x / 2 - 15, 0)))
						{ 
							m_engine->GetThreadManager()->AddTask([&](float frameTime) {

								LoadScene(loadPath.c_str());

							});
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::SetItemDefaultFocus();
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(windowSize.x / 2 - 15, 0))) { ImGui::CloseCurrentPopup(); }
					ImGui::EndPopup();
				}

			}),

			new MenuElement(MenuElementFlags::Spacer),

			new MenuElement("Exit",[&] {
				GetRendererMutex().lock();
				RequestStop();
				GetRendererMutex().unlock();
			})
			}
		));
	}
	

	
	// Init ImGUI
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	if (editor)
	{
		io.IniFilename = "EditorUI.ini";
	}
	else
	{
		io.IniFilename = "UI.ini";
	}
	io.DisplaySize = ImVec2(m_window_handle->width, m_window_handle->height);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable | ImGuiBackendFlags_HasMouseHoveredViewport;

	ImGuiStyle& style = ImGui::GetStyle();

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
		{ ShaderStage::VERTEX_SHADER, "../Shaders/ImGUI/vert.spv" },
		{ ShaderStage::FRAGMENT_SHADER, "../Shaders/ImGUI/frag.spv" }
		}, true);
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
	m_logic_lock.lock();


	m_ui->Render();


	m_logic_lock.unlock();
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


	m_logic_lock.lock();
	GetRendererMutex().lock();
	if (IsRunning())
	{
		m_imgui.m_vertex_buffer->SetData(BufferSlot::Primary);
		m_imgui.m_index_buffer->SetData(BufferSlot::Primary);
	}
	GetRendererMutex().unlock();
	m_logic_lock.unlock();
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


/*void ComponentEngine::Engine::NewThreadUpdatePass()
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
}*/


void ComponentEngine::Engine::RequestStop()
{
	m_request_stop = true;
}

void ComponentEngine::Engine::RequestToggleThreading()
{
	m_request_toggle_threading = true;
}

/*void ComponentEngine::Engine::ToggleFrameLimiting()
{
	std::lock_guard<std::mutex> guard(m_locks[TOGGLE_FRAME_LIMITING]);
	std::thread::id id = std::this_thread::get_id();
	auto it = m_thread_linker.find(id);
	if (it == m_thread_linker.end())return;
	ThreadData*& data = m_thread_linker[id];
	data->frame_limited = !data->frame_limited;
}*/

bool ComponentEngine::Engine::Threading()
{
	return m_threading;
}

void ComponentEngine::Engine::ToggleThreading()
{
	m_threading = !m_threading;

	m_threadManager->ChangeMode(m_threading ? ThreadMode::Threading : ThreadMode::Joined);

}

void ComponentEngine::Engine::SetScenePath(const char * path)
{
	m_currentScene = path;

	{ // Get directory
		const size_t lastBackSlash = m_currentScene.rfind('\\');
		if (std::string::npos != lastBackSlash)
		{
			m_currentSceneDirectory = m_currentScene.substr(0, lastBackSlash);
		}
		else
		{
			const size_t lastForwardSlash = m_currentScene.rfind('/');
			if (std::string::npos != lastForwardSlash)
			{
				m_currentSceneDirectory = m_currentScene.substr(0, lastForwardSlash);
			}
			else
			{
				m_currentSceneDirectory = "../";
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
