#pragma once

#include <EnteeZ\EnteeZ.hpp>

#include <renderer\vulkan\VulkanRenderer.hpp>
#include <renderer\IBufferPool.hpp>
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
using namespace Renderer::Vulkan;

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

	struct TextureStorage
	{
		Renderer::IDescriptorPool* texture_maps_pool;
		Renderer::IDescriptorSet* texture_descriptor_set;
		Renderer::ITextureBuffer* texture;
	};
	struct PipelinePack
	{
		IGraphicsPipeline* pipeline = nullptr;
		std::function<void(IGraphicsPipeline*, IModelPool*, const char* workingDir, tinyobj::material_t)> modelCreatePointer = nullptr;
	};
	struct ConsoleMessage
	{
		std::string message;
		unsigned int count;
		ConsoleState state;
	};
	class UIManager;
	static const std::string EngineName = "Component Engine";
	class Engine : public EnteeZ
	{
	public:


		static Engine* Singlton();
		~Engine();
		void Start();
		//void AddThread(ThreadHandler* handler, const char* name = (const char*)0);
		void Stop();
		void Join();
		bool Running();
		bool IsRunning(); // To be used, update safe and dose not call the thread timing reset
		void Update();
		void UpdateScene();
		void UpdateUI(float delta);
		void Rebuild();
		void RenderFrame();
		void Log(std::string data, ConsoleState state = Default);
		bool KeyDown(int key);
		bool MouseKeyDown(int key);
		glm::vec2 GetLastMouseMovment();

		// Merge Scene stops the old scene from being deleted before the new scene is added so both scenes will be side by side.
		bool LoadScene(const char* path);

		void SaveScene();

		void AddPipeline(std::string name, PipelinePack pipeline);

		PipelinePack& GetPipeline(std::string name);
		PipelinePack& GetPipelineContaining(std::string name);

		IGraphicsPipeline* GetDefaultGraphicsPipeline();
		VulkanRenderer* GetRenderer();
		IDescriptorPool* GetCameraPool();
		IDescriptorSet* GetCameraDescriptorSet();
		IDescriptorPool* GetTextureMapsPool();

		VertexBase GetDefaultVertexModelBinding();
		VertexBase GetDefaultVertexModelPositionBinding();

		//float GetThreadDeltaTime();
		//float GetLastThreadTime();
		TextureStorage& GetTexture(std::string path);

		std::string GetCurrentScene();
		std::string GetCurrentSceneDirectory();

		ordered_lock& GetLogicMutex();
		ordered_lock& GetRendererMutex();

		ThreadManager* GetThreadManager();

		void SetCamera(Camera* camera);

		bool HasCamera();

		Camera* GetMainCamera();

		Camera* GetDefaultCamera();

		NativeWindowHandle* GetWindowHandle();

		void RegisterComponentBase(std::string name, BaseComponentWrapper*(*default_initilizer)(enteez::Entity& entity));

		void GrabMouse(bool grab);

		void SetPlayState(PlayState play_state);

		PlayState GetPlayState();

		PhysicsWorld* GetPhysicsWorld();

		std::mutex& GetLock(EngineLock lock);

		std::vector<ConsoleMessage>& GetConsoleMessages();

		std::map<std::string, BaseComponentWrapper*(*)(enteez::Entity& entity)> GetComponentRegister();

		void SetFlag(int flags);

		UIManager* GetUIManager();

		IVertexBuffer* GetGlobalVertexBufer();

		IIndexBuffer* GetGlobalIndexBuffer();

		IUniformBuffer* GetMaterialBuffer();

		std::vector<MeshVertex>& GetGlobalVertexArray();

		std::vector<uint32_t>& GetGlobalIndexArray();

		std::vector<MatrialObj>& GetGlobalMaterialArray();

		std::vector<VkDescriptorImageInfo>& GetTextureDescriptors();

		std::vector<VulkanTextureBuffer*>& GetTextures();

		IBufferPool* GetPositionBufferPool();

		VulkanAcceleration* GetTopLevelAS();

		unsigned int& GetUsedVertex();

		unsigned int& GetUsedIndex();

		unsigned int& GetUsedTextureDescriptors();

		unsigned int& GetUsedMaterials();

		void RebuildOffsetAllocation();

		IUniformBuffer* GetModelPositionBuffer();

		friend class UIManager;
	private:
		Engine();
		Uint32 GetWindowFlags(RenderingAPI api);

		void InitWindow();
		void UpdateWindow();
		void DeInitWindow();

		void InitEnteeZ();
		void DeInitEnteeZ();

		void InitRenderer();
		void DeInitRenderer();
		void InitComponentHooks();

		void InitPhysicsWorld();
		void DeInitPhysicsWorld();

		void InitImGUI();
		void UpdateImGUI();
		void DeInitImGUI();

		//void NewThreadUpdatePass();
		void RequestStop();
		void RequestToggleThreading();

		//void ToggleFrameLimiting();
		bool Threading();
		void ToggleThreading();


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

		std::map<std::string, PipelinePack> m_pipelines;

		// Default pipeline data
		IGraphicsPipeline* m_default_pipeline = nullptr;
		IDescriptorPool* m_camera_pool;
		IDescriptorSet* m_camera_descriptor_set;
		IDescriptorPool* m_texture_maps_pool;

		// Raytrace pipeline
		VulkanRaytracePipeline* m_default_raytrace = nullptr;

		Camera* m_default_camera;
		// Main camera
		Camera* m_main_camera = nullptr;

		// ImGUI
		struct
		{
			IGraphicsPipeline* m_imgui_pipeline = nullptr;
			// Screen buffer
			glm::vec2 m_screen_dim;
			IUniformBuffer* m_screen_res_buffer = nullptr;
			IDescriptorPool* m_screen_res_pool = nullptr;
			IDescriptorSet* m_screen_res_set = nullptr;
			// Font texture
			ITextureBuffer* m_font_texture = nullptr;
			IDescriptorPool* m_font_texture_pool = nullptr;
			IDescriptorSet* m_texture_descriptor_set = nullptr;
			// Local memory
			ImDrawVert* m_vertex_data = nullptr;
			uint32_t* m_index_data = nullptr;
			// GUI GPU Buffer
			IVertexBuffer* m_vertex_buffer = nullptr;
			IIndexBuffer* m_index_buffer = nullptr;
			// Model pool instance
			IModelPool* model_pool = nullptr;
			IModel* model = nullptr;
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

		std::map<std::string, TextureStorage> m_texture_storage;

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
		unsigned int m_used_texture_descriptors = 0;
		unsigned int m_used_materials = 0;

		const unsigned int m_vertex_max = 1000000;
		const unsigned int m_index_max = 1000000;
		const unsigned int m_max_texture_descriptors = 1000;
		const unsigned int m_max_materials = 1000;

		std::vector<MeshVertex> m_all_vertexs;
		std::vector<uint32_t> m_all_indexs;

		VulkanDescriptorSet* m_standardRTConfigSet = nullptr;

		IVertexBuffer* m_vertexBuffer;
		IIndexBuffer* m_indexBuffer;
		IUniformBuffer* m_materialbuffer;
		IUniformBuffer* m_lightBuffer;

		glm::mat4* m_model_position_array;
		IUniformBuffer* m_model_position_buffer;
		IBufferPool* m_position_buffer_pool;

		struct ModelOffsets
		{
			uint32_t index;
			uint32_t vertex;
			uint32_t position;
		};

		ModelOffsets* m_offset_allocation_array;
		IUniformBuffer* m_offset_allocation_array_buffer;
	};
	
}