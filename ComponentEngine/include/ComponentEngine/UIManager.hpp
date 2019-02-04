#pragma once

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Common.hpp>
#include <sstream>
#include <filesystem>

namespace ComponentEngine
{

	struct CurrentSceneFocus
	{
		enteez::Entity* entity = nullptr;
		char* entity_temp_name = nullptr;
	};

	struct FileForms
	{
		void GenerateFileForm(std::string path)
		{
			longForm = path;
			Common::Replace(longForm, "\\", "/");
			shortForm = std::experimental::filesystem::path(path).filename().string();
			extension = std::experimental::filesystem::path(path).extension().string();
		}
		std::string longForm; // Full file path
		std::string shortForm; // File name
		std::string extension; // File extention
	};

	struct Folder
	{
		bool readFolder = false;;
		FileForms path;
		std::vector<FileForms> files;
		std::vector<Folder> folders;
	};

	struct DropInstance
	{
	protected:
		DropInstance(std::string message_prefix) : messagePrefix(message_prefix) {}
	public:
		std::string messagePrefix = "";
		std::string defaultMessage = "(Empty)";
		std::string message = "";
		virtual void Reset() = 0;
		void SetMessage(std::string mesage)
		{
			std::stringstream ss;
			ss << messagePrefix << " (" << mesage << ")";
			message = ss.str();
		}
	};

	struct StringDropInstance : public DropInstance
	{
		StringDropInstance(std::string message_prefix) : DropInstance(message_prefix)
		{
			Reset();
		}
		std::string data;
		virtual void Reset()
		{
			data = "";
			defaultMessage = messagePrefix + " (Empty)";
			message = messagePrefix + " (Empty)";
		}
	};

	template<typename T>
	struct ComponentDropInstance : public DropInstance
	{
		ComponentDropInstance(std::string message_prefix) : DropInstance(message_prefix)
		{
			Reset();
		}
		T* component = nullptr;
		Entity* entity = nullptr;
		virtual void Reset()
		{
			component = nullptr;
			entity = nullptr;
			defaultMessage = messagePrefix + " (Empty)";
			message = messagePrefix + " (Empty)";
		}
	};

	template<typename T>
	struct DropBoxInstance : public DropInstance
	{
		DropBoxInstance(std::string message_prefix = "") : DropInstance(message_prefix)
		{
			Reset();
		}
		T data;
		virtual void Reset()
		{
			defaultMessage = messagePrefix + " (Empty)";
			message = messagePrefix + " (Empty)";
		}
	};

	class UIManager
	{
	public:

		UIManager(Engine* engine);
		void Render();

		// Extra functionality to UI components
		static void Tooltip(const char* text);
		static bool DropTarget(const char* type, const  ImGuiPayload*& payload);
		static void DropPayload(const char* type, const char* message, void* payload, unsigned int size);
		template<typename T>
		static bool ComponentDropBox(const char* lable, ComponentDropInstance<T>& inst);
		static bool StringDropBox(const char* lable, const char* payloadType, StringDropInstance& inst);
		template<typename T>
		static bool DropBox(const char* lable, const char* payloadType, DropBoxInstance<T>& inst);
	private:
		void RenderMainMenu();
		//void RenderFPSCounter();

		// Dock Space
		// Used as the central docking system for the windows
		void DockSpace();

		void RendererExplorer();
		void RendererFolder(Folder& folder);

		void RenderComponentHierarchy();

		void ThreadingWindow();

		// Scene render functions
		void RenderSceneHierarchy();
		void RenderEntityTreeNode(Entity* entity);
		void RenderEntity(Entity* entity);

		void DestroyEntity(Entity* entity);

		bool ElementClicked();
		bool EdiableText(std::string& text,char*& temp_data, int max_size, bool editable = true);

		void ResetSceneFocusEntity();


		void AddEntityDialougeMenu(Entity* parent);
		void AddComponentDialougeMenu();




		void LoadFolder(Folder& folder);

		Folder sceneFolder;

		Engine* m_engine;
		CurrentSceneFocus m_current_scene_focus;
		unsigned int m_indestructable_component_id = 0;
		static const unsigned int SCENE_HIERARCHY;
		static const unsigned int COMPONENT_HIERARCHY;
		static const unsigned int EXPLORER;

		float m_thread_time_update_delay;
		std::map<Engine::ThreadData*, std::vector<float>> m_thread_times_miliseconds;
		std::map<Engine::ThreadData*, float> m_thread_times_fraction_last;

		bool m_open[3];
	};
	template<typename T>
	inline bool UIManager::ComponentDropBox(const char* lable, ComponentDropInstance<T>& inst)
	{
		ImGui::Text(lable);
		ImGui::SameLine();
		ImGui::InputText("", (char*)inst.message.c_str(), inst.message.size(), ImGuiInputTextFlags_ReadOnly);


		const ImGuiPayload* payload = nullptr;
		if (DropTarget("Entity", payload))
		{
			IM_ASSERT(payload->DataSize == sizeof(Entity*));
			Entity* entity = *(Entity**)payload->Data;

			if (entity->HasComponent<T>())
			{
				inst.entity = entity;
				inst.component = &entity->GetComponent<T>();
				inst.SetMessage(entity->GetName());
				return true;
			}
		}
		return false;
	}
	template<typename T>
	inline bool UIManager::DropBox(const char * lable, const char * payloadType, DropBoxInstance<T> & inst)
	{
		ImGui::Text(lable);
		ImGui::SameLine();
		ImGui::InputText("", (char*)inst.message.c_str(), inst.message.size(), ImGuiInputTextFlags_ReadOnly);


		const ImGuiPayload* payload = nullptr;
		if (DropTarget(payloadType, payload))
		{
			IM_ASSERT(payload->DataSize == sizeof(T));
			inst.data = *(T*)payload->Data;
			return true;
		}
		return false;
	}
}
