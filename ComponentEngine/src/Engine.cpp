#include <ComponentEngine\Engine.hpp>

#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <ComponentEngine\Components\Rigidbody.hpp>
#include <ComponentEngine\Components\ICollisionShape.hpp>
#include <ComponentEngine\Components\BoxCollision.hpp>
#include <ComponentEngine\Components\SphereCollision.hpp>
#include <ComponentEngine\Components\Light.hpp>


#include <ComponentEngine\UI\UIManager.hpp>
#include <ComponentEngine\UI\Console.hpp>
#include <ComponentEngine\UI\Threading.hpp>
#include <ComponentEngine\UI\ComponentHierarchy.hpp>
#include <ComponentEngine\UI\Explorer.hpp>
#include <ComponentEngine\UI\SceneHierarchy.hpp>
#include <ComponentEngine\UI\MenuElement.hpp>
#include <ComponentEngine\UI\EditorState.hpp>
#include <ComponentEngine\UI\SceneWindow.hpp>
#include <ComponentEngine\UI\PlayWindow.hpp>

#include <renderer\VertexBase.hpp>

#include <renderer\vulkan\VulkanFlags.hpp>
#include <renderer\vulkan\VulkanRaytracePipeline.hpp>
#include <renderer\vulkan\VulkanAcceleration.hpp>
#include <renderer\vulkan\VulkanSwapchain.hpp>
#include <renderer\vulkan\VulkanBufferPool.hpp>
#include <renderer\vulkan\VulkanModelPool.hpp>
#include <renderer\vulkan\VulkanModel.hpp>
#include <renderer\vulkan\VulkanVertexBuffer.hpp>
#include <renderer\vulkan\VulkanIndexBuffer.hpp>
#include <renderer\vulkan\VulkanTextureBuffer.hpp>
#include <renderer\vulkan\VulkanDescriptorPool.hpp>
#include <renderer\vulkan\VulkanDescriptorSet.hpp>
#include <renderer\vulkan\VulkanSwapchain.hpp>
#include <renderer\vulkan\VulkanRenderPass.hpp>

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

	m_max_recursions = 10;

	m_all_vertexs.resize(m_vertex_max);
	m_all_indexs.resize(m_index_max);
}

Engine* ComponentEngine::Engine::Singlton()
{
	// If we do not have a instance of the engine already made, create one
	if (m_engine == nullptr)
	{
		m_engine = new Engine();
	}
	return m_engine;
}

// Stop the engine and destroy the thread manager
ComponentEngine::Engine::~Engine()
{
	Stop();
	delete m_threadManager;
}

