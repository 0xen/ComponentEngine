#pragma once

#include <ComponentEngine\Engine.hpp>

namespace ComponentEngine
{

	struct CurrentSceneFocus
	{
		enteez::Entity* entity = nullptr;
		BaseComponentWrapper* component = nullptr;
		char* entity_temp_name = nullptr;
	};

	class UIManager
	{
	public:
		UIManager(Engine* engine);
		void Render();
	private:
		void RenderMainMenu();
		void RenderFPSCounter();
		// Scene render functions
		void RenderScene();
		void RenderEntityTreeNode(Entity* entity);
		void RenderEntity(Entity* entity);
		void RenderComponentTreeNode(Entity* entity, BaseComponentWrapper& wrapper);
		void RenderComponent();
		// Add component render functions
		void RenderAddComponent();

		bool ElementClicked();
		bool EdiableText(std::string& text,char*& temp_data, int max_size, bool editable = true);

		void ResetSceneFocusEntity();
		void ResetSceneFocusComponent();

		void Tooltip(const char* text);

		Engine* m_engine;
		CurrentSceneFocus m_current_scene_focus;
		unsigned int m_indestructable_component_id = 0;
		static const unsigned int SCENE;
		static const unsigned int ADD_COMPONENT;

		float m_thread_time_update_delay;
		std::map<Engine::ThreadData*, std::vector<float>> m_thread_times_miliseconds;
		std::map<Engine::ThreadData*, float> m_thread_times_fraction_last;

		bool m_open[2];
	};
}
