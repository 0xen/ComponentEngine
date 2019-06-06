#include <ComponentEngine\UI\UIManager.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\UI\UIBase.hpp>
#include <ComponentEngine\UI\MenuElement.hpp>

using namespace ComponentEngine;



void ComponentEngine::UIManager::Tooltip(const char * text)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(text);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool ComponentEngine::UIManager::DropTarget(const char * type, const ImGuiPayload *& payload)
{
	bool dropped = false;
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* pl = nullptr;
		dropped = pl = ImGui::AcceptDragDropPayload(type);
		if (dropped)
		{
			payload = pl;
		}
		ImGui::EndDragDropTarget();
	}
	return dropped;
}

void ComponentEngine::UIManager::DropPayload(const char * type, const char* message, void * payload, unsigned int size)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload(type, payload, size);        // Set payload to carry the index of our item (could be anything)
		ImGui::Text("Move %s", message);
		ImGui::EndDragDropSource();
	}
}

bool ComponentEngine::UIManager::StringDropBox(const char * lable, const char* payloadType, StringDropInstance& inst)
{
	ImGui::Text(lable);
	ImGui::SameLine();
	ImGui::InputText("", (char*)inst.message.c_str(), inst.message.size(), ImGuiInputTextFlags_ReadOnly);


	const ImGuiPayload* payload = nullptr;
	if (DropTarget(payloadType, payload))
	{
		IM_ASSERT(payload->DataSize == sizeof(std::string));
		std::string data = *(std::string*)payload->Data;


		inst.SetMessage(data);
		inst.data = data;

		return true;

	}


	return false;
}

bool ComponentEngine::UIManager::IsWindowFocused()
{
	return ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
}

bool ComponentEngine::UIManager::ElementClicked(bool repeated)
{
	return ImGui::IsItemHovered() && (ImGui::IsMouseClicked(0) && !repeated) || (ImGui::IsMouseDoubleClicked(0) && repeated);
}

void ComponentEngine::UIManager::KeyboardButtonInput(const char* lable, bool & focused, unsigned int & key)
{
	if (focused)
	{
		ImGuiIO& io = ImGui::GetIO();
		for (int i = 0; i < 512; i++)
		{
			if (io.KeysDown[i])
			{
				key = i;
				focused = false;
			}
		}
		static char tempBuffer[2]{ ' ','\0' };
		ImGui::SetKeyboardFocusHere();
		ImGui::CaptureKeyboardFromApp(true);
		bool change = ImGui::InputText(lable, tempBuffer, 1, ImGuiInputTextFlags_ReadOnly);
	}
	else
	{
		const char* keyName = SDL_GetScancodeName(static_cast<SDL_Scancode>(key));
		ImGui::InputText(lable, (char*)keyName, strlen(keyName), ImGuiInputTextFlags_ReadOnly);
		if (UIManager::ElementClicked())
		{
			focused = true;
		}
	}
}

bool ComponentEngine::UIManager::EdiableText(std::string & text, char *& temp_data, int max_size, bool editable)
{
	if (temp_data == nullptr || !editable)
	{
		ImGui::Text(text.c_str());
		if (ElementClicked())
		{
			temp_data = new char[max_size + 1];
			temp_data[max_size] = '\0';
			for (int i = 0; i < max_size; i++)
			{
				if (i < text.size())
				{
					temp_data[i] = /*m_current_scene_focus.entity->GetName()*/text.at(i);
				}
				else
				{
					temp_data[i] = '\0';
				}
			}
		}
	}
	else
	{
		ImGui::SetKeyboardFocusHere();
		ImGui::CaptureKeyboardFromApp(true);
		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_AlwaysInsertMode;
		bool done = ImGui::InputText("##data", temp_data, max_size, flags);
		if (done)
		{
			text = std::string(temp_data);
			delete temp_data;
			temp_data = nullptr;
			return true;
		}
	}
	return false;
}



UIManager::UIManager(Engine* engine) : m_engine(engine)
{

}

ComponentEngine::UIManager::~UIManager()
{
	for (auto& b : m_bases)
	{
		delete b;
	}
	for (auto&e : m_menu_elements)
	{
		delete e;
	}
}

void ComponentEngine::UIManager::AddElement(UIBase * base)
{
	base->m_engine = m_engine; // Add direct engine refrence to ui base
	base->m_manager = this; // Add direct manager refrence to ui base
	m_bases.push_back(base);
}

void ComponentEngine::UIManager::AddMenuElement(MenuElement * element)
{
	m_menu_elements.push_back(element);
}

void ComponentEngine::UIManager::Render()
{
	ImGui::NewFrame();
	PlayState state = m_engine->GetPlayState();
	DockSpace();
	RenderMainMenu();
	for (auto& b : m_bases)
	{
		if ((b->GetDisplayFlags() & state) == state)
		{
			b->Render();
		}
	}
	ImGui::Render();
}

CurrentSceneFocus & ComponentEngine::UIManager::GetCurrentSceneFocus()
{
	return m_current_scene_focus;
}

void ComponentEngine::UIManager::ResetSceneFocus()
{
	m_current_scene_focus.entity = nullptr;

	if (m_current_scene_focus.entity_temp_name != nullptr)
	{
		delete m_current_scene_focus.entity_temp_name;
		m_current_scene_focus.entity_temp_name = nullptr;
	}
}

void ComponentEngine::UIManager::DockSpace()
{
	static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_PassthruDockspace;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (opt_flags& ImGuiDockNodeFlags_PassthruDockspace)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	static bool open = true;
	ImGui::Begin("Main DockSpace", &open, window_flags);
	ImGui::PopStyleVar();

	ImGui::PopStyleVar(2);

	// Dockspace
	ImGuiIO & io = ImGui::GetIO();
	if (io.ConfigFlags& ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
	}
	else
	{
		assert(0 && "Dockspace not enabled!!! Needed in experimental branch of imgui");
	}

	ImGui::End();
}

void ComponentEngine::UIManager::RenderMainMenu()
{

	if (ImGui::BeginMainMenuBar())
	{

		for (auto& e : m_menu_elements)
		{
			RenderMenuElement(e);
		}

		ImGui::EndMainMenuBar();
	}

}

void ComponentEngine::UIManager::RenderMenuElement(MenuElement * element)
{
	ImGui::PushID(element);
	if (element->GetChildren().size() > 0)
	{
		if (ImGui::BeginMenu(element->GetText()))
		{
			for (auto& c : element->GetChildren())
			{
				RenderMenuElement(c);
			}
			ImGui::EndMenu();
		}
	}
	else
	{

		if (ImGui::MenuItem(element->GetText(), NULL, false))
		{
			element->OnClick()();
		}
	}
	ImGui::PopID();
}