// Start the engine and run all services
void ComponentEngine::Engine::Start()
{
	// Provide a default name and window size
	m_title = "Component Engine";
	m_width = 1080;
	m_height = 720;
	// Initialize the various core components of the engine
	InitWindow();
	InitEnteeZ();
	InitRenderer();
	InitImGUI();
	InitPhysicsWorld();
	// Define the EnteeZ component hooks
	InitComponentHooks();
	m_main_thread = std::this_thread::get_id();
	m_running = EngineStates::Running;

	// Start the thread manager
	m_threadManager = new ThreadManager(ThreadMode::Threading);

	// Render a frame so you know it has not crashed xD
	RenderFrame();
	// Are we in the editor or are we running a release version of the engine
	bool editor = !(m_flags & EngineFlags::ReleaseBuild) == EngineFlags::ReleaseBuild;
	if (editor)
	{
		// Define the play state to be in the editor
		m_play_state = PlayState::Editor;

		// Add Update Scene task. Update all components attached to each entity that supports it
		m_threadManager->AddTask([&, this](float frameTime) {
			EntityManager& em = GetEntityManager();
			m_logic_lock.lock();
			// Check if we are in a played state or not
			if (m_play_state == PlayState::Play)
			{
				// Loop through all entities in the scene
				for (auto e : em.GetEntitys())
				{
					// Loop through all components attached to the entity that inherits the Logic component
					e->ForEach<Logic>([&](enteez::Entity * entity, Logic & logic)
					{
						logic.Update(frameTime);
					});
				}
			}
			else
			{
				// Loop through all entities in the scene
				for (auto e : em.GetEntitys())
				{
					// Loop through all components attached to the entity that inherits the Logic component
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
		// Add Update Scene task. Update all components attached to each entity that supports it
		m_threadManager->AddTask([&, this](float frameTime) {
			EntityManager& em = GetEntityManager();
			m_logic_lock.lock();
			// Loop through all entities in the scene
			for (auto e : em.GetEntitys())
			{
				// Loop through all components attached to the entity that inherits the Logic component
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

	// Add UI task. Updates the UI manager
	m_threadManager->AddTask([&](float frameTime)
	{
		UpdateUI(frameTime);
	}, 30, "UI");

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

// Stop the engine and kill all services
void ComponentEngine::Engine::Stop()
{
	// If we are already stopped, skio
	if (m_running != EngineStates::Stoping)
	{
		return;
	}
	// Force all threads to join the primary one
	m_threadManager->ChangeMode(ThreadMode::Joined);
	// Set the engine to the stopped state
	{
		std::lock_guard<std::mutex> guard(GetLock(IS_RUNNING));
		m_running = EngineStates::Stopped;
	}

	GetRendererMutex().lock();
	// Remove all entities
	GetEntityManager().Clear();
	// Destroy all the core parts of the engine
	DeInitEnteeZ();
	DeInitPhysicsWorld();
	DeInitRenderer();
	DeInitWindow();
	GetRendererMutex().unlock();

	Log("Stopping Engine", Info);
}

// Force all threads to join the main thread
void ComponentEngine::Engine::Join()
{
	m_threadManager->ChangeMode(Joined);
}

// Is the engine running
bool ComponentEngine::Engine::Running()
{
	// Are we checking this state from the main thread
	if (m_main_thread == std::this_thread::get_id())
	{
		// Has a request to stop been made
		if (m_request_stop)
		{
			m_running = EngineStates::Stoping;
			return false;
		}
		else if (m_request_toggle_threading) // Has a request to switch between threading ~ not threading been made
		{
			m_request_toggle_threading = false;
			ToggleThreading();
		}
		std::lock_guard<std::mutex> guard(GetLock(IS_RUNNING));
		// return the renderer state
		bool result = m_renderer != nullptr && m_renderer->IsRunning();
		return result;
	}
	else
	{
		std::lock_guard<std::mutex> guard(GetLock(IS_RUNNING));
		return m_running == EngineStates::Stopped;;
	}
}

// Update the threading and window services
void ComponentEngine::Engine::Update()
{
	m_threadManager->Update();
	GetRendererMutex().lock();
	UpdateWindow();
	GetRendererMutex().unlock();
}

// Update the scene by transferring all data from the temporary secondary buffers to the primary ones
void ComponentEngine::Engine::UpdateScene()
{
	EntityManager& em = GetEntityManager();

	m_logic_lock.lock();
	Mesh::SetBufferData();
	// Store the data from local memory to the secondary buffer
	for (auto e : em.GetEntitys())
	{
		// Loop through each component that inherits TransferBuffers and set the data
		e->ForEach<TransferBuffers>([&](enteez::Entity * entity, TransferBuffers & buffer)
		{
			buffer.SetBufferData();
		});
	}

	GetRendererMutex().lock();
	Mesh::TransferToPrimaryBuffers();
	// Loop through each entity
	for (auto e : em.GetEntitys())
	{
		// Loop through each component that inherits TransferBuffers and transfer the data from the secondary buffer to the primary one
		e->ForEach<TransferBuffers>([&](enteez::Entity * entity, TransferBuffers & buffer)
		{
			buffer.BufferTransfer();
		});
	}

	m_model_position_buffer->SetData(BufferSlot::Primary);
	m_light_buffer->SetData(BufferSlot::Primary);

	GetRendererMutex().unlock();
	m_logic_lock.unlock();
}

// Update the ui manager
void ComponentEngine::Engine::UpdateUI(float delta)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = delta;
	UpdateImGUI();
}

// Rebuild the renderer components of the engine
void ComponentEngine::Engine::Rebuild()
{
	GetRendererMutex().lock();
	// Rebuild the swapchain as well as the image views
	m_swapchain->RebuildSwapchain();
	if (m_standardRTConfigSet)
	{
		// Update raytracer with new raytracing staging buffers
		m_standardRTConfigSet->AttachBuffer(1, { m_swapchain->GetRayTraceStagingBuffer() });
		m_standardRTConfigSet->UpdateSet();
	}
	// Rebuild the render pass instance
	m_render_pass->Rebuild();
	GetRendererMutex().unlock();
}

// Render the next scene frame
void ComponentEngine::Engine::RenderFrame()
{
	// If we do not have a camera, break
	if (m_main_camera == nullptr)return;
	GetRendererMutex().lock();
	// Update all renderer's via there Update function
	m_top_level_acceleration->Update();
	// Call the main engines render pass
	m_render_pass->Render();
	GetRendererMutex().unlock();
}

// Log some data to the internal console
void ComponentEngine::Engine::Log(std::string data, ConsoleState state)
{
	std::stringstream ss;
	ss << "(%i) " << data;
	std::lock_guard<std::mutex> guard(GetLock(CONSOLE));
	// If the last message was the same, append the log count
	if (m_console.size() > 0 && m_console[m_console.size() - 1].message == ss.str() && m_console[m_console.size() - 1].state == state)
	{
		m_console[m_console.size() - 1].count++;
	}
	else // Add the new message
	{
		ConsoleMessage message;
		message.message = ss.str();
		message.count = 1;
		message.state = state;
		m_console.push_back(message);
	}
}

// Check to see if X key is down
bool ComponentEngine::Engine::KeyDown(int key)
{
	std::lock_guard<std::mutex> guard(m_locks[READ_KEY_PRESS]);
	return m_keys[key];
}

// Check to see if X Mouse Button is down
bool ComponentEngine::Engine::MouseKeyDown(int key)
{
	ImGuiIO& io = ImGui::GetIO();
	std::lock_guard<std::mutex> guard(m_locks[READ_MOUSE_DATA]);
	return io.MouseDown[key];
}

// Return the last mouse movement instance
glm::vec2 ComponentEngine::Engine::GetLastMouseMovment()
{
	std::lock_guard<std::mutex> guard(m_locks[READ_MOUSE_DATA]);
	return m_mousePosDelta;
}

// Merge Scene stops the old scene from being deleted before the new scene is added so both scenes will be side by side.
bool ComponentEngine::Engine::LoadScene(const char * path)
{
	// clear scene
	{
		m_logic_lock.lock();
		GetRendererMutex().lock();

		GetEntityManager().Clear();

		GetRendererMutex().unlock();
		m_logic_lock.unlock();
	}

	std::ifstream in(path);

	EntityManager& em = GetEntityManager();


	// Check to see if the input stream is open, if not return
	if (!in.is_open()) 
	{
		Log("Uable to load scene: Could not open", ConsoleState::Error);
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
			return false;
		}
	}

	// Define where we have loaded from
	SetScenePath(path);

	// Read in how many top level entities there are (Entities that have no parents)
	unsigned int topLevelEntityCount;
	Common::Read(in, &topLevelEntityCount, sizeof(unsigned int));

	// Define a in-function, function for creating new entities using I/O stream
	std::function<void(Entity* parent)> createEntity = [&](Entity* parent)
	{
		m_logic_lock.lock();
		GetRendererMutex().lock();
		// File format for Entities
		/* - Entity Name
		** - Component Count (0-*)
		**     - Component Name
		**     - Component Payload (How much data the file has after this integer that is for the component)
		**     - Child count
		*/

		// Read in the name of the entity
		std::string entityName = Common::ReadString(in);
		Entity* entity = em.CreateEntity(entityName);

		// Read in the component count
		unsigned int componentCount;
		Common::Read(in, &componentCount, sizeof(unsigned int));

		// Loop through each component
		for (int j = 0; j < componentCount; j++)
		{
			// Read in the component name
			std::string componentName = Common::ReadString(in);

			// Read in how much data the file has for the component
			unsigned int payloadSize;
			Common::Read(in, &payloadSize, sizeof(unsigned int));

			// Check to see if we have that component on record
			auto it = m_component_register.find(componentName);

			// If we have the component
			if (it != m_component_register.end())
			{

				BaseComponentWrapper* wrapper = it->second(*entity);
				IO* io = nullptr;
				if (em.BaseClassInstance(*wrapper, io))
				{
					if (io->DynamiclySized() || io->PayloadSize() == payloadSize)
					{
						io->Load(in);
						payloadSize = 0;
					}
					else
					{
						// If the component is of the wrong size, log it
						std::stringstream ss;
						ss << "Unable to load component's '" << componentName << "' payload, size miss match. Expecting " << io->PayloadSize() << " Bytes, but recived " << payloadSize << " Bytes";
						Log(ss.str(), ComponentEngine::Warning);
					}
				}
			}
			else
			{
				// If we don't have the component log it
				std::stringstream ss;
				ss << "Unable to find component '" << componentName << "' for entity '" << entityName << "'";
				Log(ss.str(), ComponentEngine::Warning);
			}

			// Dump the invalid payload if it has not all bee read
			if (payloadSize > 0)
			{
				std::stringstream ss;
				ss << "Dumping " << payloadSize << " Bytes from input stream";
				Log(ss.str(), ComponentEngine::Warning);

				char* temp = new char[payloadSize];
				Common::Read(in, temp, payloadSize);
				delete temp;

			}
		}

		// If we did not load a transformation, attach a default one
		if (!entity->HasComponent<Transformation>())
		{
			Transformation::EntityHookDefault(*entity);
		}

		// Define the transformations parent
		entity->GetComponent<Transformation>().SetParent(parent);

		// Read in the child count
		unsigned int childrenCount;
		Common::Read(in, &childrenCount, sizeof(unsigned int));

		GetRendererMutex().unlock();
		m_logic_lock.unlock();
		// Loop through for all children and read them in
		for (int i = 0; i < childrenCount; i++)
		{
			createEntity(entity);
		}
	};
	// Loop through all top level entities and read them in
	for (int i = 0; i < topLevelEntityCount; i++)
	{
		createEntity(nullptr);
	}

	return true;
}

// Save to the current scene
void ComponentEngine::Engine::SaveScene()
{
	// Load the current scene file
	std::ofstream out(m_currentScene);
	EntityManager& em = GetEntityManager();

	{ // Add validation to file
		Common::Write(out, EngineName);
	}

	// Create a in-function, function for saving each entity
	std::function<void(Entity*)> saveEntity = [&](Entity* entity)
	{
		// Output entitys name
		Common::Write(out, entity->GetName());

		// Output the amount of components the entity has
		unsigned int componentCount = entity->GetComponentCount();
		Common::Write(out, &componentCount, sizeof(unsigned int));

		// Loop through each component
		entity->ForEach([&](BaseComponentWrapper& wrapper) {
			// Output Components name 
			Common::Write(out, wrapper.GetName());


			IO* io = nullptr;
			if (em.BaseClassInstance(wrapper, io))
			{
				// Get the payload size and write it to the file
				unsigned int payloadSize = io->PayloadSize();
				Common::Write(out, &payloadSize, sizeof(unsigned int));
				io->Save(out);
			}
			else
			{
				// Write nothing for payload size
				unsigned int payloadSize = 0;
				Common::Write(out, &payloadSize, sizeof(unsigned int));
			}
		});

		// Check to see if we have a transformation
		if (entity->HasComponent<Transformation>())
		{
			Transformation& trans = entity->GetComponent<Transformation>();

			// Output the amount of children the entity has
			unsigned int childrenCount = trans.GetChildren().size();
			Common::Write(out, &childrenCount, sizeof(unsigned int));

			// Save each child entity
			for (Transformation* child : trans.GetChildren())
			{
				saveEntity(child->GetEntity());
			}
		}
		else // If we have no transformation, set that we had 0 children
		{
			// Output the amount of children the entity has
			unsigned int childrenCount = 0;
			Common::Write(out, &childrenCount, sizeof(unsigned int));
		}

	};
	// Loop through all entities
	std::vector<Entity*> topLevelEntities;
	for (auto entity : em.GetEntitys())
	{
		// Check to see if we have a component and that it dose not have a parent, if so, save the top level parent
		if (entity->HasComponent<Transformation>() && entity->GetComponent<Transformation>().GetParent() == nullptr)
		{
			topLevelEntities.push_back(entity);
		}
	}

	// Output how many top level entities there are
	unsigned int entityCount = topLevelEntities.size();
	Common::Write(out, &entityCount, sizeof(unsigned int));
	// Loop through the entities and save them
	for (auto entity : topLevelEntities)
	{
		saveEntity(entity);
	}
	// Store into the file and close the stream
	out.flush();
	out.close();
}

// Get the renderer instance
VulkanRenderer * ComponentEngine::Engine::GetRenderer()
{
	return m_renderer;
}

// Get the cameras description pool
VulkanDescriptorPool * ComponentEngine::Engine::GetCameraPool()
{
	return m_camera_pool;
}

// Get the main cameras description set
VulkanDescriptorSet * ComponentEngine::Engine::GetCameraDescriptorSet()
{
	return m_camera_descriptor_set;
}

// Get the texture map descriptor pool
VulkanDescriptorPool * ComponentEngine::Engine::GetTextureMapsPool()
{
	return m_texture_maps_pool;
}

// Load a texture, if the texture is already loaded, get the texture instance
VulkanTextureBuffer* ComponentEngine::Engine::LoadTexture(std::string path)
{
	std::vector<unsigned char> image; //the raw pixels
	unsigned width;
	unsigned height;
	unsigned error = lodepng::decode(image, width, height, path);
	if (error)
	{
		std::stringstream ss;
		ss << "Error loading texture: " << lodepng_error_text(error) << " " << path << "("<< error << ")";
		Log(ss.str(), Error);
		return nullptr;
	}
	// Create and store the texture on the graphics card
	VulkanTextureBuffer* texture = m_renderer->CreateTextureBuffer(image.data(), VkFormat::VK_FORMAT_R8G8B8A8_UNORM, width, height);
	texture->SetData(BufferSlot::Primary);
	{
		std::stringstream ss;
		ss << "Loaded texture" << path;
		Log(ss.str(), Info);
	}
	return texture;
}

VulkanTextureBuffer * ComponentEngine::Engine::LoadTexture(unsigned int width, unsigned int height, char * data)
{
	// Create and store the texture on the graphics card
	VulkanTextureBuffer* texture = m_renderer->CreateTextureBuffer(data, VkFormat::VK_FORMAT_R8G8B8A8_UNORM, width, height);
	texture->SetData(BufferSlot::Primary);
	{
		std::stringstream ss;
		ss << "Loaded texture";
		Log(ss.str(), Info);
	}
	return texture;
}

// Get the current scene name and directory
std::string ComponentEngine::Engine::GetCurrentScene()
{
	return m_currentScene;
}

// Get the current scene directory
std::string ComponentEngine::Engine::GetCurrentSceneDirectory()
{
	return m_currentSceneDirectory;
}

// Get the mutex that locks all logic calls
ordered_lock& ComponentEngine::Engine::GetLogicMutex()
{
	return m_logic_lock;
}

// Get the mutex that locks all render calls
ordered_lock& ComponentEngine::Engine::GetRendererMutex()
{
	return m_renderer_thread;
}

// Get the thread manager instance
ThreadManager * ComponentEngine::Engine::GetThreadManager()
{
	return m_threadManager;
}

// Define what camera should be the primary camera
void ComponentEngine::Engine::SetCamera(Camera* camera)
{
	GetRendererMutex().lock();
	// Set the current camera
	m_main_camera = camera;
	// Define to the camera descriptor, what is the current camera
	m_camera_descriptor_set->AttachBuffer(0, camera->GetCameraBuffer());
	m_camera_descriptor_set->UpdateSet();
	camera->UpdateProjection();
	if (m_standardRTConfigSet != nullptr)
	{
		m_standardRTConfigSet->AttachBuffer(0, { m_top_level_acceleration->GetDescriptorAcceleration() });
		m_standardRTConfigSet->AttachBuffer(1, { m_renderer->GetSwapchain()->GetRayTraceStagingBuffer() });
		m_standardRTConfigSet->AttachBuffer(2, m_main_camera->GetCameraBuffer());
		m_standardRTConfigSet->UpdateSet();
		// Rebuild the render pass and raytracing dependences
		Rebuild();
	}
	GetRendererMutex().unlock();
}

// Check to see if we have a custom camera defined
bool ComponentEngine::Engine::HasCamera()
{
	return m_main_camera != m_default_camera;
}

// Get the current main camera even if it is the default one
Camera* ComponentEngine::Engine::GetMainCamera()
{
	return m_main_camera;
}

// Get the default camera
Camera* ComponentEngine::Engine::GetDefaultCamera()
{
	return m_default_camera;;
}

// Get the current window handle
NativeWindowHandle* ComponentEngine::Engine::GetWindowHandle()
{
	return m_window_handle;
}

// Define a new component and how it should be initialized
void ComponentEngine::Engine::RegisterComponentBase(std::string name, BaseComponentWrapper*(*default_initilizer)(enteez::Entity &entity))
{
	m_component_register[name] = default_initilizer;
}

// Define that the mouse should be locked to the window
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

// Set the engines current state
PlayState ComponentEngine::Engine::GetPlayState()
{
	return m_play_state;
}

// Get the current physics world
PhysicsWorld * ComponentEngine::Engine::GetPhysicsWorld()
{
	return m_physicsWorld;
}

// Get a engine mutex
std::mutex & ComponentEngine::Engine::GetLock(EngineLock lock)
{
	return m_locks[(int)lock];
}

// Get all console messages
std::vector<ConsoleMessage>& ComponentEngine::Engine::GetConsoleMessages()
{
	return m_console;
}

// Get all registered components
std::map<std::string, BaseComponentWrapper*(*)(enteez::Entity& entity)> ComponentEngine::Engine::GetComponentRegister()
{
	return m_component_register;
}

// Set a engine flag
void ComponentEngine::Engine::SetFlag(int flags)
{
	m_flags |= flags;
}

// Return the UI manager instance
UIManager * ComponentEngine::Engine::GetUIManager()
{
	return m_ui;
}

// Get the global vertex buffer that stores all primary vertex data
VulkanVertexBuffer * ComponentEngine::Engine::GetGlobalVertexBufer()
{
	return m_vertexBuffer;
}

// Get the global index buffer that stores all primary index data
VulkanIndexBuffer * ComponentEngine::Engine::GetGlobalIndexBuffer()
{
	return m_indexBuffer;
}

// Get the uniform buffer that stored all material data
VulkanUniformBuffer * ComponentEngine::Engine::GetMaterialBuffer()
{
	return m_materialbuffer;
}

// Get the local vertex array data
std::vector<MeshVertex>& ComponentEngine::Engine::GetGlobalVertexArray()
{
	return m_all_vertexs;
}

// Get the local index array data
std::vector<uint32_t>& ComponentEngine::Engine::GetGlobalIndexArray()
{
	return m_all_indexs;
}

// Get the local material array data
std::vector<MatrialObj>& ComponentEngine::Engine::GetGlobalMaterialArray()
{
	return m_materials;
}

// Get the local texture descriptor array data
std::vector<VkDescriptorImageInfo>& ComponentEngine::Engine::GetTextureDescriptors()
{
	return m_texture_descriptors;
}

// Get the local texture buffer array
std::vector<VulkanTextureBuffer*>& ComponentEngine::Engine::GetTextures()
{
	return m_textures;
}

// Get the allocation pool for model positions
VulkanBufferPool * ComponentEngine::Engine::GetPositionBufferPool()
{
	return m_position_buffer_pool;
}

VulkanBufferPool * ComponentEngine::Engine::GetLightBufferPool()
{
	return m_light_buffer_pool;
}

// Get the raytracing top level acceleration structure
VulkanAcceleration * ComponentEngine::Engine::GetTopLevelAS()
{
	return m_top_level_acceleration;
}

VulkanRenderPass * ComponentEngine::Engine::GetRenderPass()
{
	return m_render_pass;
}

// Get the total used vertex size
unsigned int & ComponentEngine::Engine::GetUsedVertex()
{
	return m_used_vertex;
}

// Get the total used index size
unsigned int & ComponentEngine::Engine::GetUsedIndex()
{
	return m_used_index;
}

// Get the total used materials size
unsigned int & ComponentEngine::Engine::GetUsedMaterials()
{
	return m_used_materials;
}

// Rebuild the offset array that defined where models information is defined
void ComponentEngine::Engine::RebuildOffsetAllocation()
{
	unsigned int index = 0;
	// Loop through all model pools
	for (auto& mp : m_top_level_acceleration->GetModelPools())
	{
		// Define where the index and vertex offsets for the model are
		unsigned int index_offset = mp.model_pool->GetIndexOffset();
		unsigned int vertex_offset = mp.model_pool->GetVertexOffset();
		// Loop through each model instance
		for (auto& model : mp.model_pool->GetModels())
		{
			// Define all the ofsets
			m_offset_allocation_array[index].index = index_offset;
			m_offset_allocation_array[index].vertex = vertex_offset;
			m_offset_allocation_array[index].position = mp.model_pool->GetModelBufferOffset(model.second, 0);
			index++;
		}
	}
	// Push the data to the GPU
	m_offset_allocation_array_buffer->SetData(BufferSlot::Primary);
}

// Update all buffers and offsets that are needed for RTX
void ComponentEngine::Engine::UpdateAccelerationDependancys()
{
	GetRendererMutex().lock();
	// Update offsets
	RebuildOffsetAllocation();

	// Push all the data to the GPU
	m_vertexBuffer->SetData(BufferSlot::Primary);
	m_indexBuffer->SetData(BufferSlot::Primary);
	m_materialbuffer->SetData(BufferSlot::Primary);
	m_light_buffer->SetData(BufferSlot::Primary);
	m_model_position_buffer->SetData(BufferSlot::Primary);
	m_offset_allocation_array_buffer->SetData(BufferSlot::Primary);

	m_top_level_acceleration->Build();

	{
		m_standardRTConfigSet->AttachBuffer(0, { m_top_level_acceleration->GetDescriptorAcceleration() });
		m_standardRTConfigSet->AttachBuffer(1, { m_renderer->GetSwapchain()->GetRayTraceStagingBuffer() });
		m_standardRTConfigSet->AttachBuffer(2, m_main_camera->GetCameraBuffer());
		m_standardRTConfigSet->UpdateSet();
	}

	{
		m_RTModelPoolSet->AttachBuffer(0, m_vertexBuffer);
		m_RTModelPoolSet->AttachBuffer(1, m_indexBuffer);
		m_RTModelPoolSet->AttachBuffer(2, m_materialbuffer);
		m_RTModelPoolSet->AttachBuffer(3, m_light_buffer);

		m_RTModelPoolSet->UpdateSet();
	}

	{
		if (m_texture_descriptors.size() > 0)
		{
			m_RTTexturePoolSet->AttachBuffer(0, m_texture_descriptors);
			m_RTTexturePoolSet->UpdateSet();
		}
	}

	{
		m_RTModelInstanceSet->AttachBuffer(0, m_model_position_buffer);
		m_RTModelInstanceSet->AttachBuffer(1, m_offset_allocation_array_buffer);
		m_RTModelInstanceSet->UpdateSet();
	}


	m_render_pass->RebuildCommandBuffers();


	GetRendererMutex().unlock();
}

// Get the uniform buffer that stores all position buffers
VulkanUniformBuffer * ComponentEngine::Engine::GetModelPositionBuffer()
{
	return m_model_position_buffer;
}

// Define a new miss shader for the pipeline
unsigned int ComponentEngine::Engine::AddMissShader(const char * missShader, const std::vector<unsigned int>& constants)
{
	// Pre construct the renderer shader input format
	m_miss_groups.push_back({ VK_SHADER_STAGE_MISS_BIT_NV, missShader });
	m_miss_groups_constants.push_back(constants);
	return m_miss_groups.size() - 1;
}

// Define a new hit group for the raytracing pipeline
unsigned int ComponentEngine::Engine::AddHitShaderPipeline(HitShaderPipeline pipeline, const std::vector<unsigned int>& constants)
{
	m_pipelines.push_back(pipeline);
	m_pipelines_constants.push_back(constants);
	return m_pipelines.size() - 1;
}

// Get all hit shader instances
std::vector<HitShaderPipeline>& ComponentEngine::Engine::GetHitShaderPipelines()
{
	return m_pipelines;
}

// Create a SDL window instance
void ComponentEngine::Engine::InitWindow()
{
	m_window = SDL_CreateWindow(
		m_title,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		m_width, m_height,
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
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

// Poll the SDL window for updates
void ComponentEngine::Engine::UpdateWindow()
{
	ImGuiIO& io = ImGui::GetIO();
	SDL_Event event;
	m_mousePosDelta = glm::vec2();
	// Loop through each window update
	while (SDL_PollEvent(&event) > 0)
	{
		switch (event.type)
		{
		// Was the window closed
		case SDL_QUIT: 
			if (Running())
			{
				RequestStop();
				return;
			}
			break;
		// Was a window event called
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
			// Was a mouse button pressed or released
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT) io.MouseDown[0] = event.button.state == SDL_PRESSED;
				if (event.button.button == SDL_BUTTON_RIGHT) io.MouseDown[1] = event.button.state == SDL_PRESSED;
				if (event.button.button == SDL_BUTTON_MIDDLE) io.MouseDown[2] = event.button.state == SDL_PRESSED;
				//io.MouseDown[0] = true;
				break;
			// Was a mouse moved
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
			// Was a key typed
			case SDL_TEXTINPUT:
			{
				io.AddInputCharactersUTF8(event.text.text);
				break;
			}
			// Was a mouse wheel rotated
			case SDL_MOUSEWHEEL:
			{
				if (event.wheel.x > 0) io.MouseWheelH += 1;
				if (event.wheel.x < 0) io.MouseWheelH -= 1;
				if (event.wheel.y > 0) io.MouseWheel += 1;
				if (event.wheel.y < 0) io.MouseWheel -= 1;
				break;
			}
			// Was a keyboard key pressed or released
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

// Destroy the SDL window instance
void ComponentEngine::Engine::DeInitWindow()
{
	SDL_RestoreWindow(m_window);
	SDL_DestroyWindow(m_window);
	delete m_window_handle;
}

// Define all built in component definitions
void ComponentEngine::Engine::InitEnteeZ()
{
	// Define what base classes each one of these components have
	RegisterBase<Transformation, MsgRecive<TransformationPtrRedirect>, UI, IO>();
	RegisterBase<RendererComponent, UI, MsgRecive<OnComponentEnter<Mesh>>>();
	RegisterBase<Mesh, MsgRecive<RenderStatus>, UI, IO, Logic, MsgRecive<OnComponentEnter<Transformation>>, MsgRecive<OnComponentExit<Transformation>>>();
	RegisterBase<Camera, Logic, UI, TransferBuffers>(); 
	RegisterBase<Rigidbody,
		MsgRecive<TransformationChange>, MsgRecive<OnComponentEnter<ICollisionShape>>,
		MsgRecive<CollisionRecording>, MsgRecive<CollisionEvent>,
		MsgRecive<OnComponentChange<ICollisionShape>>, MsgRecive<OnComponentExit<ICollisionShape>>,
		MsgRecive<OnComponentExit<Rigidbody>>, Logic, IO, UI
	>();
	RegisterBase<BoxCollision, ICollisionShape, Logic, IO, UI>();
	RegisterBase<SphereCollision, ICollisionShape, Logic, IO, UI>();
	RegisterBase<Light, Logic, IO, UI>();
}

// DeInit EnteeZ
void ComponentEngine::Engine::DeInitEnteeZ()
{

}

void ComponentEngine::Engine::RebuildRaytracePipeline()
{
	if (m_default_raytrace != nullptr)
	{
		m_render_pass->RemoveGraphicsPipeline(m_default_raytrace);
		delete m_default_raytrace;
	}

	// Shaders such as miss and ray gen
	std::vector<std::pair<VkShaderStageFlagBits, const char*>> primaryShaders;
	// Resize to fir all miss shader + the ray gen shader
	primaryShaders.resize(1 + m_miss_groups.size());
	// Bind the ray gen shader
	primaryShaders[0] = { VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV,		"../Shaders/Raytrace/Gen/rgen.spv" };
	// Copy over the miss shaders
	memcpy(primaryShaders.data() + 1, m_miss_groups.data(), m_miss_groups.size() * sizeof(std::pair<VkShaderStageFlagBits, const char*>));
	
	std::vector<std::map<VkShaderStageFlagBits, const char*>> hitgroups;
	hitgroups.resize(m_pipelines.size());
	for (int i = 0; i < hitgroups.size(); i++)
	{
		hitgroups[i] = m_pipelines[i].hitgroup;
	}


	// Create the raytracing pipeline
	m_default_raytrace = m_renderer->CreateRaytracePipeline(
		m_render_pass, // Current render pass
		primaryShaders, // Ray gen + miss shaders
		hitgroups // All defined hit groups
	);





	int groupID = 0;
	// Ray generation entry point
	m_default_raytrace->AddRayGenerationProgram(groupID++, {}, { m_max_recursions, m_general_miss_shader });

	// Define all miss shaders
	for (int i = 1; i < primaryShaders.size(); i++)
	{
		m_default_raytrace->AddMissProgram(groupID++, {}, m_miss_groups_constants[i-1]);
	}
	// Define all hit groups
	for (int i = 0; i < hitgroups.size(); i++)
	{
		m_default_raytrace->AddHitGroup(groupID++, {}, m_pipelines_constants[i]);
	}


	m_default_raytrace->SetMaxRecursionDepth(m_max_recursions);

	m_default_raytrace->AttachVertexBinding({
		VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX,
		{
			{ 0, VkFormat::VK_FORMAT_R32G32B32_SFLOAT,offsetof(MeshVertex,pos) },
			{ 1, VkFormat::VK_FORMAT_R32G32_SFLOAT,offsetof(MeshVertex,texCoord) },
			{ 2, VkFormat::VK_FORMAT_R32G32B32_SFLOAT,offsetof(MeshVertex,nrm) },
			{ 3, VkFormat::VK_FORMAT_R32G32B32_SFLOAT,offsetof(MeshVertex,color) },
		},
		sizeof(MeshVertex),
		0
		});

	{ // Descriptor sets and pools

		m_default_raytrace->AttachDescriptorPool(0, standardRTConfigPool);
		m_default_raytrace->AttachDescriptorSet(0, m_standardRTConfigSet);

		m_default_raytrace->AttachDescriptorPool(1, RTModelPool);
		m_default_raytrace->AttachDescriptorSet(1, m_RTModelPoolSet);

		m_default_raytrace->AttachDescriptorPool(2, RTTextureDescriptorPool);
		m_default_raytrace->AttachDescriptorSet(2, m_RTTexturePoolSet);

		m_default_raytrace->AttachDescriptorPool(3, RTModelInstancePool);
		m_default_raytrace->AttachDescriptorSet(3, m_RTModelInstanceSet);
	}
	m_default_raytrace->Build();
	m_render_pass->AttachGraphicsPipeline(m_default_raytrace);

}

// Create the renderer instance and all required components
void ComponentEngine::Engine::InitRenderer()
{
	// Create a instance of the renderer
	m_renderer = new VulkanRenderer();
	m_renderer->Start(m_window_handle, VulkanFlags::Raytrace);

	// Get the swapchain and render pass instances we need
	m_swapchain = m_renderer->GetSwapchain();
	m_render_pass = m_renderer->CreateRenderPass((m_flags & EngineFlags::ReleaseBuild) == EngineFlags::ReleaseBuild ? 1 : 2);


	// If the rendering was not fully created, error out
	assert(m_renderer != nullptr && "Error, renderer instance could not be created");

	m_default_camera = new Camera();


	// Create camera pool
	// This is a layout for the camera input data
	m_camera_pool = m_renderer->CreateDescriptorPool({
		m_renderer->CreateDescriptor(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, 0),
		});


	// Create camera descriptor set from the tempalte
	m_camera_descriptor_set = m_camera_pool->CreateDescriptorSet();

	SetCamera(m_default_camera);



	{
		// Load a default white texture
		char imageData[4] = { 255,255,255,255 };
		VulkanTextureBuffer* texture = LoadTexture(1, 1, imageData);
		m_textures.push_back(texture);
		m_texture_descriptors.push_back(texture->GetDescriptorImageInfo(BufferSlot::Primary));
	}
	{
		// Load a default black texture
		char imageData[4] = { 0,0,0,255 };
		VulkanTextureBuffer* texture = LoadTexture(1, 1, imageData);
		m_textures.push_back(texture);
		m_texture_descriptors.push_back(texture->GetDescriptorImageInfo(BufferSlot::Primary));
	}

	{
		



		m_top_level_acceleration = m_renderer->CreateAcceleration();

		m_top_level_acceleration->Build();




		{
			standardRTConfigPool = m_renderer->CreateDescriptorPool({
				m_renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 0),
				m_renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV, 1),
				m_renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_NV, 2),
				});

			m_standardRTConfigSet = standardRTConfigPool->CreateDescriptorSet();

			m_standardRTConfigSet->AttachBuffer(0, { m_top_level_acceleration->GetDescriptorAcceleration() });
			m_standardRTConfigSet->AttachBuffer(1, { m_swapchain->GetRayTraceStagingBuffer() });
			m_standardRTConfigSet->AttachBuffer(2, m_main_camera->GetCameraBuffer());
			m_standardRTConfigSet->UpdateSet();

		}


		// Create all needed buffers for vertex, index, lighting and materials
		m_vertexBuffer = m_renderer->CreateVertexBuffer(m_all_vertexs.data(), sizeof(MeshVertex), m_all_vertexs.size());
		m_indexBuffer = m_renderer->CreateIndexBuffer(m_all_indexs.data(), sizeof(uint32_t), m_all_indexs.size());

		m_lights.resize(100);

		m_light_buffer = m_renderer->CreateUniformBuffer(m_lights.data(), BufferChain::Double, sizeof(LightData), m_lights.size(), true);
		m_light_buffer->SetData(BufferSlot::Primary);

		m_light_buffer_pool = new VulkanBufferPool(m_light_buffer);


		m_materials.resize(m_max_materials);

		m_materialbuffer = m_renderer->CreateUniformBuffer(m_materials.data(), BufferChain::Single, sizeof(MatrialObj), m_materials.size(), true);
		m_materialbuffer->SetData(BufferSlot::Primary);


		{
			RTModelPool = m_renderer->CreateDescriptorPool({
				m_renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 0),
				m_renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 1),
				m_renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 2),
				m_renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 3),
				});

			m_RTModelPoolSet = RTModelPool->CreateDescriptorSet();

			m_RTModelPoolSet->AttachBuffer(0, m_vertexBuffer);
			m_RTModelPoolSet->AttachBuffer(1, m_indexBuffer);
			m_RTModelPoolSet->AttachBuffer(2, m_materialbuffer);
			m_RTModelPoolSet->AttachBuffer(3, m_light_buffer);


			m_RTModelPoolSet->UpdateSet();
		}

		{
			RTTextureDescriptorPool = m_renderer->CreateDescriptorPool({
				m_renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 0, m_max_texture_descriptors),
				});

			m_RTTexturePoolSet = RTTextureDescriptorPool->CreateDescriptorSet();

			if (m_texture_descriptors.size() > 0)
			{
				m_RTTexturePoolSet->AttachBuffer(0, m_texture_descriptors);
				m_RTTexturePoolSet->UpdateSet();
			}
		}

		// Create buffers for model positions, position allocation pool and offsets
		m_model_position_array = new glm::mat4[1000];
		m_model_position_buffer = m_renderer->CreateUniformBuffer(m_model_position_array, BufferChain::Double, sizeof(glm::mat4), 1000, true);

		m_position_buffer_pool = new VulkanBufferPool(m_model_position_buffer);



		m_offset_allocation_array = new ModelOffsets[1000];
		m_offset_allocation_array_buffer = m_renderer->CreateUniformBuffer(m_offset_allocation_array, BufferChain::Single, sizeof(ModelOffsets), 1000, true);


		{
			RTModelInstancePool = m_renderer->CreateDescriptorPool({
				m_renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 0),
				m_renderer->CreateDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 1),
				});


			m_RTModelInstanceSet = static_cast<VulkanDescriptorSet*>(RTModelInstancePool->CreateDescriptorSet());

			m_RTModelInstanceSet->AttachBuffer(0, m_model_position_buffer);
			m_RTModelInstanceSet->AttachBuffer(1, m_offset_allocation_array_buffer);


			m_RTModelInstanceSet->UpdateSet();

		}
	}
	{// Define default hit and miss groups

		
		m_general_miss_shader = AddMissShader("../Shaders/Raytrace/Default/Miss/rmiss.spv", {});


		{ // Default Textured PBR - No lights
			HitShaderPipeline defaultHitgroup{ "PBR - No Light",
			{
				{ VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, "../Shaders/Raytrace/Textured/rchit.spv" },
			},
			true
			};
			m_default_textured_pbr_shader = AddHitShaderPipeline(defaultHitgroup, {});
		}

		{ // Default Textured PBR


			// Add the PBR shadow miss shader
			unsigned int missShader = AddMissShader("../Shaders/Raytrace/PBR/Miss/rmiss.spv", {});



			{ // Primary PBR hitgroup
				HitShaderPipeline defaultHitgroup{ "PBR",
				{
					{ VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, "../Shaders/Raytrace/PBR/Hitgroups/rchit.spv" },
				},
				true
				};
				AddHitShaderPipeline(defaultHitgroup, { missShader, 0 });
			}


			{

				// Secondary shadow fall through hitgroup 
				HitShaderPipeline shadowHitHitgroup{ "Default Textured PBR Shadow Fall through",
				{
				},
				false
				};
				AddHitShaderPipeline(shadowHitHitgroup, {});
			}
		}
	}
	RebuildRaytracePipeline();
}

// Destroy the current renderer instance
void ComponentEngine::Engine::DeInitRenderer()
{
	// Destroy ui instance
	DeInitImGUI();
	// delete all textures
	for (auto it : m_textures)
	{
		delete it;
	}
	// Destroy all renderer components
	delete m_default_camera;
	m_default_camera = nullptr;
	delete m_camera_pool;
	m_camera_pool = nullptr;
	delete m_texture_maps_pool;
	m_texture_maps_pool = nullptr;
	m_renderer->Stop();
	delete m_renderer;
	m_renderer = nullptr;
}

// Define the EnteeZ component hooks
void ComponentEngine::Engine::InitComponentHooks()
{
	RegisterComponentBase("Transformation", Transformation::EntityHookDefault);
	RegisterComponentBase("Mesh", Mesh::EntityHookDefault);
	RegisterComponentBase("Renderer", RendererComponent::EntityHookDefault);
	RegisterComponentBase("Camera", Camera::EntityHookDefault);
	RegisterComponentBase("Rigidbody", Rigidbody::EntityHookDefault);
	RegisterComponentBase("Box Collision", BoxCollision::EntityHookDefault);
	RegisterComponentBase("Sphere Collision", SphereCollision::EntityHookDefault);
	RegisterComponentBase("Light", Light::EntityHookDefault);
}

// Create the physics world instance
void ComponentEngine::Engine::InitPhysicsWorld()
{
	m_physicsWorld = new PhysicsWorld(this);
}

// Destroy physics world
void ComponentEngine::Engine::DeInitPhysicsWorld()
{
	delete m_physicsWorld;
}

// Create a instance of imgui
void ComponentEngine::Engine::InitImGUI()
{
	// Create the ui manager instance
	m_ui = new UIManager(this);

	// Are we in the editor?
	bool editor = !(m_flags & EngineFlags::ReleaseBuild) == EngineFlags::ReleaseBuild;
	if (editor)
	{
		// Define all the ui windows
		m_ui->AddElement(new Console("Console", ImGuiWindowFlags_NoCollapse, PlayState::Editor));
		m_ui->AddElement(new ThreadingWindow("Threading", ImGuiWindowFlags_NoCollapse, PlayState::Editor));
		m_ui->AddElement(new ComponentHierarchy("ComponentHierarchy", ImGuiWindowFlags_NoCollapse, PlayState::Editor));
		m_ui->AddElement(new Explorer("Explorer", ImGuiWindowFlags_NoCollapse, PlayState::Editor));
		m_ui->AddElement(new SceneHierarchy("SceneHierarchy", ImGuiWindowFlags_NoCollapse, PlayState::Editor));
		m_ui->AddElement(new EditorState("EditorState", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking, PlayState::Editor | PlayState::Play));

		m_ui->AddElement(new SceneWindow("SceneWindow", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar, PlayState::Editor));

		m_ui->AddElement(new PlayWindow("PlayWindow", ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus, PlayState::Play | PlayState::Release));




		// Define the autofills
		static std::string filenameAutofill = "Scene.bin";
		static std::string defaultSavePath = "../Scene.bin";
		static unsigned int maxFileLength = 200;
		static char* saveAsStream = new char[maxFileLength + 1];
		// Define all default menu elements for the engine
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

		// Add the window open/close menu drop down
		m_window_dropdown = new MenuElement("Window", std::vector<MenuElement*>{});

		// Loop through all currently added windows and append them to the toggle list
		for (auto& window : m_ui->GetWindowBases())
		{
			m_window_dropdown->AddChild(new MenuElement(window->GetTitle(), [&]
			{
				// Toggle the windows open status
				window->Open(!window->IsOpen());
			}));
		}
		// Append the window toggle element to the UI
		m_ui->AddMenuElement(m_window_dropdown);
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

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.39f, 0.39f, 0.39f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);






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
		m_renderer->CreateDescriptor(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, 0),
		});
	m_imgui.m_screen_res_set = m_imgui.m_screen_res_pool->CreateDescriptorSet();
	m_imgui.m_screen_res_set->AttachBuffer(0, m_imgui.m_screen_res_buffer);
	m_imgui.m_screen_res_set->UpdateSet();

	// Create font texture
	unsigned char* font_data;
	int font_width, font_height;
	io.Fonts->GetTexDataAsRGBA32(&font_data, &font_width, &font_height);
	m_imgui.m_font_texture = m_renderer->CreateTextureBuffer(font_data, VkFormat::VK_FORMAT_R8G8B8A8_UNORM, font_width, font_height);

	io.Fonts->TexID = (ImTextureID)1;
	m_imgui.texture_descriptors.push_back(m_imgui.m_font_texture->GetDescriptorImageInfo(BufferSlot::Primary));


	m_imgui.m_font_texture_pool = m_renderer->CreateDescriptorPool({
		m_renderer->CreateDescriptor(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, 0 ,m_imgui.texture_descriptors.size()),
		});
	m_imgui.m_texture_descriptor_set = m_imgui.m_font_texture_pool->CreateDescriptorSet();
	m_imgui.m_texture_descriptor_set->AttachBuffer(0, m_imgui.texture_descriptors);
	m_imgui.m_texture_descriptor_set->UpdateSet();

	// Setup ImGUI Pipeline
	if (editor)
	{
		m_imgui.m_imgui_pipeline = m_renderer->CreateGraphicsPipeline(m_render_pass, {
			{ VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, "../Shaders/ImGUI/vert.spv" },
			{ VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, "../Shaders/ImGUI/Editor/frag.spv" }
			});
		// Attach screen buffer
		//
		m_imgui.m_imgui_pipeline->AttachDescriptorPool(0, m_render_pass->GetCombinedImageSamplerReadPool());

		// Attach font buffer
		m_imgui.m_imgui_pipeline->AttachDescriptorPool(2, m_imgui.m_font_texture_pool);
		m_imgui.m_imgui_pipeline->AttachDescriptorSet(2, m_imgui.m_texture_descriptor_set);
	}
	else
	{
		m_imgui.m_imgui_pipeline = m_renderer->CreateGraphicsPipeline(m_render_pass, {
			{ VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, "../Shaders/ImGUI/vert.spv" },
			{ VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, "../Shaders/ImGUI/Release/frag.spv" }
			});
		// Attach font buffer
		m_imgui.m_imgui_pipeline->AttachDescriptorPool(0, m_imgui.m_font_texture_pool);
		m_imgui.m_imgui_pipeline->AttachDescriptorSet(0, m_imgui.m_texture_descriptor_set);
	}

	// Attach screen buffer
	m_imgui.m_imgui_pipeline->AttachDescriptorPool(1, m_imgui.m_screen_res_pool);
	m_imgui.m_imgui_pipeline->AttachDescriptorSet(1, m_imgui.m_screen_res_set);

	{
		VulkanGraphicsPipelineConfig& config = m_imgui.m_imgui_pipeline->GetGraphicsPipelineConfig();
		config.input = COMBINED_IMAGE_SAMPLER;
		config.subpass = editor ? 1 : 0;
		config.culling = VK_CULL_MODE_NONE;
		config.use_depth_stencil = false;
	}

	m_imgui.m_imgui_pipeline->AttachVertexBinding({
		VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX,
		{
			{ 0, VkFormat::VK_FORMAT_R32G32_SFLOAT,offsetof(ImDrawVert,pos) },
			{ 1, VkFormat::VK_FORMAT_R32G32_SFLOAT,offsetof(ImDrawVert,uv) },
			{ 2, VkFormat::VK_FORMAT_R8G8B8A8_UNORM,offsetof(ImDrawVert,col) },
		},
		sizeof(ImDrawVert),
		0
		});

	// Texture ID and layer
	m_imgui.m_imgui_pipeline->AttachVertexBinding({
		VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE,
		{
			{ 3, VkFormat::VK_FORMAT_R32_SINT,0 }, // Texture ID
		},
		sizeof(glm::mat4),
		1
		});
	

	// Build Pipeline
	m_imgui.m_imgui_pipeline->Build();

	m_render_pass->AttachGraphicsPipeline(m_imgui.m_imgui_pipeline);

	const int temp_vert_max = 30000;
	const int temp_in_max = 40000;

	m_imgui.m_vertex_data = new ImDrawVert[temp_vert_max];
	m_imgui.m_vertex_buffer = m_renderer->CreateVertexBuffer(m_imgui.m_vertex_data, sizeof(ImDrawVert), temp_vert_max);

	m_imgui.m_index_data = new uint32_t[temp_in_max];
	m_imgui.m_index_buffer = m_renderer->CreateIndexBuffer(m_imgui.m_index_data, sizeof(uint32_t), temp_in_max);


	// Setup model instance
	m_imgui.model_pool = m_renderer->CreateModelPool(m_imgui.m_vertex_buffer, 0, 0, m_imgui.m_index_buffer, 0, 0, ModelPoolUsage::MultiMesh);

	// Define a buffer for the texture ids
	m_imgui.draw_group_textures_buffer = m_renderer->CreateUniformBuffer(m_imgui.draw_group_textures, BufferChain::Single, sizeof(glm::mat4), 1000, true);

	// Pass the data to the GPU
	m_imgui.draw_group_textures_buffer->SetData(BufferSlot::Primary);

	// Create a allocation pool for the textures
	m_imgui.draw_group_textures_buffer_pool = new VulkanBufferPool(m_imgui.draw_group_textures_buffer);

	m_imgui.model_pool->AttachBufferPool(0, m_imgui.draw_group_textures_buffer_pool);

	m_imgui.m_imgui_pipeline->AttachModelPool(m_imgui.model_pool);

	m_imgui.model_pool->AllowCustomScissors(true);
}

