#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include <renderer\IRenderer.hpp>

#include <ComponentEngine\Components\Transformation.hpp>
#include <ComponentEngine\Components\Indestructable.hpp>
#include <ComponentEngine\Components\Camera.hpp>
#include <ComponentEngine\DefaultMeshVertex.hpp>
#include <ComponentEngine\ThreadHandler.hpp>
#include <ComponentEngine\pugixml.hpp>


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

namespace ComponentEngine
{
	class UIManager;
	class Engine : public EnteeZ
	{
		struct ComponentTemplate
		{
			void(*default_initilizer)(enteez::Entity& entity) = nullptr;
			void(*xml_initilizer)(enteez::Entity& entity, pugi::xml_node& component_data) = nullptr;
		};
	public:
		static Engine* Singlton();
		~Engine();
		void Start();
		void AddThread(ThreadHandler* handler, const char* name = (const char*)0);
		void Stop();
		void Join();
		bool Running();
		bool Running(int ups);
		bool IsRunning(); // To be used, update safe and dose not call the thread timing reset
		void Update();
		void UpdateScene();
		void UpdateUI();
		void Rebuild();
		void RenderFrame();
		bool KeyDown(int key);
		float Sync(int ups);
		// Merge Scene stops the old scene from being deleted before the new scene is added so both scenes will be side by side.
		bool LoadScene(const char* path, bool merge_scenes = false);
		IGraphicsPipeline* GetDefaultGraphicsPipeline();
		IRenderer* GetRenderer();
		Entity* GetCameraEntity();
		Transformation* GetCameraTransformation();
		IDescriptorPool* GetCameraPool();
		IDescriptorSet* GetCameraDescriptorSet();
		IDescriptorPool* GetTextureMapsPool();

		float GetThreadDeltaTime();
		float GetLastThreadTime();
		ITextureBuffer* GetTexture(std::string path);

		std::string GetCurrentScene();
		std::string GetCurrentSceneDirectory();

		//ordered_lock& GetLogicMutex();
		ordered_lock& GetRendererMutex();

		void RegisterComponentBase(std::string name, void(*default_initilizer)(enteez::Entity& entity), void(*xml_initilizer)(enteez::Entity& entity, pugi::xml_node& component_data));

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

		void UpdateCameraProjection();

		void LoadXMLGameObject(pugi::xml_node& xml_entity, Entity* parent = nullptr);
		void AttachXMLComponent(pugi::xml_node& xml_component, enteez::Entity* entity);

		void InitImGUI();
		void UpdateImGUI();
		void DeInitImGUI();

		void NewThreadUpdatePass();
		void RequestStop();
		void RequestToggleThreading();

		void ToggleFrameLimiting();
		bool Threading();
		void ToggleThreading();

		// Singlton instance of engine
		static Engine* m_engine;

		// Windowing data
		SDL_Window* m_window;
		NativeWindowHandle* m_window_handle;
		RenderingAPI m_api;
		const char* m_title; 
		int m_width;
		int m_height;

		bool m_running = false;
		bool m_request_stop = false;
		bool m_request_toggle_threading = false;
		bool m_threading = true;

		std::thread::id m_main_thread;

		UIManager* m_ui;

		// Rendering Data
		IRenderer* m_renderer = nullptr;

		// Default pipeline data
		IGraphicsPipeline* m_default_pipeline = nullptr;
		Entity* m_camera_entity;
		Camera m_camera_component;
		IUniformBuffer* m_camera_buffer;
		IDescriptorPool* m_camera_pool;
		IDescriptorSet* m_camera_descriptor_set;
		IDescriptorPool* m_texture_maps_pool;

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
			ImDrawIdx* m_index_data = nullptr;
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

			ThreadHandler* thread_instance = nullptr;
		};

		ordered_lock m_renderer_thread;
		ordered_lock m_thread_data_lock;

		std::vector<ThreadData*> m_thread_data;
		std::map<std::thread::id, ThreadData*> m_thread_linker;

		std::map<std::string, ComponentTemplate> m_component_register;

		std::map<std::string, void(*)(enteez::Entity& entity)> m_component_gui;
		std::map<std::string, ITextureBuffer*> m_texture_storage;

		std::string m_currentScene;
		std::string m_currentSceneDirectory;

		static const unsigned int IS_RUNNING_LOCK;
		static const unsigned int TOGGLE_FRAME_LIMITING;
		static const unsigned int READ_KEY_PRESS;

		// Store the various locks that will be needed
		std::mutex m_locks[3];

		bool m_keys[256];
	};
}