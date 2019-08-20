#pragma once

#include <EnteeZ\EnteeZ.hpp>

#include <renderer\vulkan\VulkanRenderer.hpp>
#include <renderer\vulkan\VulkanDescriptorSet.hpp>

#include <ComponentEngine\Components\Transformation.hpp>
#include <ComponentEngine\Components\Camera.hpp>
#include <ComponentEngine\DefaultMeshVertex.hpp>
#include <ComponentEngine\ThreadHandler.hpp>
#include <ComponentEngine\ThreadManager.hpp>
#include <ComponentEngine/PhysicsWorld.hpp>
#include <ComponentEngine\tiny_obj_loader.h>
#include <ComponentEngine\DefaultMeshVertex.hpp>
#include <ComponentEngine\obj_loader.h>


#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <SDL.h>
#include <SDL_syswm.h>
#include <imgui.h>

using namespace enteez;
using namespace Renderer;
using namespace Renderer::Vulkan;

namespace Renderer
{
	class VertexBase;
	namespace Vulkan
	{
		class VulkanDescriptorPool;
		class VulkanDescriptorSet;
		class VulkanTextureBuffer;
		class VulkanBufferPool;
		class VulkanModelPool;
		class VulkanModel;
		class VulkanRenderPass;
		class VulkanSwapchain;
	}
}

namespace ComponentEngine
{
	enum EngineFlags
	{
		ReleaseBuild = 0x01
	};

	enum ConsoleState
	{
		Default,
		Info,
		Warning,
		Error
	};

	enum EngineStates
	{
		Stopped,
		Running,
		Stoping
	};

	enum PlayState
	{
		Editor = 0x01,
		Play = 0x02,
		Release = 0x04
	};

	enum EngineLock
	{
		IS_RUNNING,
		TOGGLE_FRAME_LIMITING,
		READ_KEY_PRESS,
		READ_MOUSE_DATA,
		CONSOLE,

		MAX
	};
	struct ConsoleMessage
	{
		std::string message;
		unsigned int count;
		ConsoleState state;
	};
	class UIManager;
	// Found in Components/Light.hpp
	struct LightData;


	static const std::string EngineName = "Component Engine";
	class Engine : public EnteeZ
	{
	public:

		// Gets a singlton instance of the engine
		static Engine* Singlton();

		~Engine();
		// Start the engine and run all services
		void Start();
		// Stop the engine and kill all services
		void Stop();
		// Force all threads to join the main thread
		void Join();
		// Is the engine running
		bool Running();
		// To be used, update safe and dose not call the thread timing reset
		bool IsRunning();
		// Update the threading and window services
		void Update();
		// Update the scene by transferring all data from the temporary secondary buffers to the primary ones
		void UpdateScene();
		// Update the ui manager
		void UpdateUI(float delta);
		// Rebuild the renderer components of the engine
		void Rebuild();
		// Render the next scene frame
		void RenderFrame();
		// Log some data to the internal console
		void Log(std::string data, ConsoleState state = Default);
		// Check to see if X key is down
		bool KeyDown(int key);
		// Check to see if X Mouse Button is down
		bool MouseKeyDown(int key);
		// Return the last mouse movement instance
		glm::vec2 GetLastMouseMovment();
		// Merge Scene stops the old scene from being deleted before the new scene is added so both scenes will be side by side.
		bool LoadScene(const char* path);
		// Save to the current scene
		void SaveScene();
		// Get the renderer instance
		VulkanRenderer* GetRenderer();
		// Get the cameras description pool
		VulkanDescriptorPool* GetCameraPool();
		// Get the main cameras description set
		VulkanDescriptorSet* GetCameraDescriptorSet();
		// Get the texture map descriptor pool
		VulkanDescriptorPool* GetTextureMapsPool();
		// Load a texture, if the texture is already loaded, get the texture instance
		VulkanTextureBuffer* LoadTexture(std::string path);
		// Load a texture from a data pointer
		VulkanTextureBuffer* LoadTexture(unsigned int width, unsigned int height, char* data);
		// Get the current scene name and directory
		std::string GetCurrentScene();
		// Get the current scene directory
		std::string GetCurrentSceneDirectory();
		// Get the mutex that locks all logic calls
		ordered_lock& GetLogicMutex();
		// Get the mutex that locks all render calls
		ordered_lock& GetRendererMutex();
		// Get the thread manager instance
		ThreadManager* GetThreadManager();
		// Define what camera should be the primary camera
		void SetCamera(Camera* camera);
		// Check to see if we have a custom camera defined
		bool HasCamera();
		// Get the current main camera even if it is the default one
		Camera* GetMainCamera();
		// Get the default camera
		Camera* GetDefaultCamera();
		// Get the current window handle
		NativeWindowHandle* GetWindowHandle();
		// Define a new component and how it should be initialized
		void RegisterComponentBase(std::string name, BaseComponentWrapper*(*default_initilizer)(enteez::Entity& entity));
		// Define that the mouse should be locked to the window
		void GrabMouse(bool grab);
		// Set the engines current state
		void SetPlayState(PlayState play_state);
		// Set the engines current state
		PlayState GetPlayState();
		// Get the current physics world
		PhysicsWorld* GetPhysicsWorld();
		// Get a engine mutex
		std::mutex& GetLock(EngineLock lock);
		// Get all console messages
		std::vector<ConsoleMessage>& GetConsoleMessages();
		// Get all registered components
		std::map<std::string, BaseComponentWrapper*(*)(enteez::Entity& entity)> GetComponentRegister();
		// Set a engine flag
		void SetFlag(int flags);
		// Return the UI manager instance
		UIManager* GetUIManager();
		// Get the global vertex buffer that stores all primary vertex data
		VulkanVertexBuffer* GetGlobalVertexBufer();
		// Get the global index buffer that stores all primary index data
		VulkanIndexBuffer* GetGlobalIndexBuffer();
		// Get the uniform buffer that stored all material data
		VulkanUniformBuffer* GetMaterialBuffer();
		// Get the local vertex array data
		std::vector<MeshVertex>& GetGlobalVertexArray();
		// Get the local index array data
		std::vector<uint32_t>& GetGlobalIndexArray();
		// Get the local material array data
		std::vector<MatrialObj>& GetGlobalMaterialArray();
		// Get the local texture descriptor array data
		std::vector<VkDescriptorImageInfo>& GetTextureDescriptors();
		// Get the local texture buffer array
		std::vector<VulkanTextureBuffer*>& GetTextures();
		// Get the allocation pool for model positions
		VulkanBufferPool* GetPositionBufferPool();
		// Get the allocation pool for lights
		VulkanBufferPool* GetLightBufferPool();
		// Get the raytracing top level acceleration structure
		VulkanAcceleration* GetTopLevelAS();
		// Get the total used vertex size
		unsigned int& GetUsedVertex();
		// Get the total used index size
		unsigned int& GetUsedIndex();
		// Get the total used materials size
		unsigned int& GetUsedMaterials();
		// Rebuild the offset array that defined where models information is defined
		void RebuildOffsetAllocation();
		// Update all buffers and offsets that are needed for RTX
		void UpdateAccelerationDependancys();
		// Get the uniform buffer that stores all position buffers
		VulkanUniformBuffer* GetModelPositionBuffer();

