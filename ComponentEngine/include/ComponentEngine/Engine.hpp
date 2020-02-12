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

	struct HitShaderPipeline
	{
		std::string name;
		std::map<VkShaderStageFlagBits, const char*> hitgroup;
		// Can the hitgroup be used as a primary hit group? For example, if we have a fall through shadow shader, then we should not be able to select it
		bool primaryHitgroup;
	};

	class UIManager;
	class MenuElement;
	// Found in Components/Light.hpp
	struct LightData;
	struct MaterialDefintion;


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
		// Is the engine running
		bool Running();
		// To be used, update safe and dose not call the thread timing reset
		bool IsRunning();
		// Update the threading and window services
		void Update();
		// Update the scene by transferring all data from the temporary secondary buffers to the primary ones
		void UpdateSceneBuffers();
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
		// Reset mouse movement
		void ResetMouseMovment();
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

		MaterialDefintion GetMaterialDefinition(int index);

		void RegisterMaterial(MaterialDefintion definition, MatrialObj material);
		// Get the textures offset in the texture array
		int GetMaterialOffset(MaterialDefintion definition);

		// Get the textures offset in the texture array
		int GetTextureOffset(std::string path);
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

		// 
		ordered_lock& GetModelLoadMutex();
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
		// Get the uniform buffer that maps models to there materials
		VulkanUniformBuffer* GetMaterialMappingBuffer();
		// Get the uniform buffer that stored all material data
		VulkanUniformBuffer* GetMaterialBuffer();
		// Get the local material mapping array
		std::vector<std::array<int, 8>>& GetGlobalMaterialMappingArray();
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
		// How many materials can each model have
		int GetMaxMaterialsPerModel();
		// Get the allocation pool for model positions
		VulkanBufferPool* GetPositionBufferPool();

		VulkanBufferPool* GetPositionITBufferPool();
		// Get the allocation pool for model positions
		VulkanBufferPool* GetMaterialMappingPool();
		// Get the allocation pool for lights
		VulkanBufferPool* GetLightBufferPool();
		// Get the raytracing top level acceleration structure
		VulkanAcceleration* GetTopLevelAS();
		// Get the engines render pass instance
		VulkanRenderPass* GetRenderPass();
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
		// Define a new miss shader for the pipeline
		unsigned int AddMissShader(const char* missShader, const std::vector<unsigned int>& constants);
		// Define a new hit group for the raytracing pipeline
		unsigned int AddHitShaderPipeline(HitShaderPipeline pipeline, const std::vector<unsigned int>& constants);
		// Get all hit shader instances
		std::vector<HitShaderPipeline>& GetHitShaderPipelines();

		void ResetViewportBuffers();

		const unsigned int GetRaytracerRecursionDepth();

		std::string GetHoveredWindowName();

		void SetHoveredWindowName(std::string name);

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

		void RebuildRaytracePipeline();

		// Create the renderer instance and all required components
		void InitRenderer();
		// Destroy the current renderer instance
		void DeInitRenderer();

		void InitRaytracingResources();

		void RebuildRaytracingResources();

		void DestroyRaytracingResources();

		// Define the EnteeZ component hooks
		void InitComponentHooks();

		// Create the physics world instance
		void InitPhysicsWorld();
		// Destroy physics world
		void DeInitPhysicsWorld();

		// Create a instance of imgui
		void InitImGUI();
		// Transfer ImGui Buffers to the GPU
		void TransferImGui();
		// Update the ui manager
		void UpdateImGUI();
		// Destroy the instance of imgui
		void DeInitImGUI();
		// Request that the engine should stop
		void RequestStop();
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

		std::thread::id m_main_thread;

		PlayState m_play_state;

		UIManager* m_ui;

		MenuElement* m_window_dropdown;

		// Rendering Data
		VulkanRenderer* m_renderer = nullptr;
		VulkanRenderPass* m_raytrace_renderpass = nullptr;
		VulkanSwapchain* m_swapchain = nullptr;

		// Default pipeline data
		VulkanDescriptorPool* m_camera_pool;
		VulkanDescriptorSet* m_camera_descriptor_set;
		VulkanDescriptorPool* m_texture_maps_pool;

		// Raytrace pipeline
		VulkanRaytracePipeline* m_default_raytrace = nullptr;
		VulkanDescriptorPool* standardRTConfigPool = nullptr;
		VulkanDescriptorPool* RTModelPool = nullptr;
		VulkanDescriptorPool* RTTextureDescriptorPool = nullptr;
		VulkanDescriptorPool* RTModelInstancePool = nullptr;
		// All hit groups
		std::vector<HitShaderPipeline> m_pipelines;
		std::vector<std::vector<unsigned int>> m_pipelines_constants;
		// All mis groups
		std::vector<std::pair<VkShaderStageFlagBits, const char*>> m_miss_groups;
		std::vector<std::vector<unsigned int>> m_miss_groups_constants;

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
		ordered_lock m_ui_lock;
		ordered_lock m_renderer_ui_transfer;
		bool m_ui_transfer_ready = false;

		ordered_lock m_logic_lock;
		ordered_lock m_renderer_logic_transfer;
		bool m_logic_transfer_ready = false;

		ordered_lock m_thread_data_lock;
		ordered_lock m_model_load_lock;
		ordered_lock m_texture_load_lock;
		ordered_lock m_material_load_lock;

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

		// Use real time clock
		bool m_use_RTC;

		// Store the various locks that will be needed
		std::mutex m_locks[EngineLock::MAX];

		bool m_keys[256];
		glm::vec2 m_mousePosDelta;
		glm::vec2 m_lastMousePos;
		int m_lockedPosX;
		int m_lockedPosY;

		std::string m_hovered_window_name;

		VulkanAcceleration* m_top_level_acceleration;
		
		std::map<std::string, int> m_texture_mapping;
		std::vector<VulkanTextureBuffer*> m_textures;
		std::vector<VkDescriptorImageInfo> m_texture_descriptors;


		std::map<MaterialDefintion, int> m_material_mapping;

		std::vector<std::array<int, 8>> m_materials_vertex_mapping;
		VulkanBufferPool* m_materials_vertex_mapping_buffer_pool;

		std::vector<MatrialObj> m_materials;

		unsigned int m_used_vertex = 0;
		unsigned int m_used_index = 0;
		unsigned int m_used_materials = 0;

		const unsigned int m_vertex_max = 1000000;
		const unsigned int m_index_max = 1000000;
		const unsigned int m_max_texture_descriptors = 1000;
		const unsigned int m_max_materials = 200;

		std::vector<MeshVertex> m_all_vertexs;
		std::vector<uint32_t> m_all_indexs;

		VulkanDescriptorSet* m_standardRTConfigSet = nullptr;
		VulkanDescriptorSet* m_RTModelPoolSet = nullptr;
		VulkanDescriptorSet* m_RTTexturePoolSet = nullptr;
		VulkanDescriptorSet* m_RTModelInstanceSet = nullptr;

		VulkanVertexBuffer* m_vertexBuffer;
		VulkanIndexBuffer* m_indexBuffer;

		const unsigned int m_maxMaterialsPerModel = 8;
		VulkanUniformBuffer* m_materialMappingBuffer;
		VulkanUniformBuffer* m_materialbuffer;

		glm::mat4* m_model_position_array;
		VulkanUniformBuffer* m_model_position_buffer;
		VulkanBufferPool* m_position_buffer_pool;

		glm::mat4* m_model_position_it_array;
		VulkanUniformBuffer* m_model_position_it_buffer;
		VulkanBufferPool* m_position_buffer_it_pool;

		struct ModelOffsets
		{
			uint32_t index;
			uint32_t vertex;
			uint32_t position;
		};

		ModelOffsets* m_offset_allocation_array;
		VulkanUniformBuffer* m_offset_allocation_array_buffer;

		
		std::vector<LightData> m_lights;
		VulkanUniformBuffer* m_light_buffer;
		VulkanBufferPool* m_light_buffer_pool;

		// Miss shader index. Create a gradient effect when the ray misses
		unsigned int m_gradient_miss_shader;
		//
		unsigned int m_global_illumination_miss_shader;
		//
		unsigned int m_shadow_miss_shader;
		// Default textured PBR shader
		unsigned int m_default_textured_pbr_shader;
		// Default single color shader
		unsigned int m_default_light_color_shader;
		// 
		unsigned int m_glass_shader;
		// 
		unsigned int m_textured_default_shader;

		const unsigned int m_maxRecursionDepth;

		// Used to store the acumilated image from multiple render passes
		VulkanTextureBuffer* m_accumilation_texture_buffer = nullptr;


		VulkanTextureBuffer* m_sample_texture_buffer = nullptr;


		VulkanTextureBuffer* m_ray_depth_buffer = nullptr;




		VulkanComputePipeline* m_sample_texture_rebuild_pipeline = nullptr;
		VulkanDescriptorPool* m_sample_texture_pool = nullptr;
		VulkanDescriptorSet* m_sample_texture_set = nullptr; 
		VulkanComputeProgram* m_sample_texture_rebuild_program = nullptr;


	};
	
}