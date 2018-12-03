#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include <renderer\IRenderer.hpp>

#include <ComponentEngine\Components\Transformation.hpp>
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
	class UIMaanger;
	class Engine : public EnteeZ
	{
		struct ComponentFunctionPointers
		{
			void(*initilizer)(enteez::Entity& entity, pugi::xml_node& component_data) = nullptr;
		};
	public:
		static Engine* Singlton();
		~Engine();
		void Start();
		void AddThread(void(*function)());
		void Stop();
		void Join();
		bool Running();
		void Update();
		void UpdateScene();
		void UpdateUI();
		void Rebuild();
		void RenderFrame();
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
		float GetFrameTime();
		float GetThreadTime();
		float GetFPS();
		ITextureBuffer* GetTexture(std::string path);

		//ordered_lock& GetLogicMutex();
		ordered_lock& GetRendererMutex();
		// Name needs to match how the component name will be written in the xml scene file

		void RegisterComponentInitilizer(const char* name, ComponentFunctionPointers cfp);

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

		void LoadXMLGameObject(pugi::xml_node& xml_entity);
		void AttachXMLComponent(pugi::xml_node& xml_component, enteez::Entity* entity);

		void InitImGUI();
		void UpdateImGUI();
		void DeInitImGUI();

		// Singlton instance of engine
		static Engine* m_engine;

		// Windowing data
		SDL_Window* m_window;
		NativeWindowHandle* m_window_handle;
		RenderingAPI m_api;
		const char* m_title; 
		int m_width;
		int m_height;

		UIMaanger* m_ui;

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

		// FPS
		Uint64 m_delta_time = 0;
		Uint64 m_now_delta_time = 0;
		float m_frame_time = 0;
		float m_fps_update = 0.0f;
		unsigned int m_delta_fps = 0;
		float m_fps = 0;

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

		std::vector<ThreadHandler*> m_threads;
		ordered_lock m_renderer_thread;

		std::map<std::thread::id, Uint64> m_thread_time_delta;

		std::map<std::string, ComponentFunctionPointers> m_component_initilizers;
		std::map<std::string, void(*)(enteez::Entity& entity)> m_component_gui;
		std::map<std::string, ITextureBuffer*> m_texture_storage;


		static const unsigned int IS_RUNNING_LOCK;
		static const unsigned int THREAD_TIME_LOCK;

		// Store the various locks that will be needed
		std::mutex m_locks[2];
	};
}