#pragma once
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Common.hpp>

#include <vector>
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

			{ // Get directory
				const size_t lastBackSlash = longForm.rfind('\\');
				if (std::string::npos != lastBackSlash)
				{
					folder = longForm.substr(0, lastBackSlash);
				}
				else
				{
					const size_t lastForwardSlash = longForm.rfind('/');
					if (std::string::npos != lastForwardSlash)
					{
						folder = longForm.substr(0, lastForwardSlash);
					}
					else
					{
						folder = "../";
					}
				}
			}
			extension = std::experimental::filesystem::path(path).extension().string();
		}
		std::string folder; // Full file path
		std::string longForm; // Full file path
		std::string shortForm; // File name
		std::string extension; // File extention
	};

	struct Folder
	{
		bool topLevel = false;
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



	class Engine;
	class UIBase;
	class MenuElement;
	class UIManager
	{
	public:
		// Extra functionality to UI components
		static void Tooltip(const char* text);
		static bool DropTarget(const char* type, const  ImGuiPayload*& payload);
		static void DropPayload(const char* type, const char* message, void* payload, unsigned int size);
		template<typename T>
		static bool ComponentDropBox(const char* lable, ComponentDropInstance<T>& inst);
		static bool StringDropBox(const char* lable, const char* payloadType, StringDropInstance& inst);
		template<typename T>
		static bool DropBox(const char* lable, const char* payloadType, DropBoxInstance<T>& inst);
		static bool IsWindowFocused();
		static bool ElementClicked(bool repeated = false);
		static void KeyboardButtonInput(const char* lable, bool& focused, unsigned int& key);
		static bool EdiableText(std::string& text, char*& temp_data, int max_size, bool editable = true);

		static unsigned int GetMenuBarHeight();
		static void DrawScalingImage(unsigned int texture_id, unsigned int image_width, unsigned int image_height, unsigned int window_width, unsigned int window_height);
		static void CalculateImageScaling(unsigned int image_width, unsigned int image_height, unsigned int window_width, unsigned int window_height, ImVec2& new_image_offset, ImVec2& new_image_size);

		UIManager(Engine* engine);
		~UIManager();
		void AddElement(UIBase* base);
		void AddMenuElement(MenuElement* element);
		void Render();
		CurrentSceneFocus& GetCurrentSceneFocus();
		void ResetSceneFocus();

	private:
		static void DockSpace();
		void RenderMainMenu();
		void RenderMenuElement(MenuElement* element);

		CurrentSceneFocus m_current_scene_focus;

		std::vector<UIBase*> m_bases;
		std::vector<MenuElement*> m_menu_elements;

		
		Engine * m_engine;
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