// Update the ui manager
void ComponentEngine::Engine::UpdateImGUI()
{
	m_logic_lock.lock();
	GetRendererMutex().lock();

	// Render all imgui menu components
	m_ui->Render();



	ImDrawData* imDrawData = ImGui::GetDrawData();

	ImVec2 clip_off = imDrawData->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = imDrawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
	int fb_width = (int)(imDrawData->DisplaySize.x * imDrawData->FramebufferScale.x);
	int fb_height = (int)(imDrawData->DisplaySize.y * imDrawData->FramebufferScale.y);

	ImDrawVert* temp_vertex_data = m_imgui.m_vertex_data;
	unsigned int* temp_index_data = m_imgui.m_index_data;
	unsigned int index_count = 0;
	unsigned int vertex_count = 0;
	int drawGroup = 0;
	for (int n = 0; n < imDrawData->CmdListsCount; n++)
	{

		const ImDrawList* cmd_list = imDrawData->CmdLists[n];

		memcpy(temp_vertex_data, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		//memcpy(temp_index_data, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(unsigned int));

		// Loop through and manually add a offset to the index's so they can all be rendered in one render pass
		for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
		{
			temp_vertex_data[i] = cmd_list->VtxBuffer.Data[i];
		}
		for (int i = 0; i < cmd_list->IdxBuffer.Size; i++)
		{
			temp_index_data[i] = cmd_list->IdxBuffer.Data[i];
		}

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];


			
			// Generate the clip area
			ImVec4 clip_rect;
			clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
			clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
			clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
			clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

			// If the object is out of the scissor, ignore it
			if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
			{
				// Negative offsets are illegal for vkCmdSetScissor
				if (clip_rect.x < 0.0f)
					clip_rect.x = 0.0f;
				if (clip_rect.y < 0.0f)
					clip_rect.y = 0.0f;


				// If we do not have enough reserved models for imGui, create a new one
				if (drawGroup >= m_imgui.draw_group.size())
				{
					m_imgui.draw_group.push_back(m_imgui.model_pool->CreateModel(pcmd->VtxOffset + vertex_count, pcmd->IdxOffset + index_count, pcmd->ElemCount));
				}
				else
				{
					m_imgui.draw_group[drawGroup]->SetIndexOffset(pcmd->IdxOffset + index_count);
					m_imgui.draw_group[drawGroup]->SetVertexOffset(pcmd->VtxOffset + vertex_count);
					m_imgui.draw_group[drawGroup]->SetIndexSize(pcmd->ElemCount);
				}
				// Define the texture for the imgui render call
				m_imgui.draw_group[drawGroup]->SetData(0, (int)pcmd->TextureId);


				// Generate a vulkan friendly scissor area
				VkRect2D scissor;
				scissor.offset.x = (int32_t)(clip_rect.x);
				scissor.offset.y = (int32_t)(clip_rect.y);
				scissor.extent.width = (uint32_t)(clip_rect.z - clip_rect.x);
				scissor.extent.height = (uint32_t)(clip_rect.w - clip_rect.y);
				// Add the scissor to the model
				m_imgui.draw_group[drawGroup]->SetScissor(scissor);
				drawGroup++;
			}


		}


		temp_vertex_data += cmd_list->VtxBuffer.Size;
		temp_index_data += cmd_list->IdxBuffer.Size;

		vertex_count += cmd_list->VtxBuffer.Size;
		index_count += cmd_list->IdxBuffer.Size;
	}

	for (int i = m_imgui.draw_group.size() - 1; i >= drawGroup; i--)
	{
		m_imgui.draw_group[i]->SetIndexOffset(0);
		m_imgui.draw_group[i]->SetVertexOffset(0);
		m_imgui.draw_group[i]->SetIndexSize(0);
		m_imgui.draw_group[i]->ResetScissor();
	}

	m_imgui.draw_group_textures_buffer->SetData(BufferSlot::Primary);
	m_imgui.model_pool->Update();

	if (IsRunning())
	{
		m_imgui.m_vertex_buffer->SetData(BufferSlot::Primary);
		m_imgui.m_index_buffer->SetData(BufferSlot::Primary);
	}

	m_render_pass->RebuildCommandBuffers();

	GetRendererMutex().unlock();
	m_logic_lock.unlock();
}

// Destroy the instance of imgui
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
	delete m_ui;
}

// Request that the engine should stop
void ComponentEngine::Engine::RequestStop()
{
	m_request_stop = true;
}

// Request that the engine should switch threading mode
void ComponentEngine::Engine::RequestToggleThreading()
{
	m_request_toggle_threading = true;
}

// Are we currently threading
bool ComponentEngine::Engine::Threading()
{
	return m_threading;
}

// Request that the engine should switch threading mode
void ComponentEngine::Engine::ToggleThreading()
{
	m_threading = !m_threading;
	m_threadManager->ChangeMode(m_threading ? ThreadMode::Threading : ThreadMode::Joined);
}

// Set the current scene path
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