		friend class UIManager;
	private:
		Engine();

		// Create a SDL window instance
		void InitWindow();
		// Poll the SDL window for updates
		void UpdateWindow();
		// Destroy the SDL window instance
		void DeInitWindow();

		// Define all built in component definitions
		void InitEnteeZ();
		// DeInit EnteeZ
		void DeInitEnteeZ();

		// Create the renderer instance and all required components
		void InitRenderer();
		// Destroy the current renderer instance
		void DeInitRenderer();
		// Define the EnteeZ component hooks
		void InitComponentHooks();

		// Create the physics world instance
		void InitPhysicsWorld();
		// Destroy physics world
		void DeInitPhysicsWorld();

		// Create a instance of imgui
		void InitImGUI();
		// Update the ui manager
		void UpdateImGUI();
		// Destroy the instance of imgui
		void DeInitImGUI();
		// Request that the engine should stop
		void RequestStop();
		// Request that the engine should switch threading mode
		void RequestToggleThreading();
		// Are we currently threading
		bool Threading();
		// Request that the engine should switch threading mode
		void ToggleThreading();
		// Set the current scene path
		void SetScenePath(const char* path);

		// Singlton instance of engine
		static Engine* m_engine;

		// Windowing data
		SDL_Window* m_window;
		NativeWindowHandle* m_window_handle;
		const char* m_title; 
		int m_width;
		int m_height;

		EngineStates m_running = EngineStates::Stopped;
		bool m_request_stop = false;
		bool m_request_toggle_threading = false;
		bool m_threading = true;

		std::thread::id m_main_thread;

		PlayState m_play_state;

		UIManager* m_ui;

		// Rendering Data
		VulkanRenderer* m_renderer = nullptr;
		VulkanRenderPass* m_render_pass = nullptr;
		VulkanSwapchain* m_swapchain = nullptr;

		// Default pipeline data
		VulkanDescriptorPool* m_camera_pool;
		VulkanDescriptorSet* m_camera_descriptor_set;
		VulkanDescriptorPool* m_texture_maps_pool;

		// Raytrace pipeline
		VulkanRaytracePipeline* m_default_raytrace = nullptr;

		Camera* m_default_camera;
		// Main camera
		Camera* m_main_camera = nullptr;

		// ImGUI
		struct
		{
			VulkanGraphicsPipeline* m_imgui_pipeline = nullptr;
			// Screen buffer
			glm::vec2 m_screen_dim;
			VulkanUniformBuffer* m_screen_res_buffer = nullptr;
			VulkanDescriptorPool* m_screen_res_pool = nullptr;
			VulkanDescriptorSet* m_screen_res_set = nullptr;
			// Font texture
			VulkanTextureBuffer* m_font_texture = nullptr;
			VulkanDescriptorPool* m_font_texture_pool = nullptr;
			VulkanDescriptorSet* m_texture_descriptor_set = nullptr;
			// Local memory
			ImDrawVert* m_vertex_data = nullptr;
			uint32_t* m_index_data = nullptr;
			// GUI GPU Buffer
			VulkanVertexBuffer* m_vertex_buffer = nullptr;
			VulkanIndexBuffer* m_index_buffer = nullptr;
			// Model pool instance
			VulkanModelPool* model_pool = nullptr;
			std::vector<VulkanModel*> draw_group;

			glm::mat4* draw_group_textures = new glm::mat4[1000];
			VulkanUniformBuffer* draw_group_textures_buffer = nullptr;
			VulkanBufferPool* draw_group_textures_buffer_pool = nullptr;

			std::vector<VkDescriptorImageInfo> texture_descriptors;
		}m_imgui;




		struct ThreadData
		{
			Uint64 delta_time;

			void ResetTimers();

			std::vector<float> process_time_average_storage;
			float delta_process_time;
			float process_time;
			float process_time_average;

			std::vector<float> loop_time_average_storage;
			float delta_loop_time;
			float loop_time;
			float loop_time_average;
			// Requested Update Per Second
			unsigned int requested_ups = 60; // Check for a average of 60 updates per second

			bool frame_limited;

			const char* name;
			ordered_lock data_lock;

			//ThreadHandler* thread_instance = nullptr;
		};

		ordered_lock m_renderer_thread;
		ordered_lock m_logic_lock;
		ordered_lock m_thread_data_lock;

		std::vector<ConsoleMessage> m_console;
		//std::vector<ThreadData*> m_thread_data;
		//std::map<std::thread::id, ThreadData*> m_thread_linker;

		std::map<std::string, BaseComponentWrapper*(*)(enteez::Entity& entity)> m_component_register;

		std::map<std::string, void(*)(enteez::Entity& entity)> m_component_gui;

		std::string m_currentScene;
		std::string m_currentSceneDirectory;

		PhysicsWorld* m_physicsWorld;

		ThreadManager* m_threadManager;

		int m_flags;

		// Store the various locks that will be needed
		std::mutex m_locks[EngineLock::MAX];

		bool m_keys[256];
		glm::vec2 m_mousePosDelta;
		glm::vec2 m_lastMousePos;
		int m_lockedPosX;
		int m_lockedPosY;


		VulkanAcceleration* m_top_level_acceleration;
		
		std::vector<VulkanTextureBuffer*> m_textures;
		std::vector<VkDescriptorImageInfo> m_texture_descriptors;
		std::vector<MatrialObj> m_materials;

		unsigned int m_used_vertex = 0;
		unsigned int m_used_index = 0;
		unsigned int m_used_materials = 0;

		const unsigned int m_vertex_max = 1000000;
		const unsigned int m_index_max = 1000000;
		const unsigned int m_max_texture_descriptors = 1000;
		const unsigned int m_max_materials = 1000;

		std::vector<MeshVertex> m_all_vertexs;
		std::vector<uint32_t> m_all_indexs;

		VulkanDescriptorSet* m_standardRTConfigSet = nullptr;
		VulkanDescriptorSet* m_RTModelPoolSet = nullptr;
		VulkanDescriptorSet* m_RTTexturePoolSet = nullptr;
		VulkanDescriptorSet* m_RTModelInstanceSet = nullptr;

		VulkanVertexBuffer* m_vertexBuffer;
		VulkanIndexBuffer* m_indexBuffer;
		VulkanUniformBuffer* m_materialbuffer;

		glm::mat4* m_model_position_array;
		VulkanUniformBuffer* m_model_position_buffer;
		VulkanBufferPool* m_position_buffer_pool;

		struct ModelOffsets
		{
			uint32_t index;
			uint32_t vertex;
			uint32_t position;
		};

		ModelOffsets* m_offset_allocation_array;
		VulkanUniformBuffer* m_offset_allocation_array_buffer;

		
		std::vector<LightData> m_lights = {
		//{ glm::vec3(60.0f, 200.0f, -20.0f), 32000, glm::vec3(1.0f, 0.9f, 0.8f) },
		//{ glm::vec3(0.0f, 2.0f, 0.0f), 20, glm::vec3(0.0f, 1.0f, 1.0f) },
		};
		VulkanUniformBuffer* m_light_buffer;
		VulkanBufferPool* m_light_buffer_pool;

	};
	
}