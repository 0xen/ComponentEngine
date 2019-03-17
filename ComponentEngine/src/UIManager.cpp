#include <ComponentEngine\UIManager.hpp>
#include <ComponentEngine\Components\UI.hpp>


#include <imgui.h>

const unsigned int ComponentEngine::UIManager::SCENE_HIERARCHY = 0;
const unsigned int ComponentEngine::UIManager::COMPONENT_HIERARCHY = 1;
const unsigned int ComponentEngine::UIManager::EXPLORER = 2;
const unsigned int ComponentEngine::UIManager::THREADING_MANAGER = 3;
const unsigned int ComponentEngine::UIManager::CONSOLE = 4;
const unsigned int ComponentEngine::UIManager::PLAY_PAUSE = 5;
const unsigned int ComponentEngine::UIManager::ABOUT = 6;

ComponentEngine::UIManager::UIManager(Engine* engine) : m_engine(engine)
{
	m_indestructable_component_id = engine->GetEntityManager().GetComponentIndex<Indestructable>();
	m_open[SCENE_HIERARCHY] = true;
	m_open[EXPLORER] = true;
	m_open[COMPONENT_HIERARCHY] = true;
	m_open[THREADING_MANAGER] = true;
	m_open[CONSOLE] = true;
	m_open[ABOUT] = false;

	m_fullscreenOnPlay = true;
	m_thread_time_update_delay = 0.0f;
}

void ComponentEngine::UIManager::Render()
{
	ImGui::NewFrame();
	//ImGui::ShowTestWindow();

	DockSpace();

	PlayPause();

	
	RenderMainMenu();

	if (Engine::Singlton()->GetPlayState() != PlayState::Playing || !m_fullscreenOnPlay)
	{


		if (m_open[THREADING_MANAGER])ThreadingWindow();
		if (m_open[CONSOLE])AddConsole();
		if (m_open[SCENE_HIERARCHY]) RenderSceneHierarchy();
		if (m_open[COMPONENT_HIERARCHY]) RenderComponentHierarchy();
		if (m_open[EXPLORER]) RendererExplorer();
		if (m_open[ABOUT]) AboutPage();
	}


	ImGui::Render();
}

bool ComponentEngine::UIManager::IsWindowFocused()
{
	return ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
}

void ComponentEngine::UIManager::RenderMainMenu()
{
	bool dissableInPlay = Engine::Singlton()->GetPlayState() != PlayState::Playing || !m_fullscreenOnPlay;



	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{

			if (ImGui::BeginMenu("Add", dissableInPlay))
			{

				if (ImGui::MenuItem("Entity", NULL, false, dissableInPlay))
				{
					AddEntity(m_current_scene_focus.entity);
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();




			if (ImGui::MenuItem("Reload Scene"))
			{

				// spawn a new task to load in the new entitys
				m_engine->GetThreadManager()->AddTask([&](float frameTime) {

					m_engine->m_logic_lock.lock();
					m_engine->GetRendererMutex().lock();

					m_engine->GetEntityManager().Clear();
					// Load in the scene
					m_engine->LoadScene(m_engine->GetCurrentScene().c_str(), false);
					m_engine->UpdateScene();

					m_engine->GetRendererMutex().unlock();
					m_engine->m_logic_lock.unlock();

				});
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
			{
				m_engine->GetRendererMutex().lock();
				m_engine->RequestStop();
				m_engine->GetRendererMutex().unlock();
			}
			ImGui::EndMenu();
		}

		
		if (ImGui::BeginMenu("Edit"))
		{
			bool validEntity = m_current_scene_focus.entity != nullptr;
			if (ImGui::MenuItem("Remove Entity", NULL, false, validEntity && dissableInPlay))
			{
				DestroyEntity(m_current_scene_focus.entity);
				ResetSceneFocusEntity();
			}
			//ImGui::Separator();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window", dissableInPlay))
		{
			ImGui::MenuItem("Scene Hierarchy", NULL, &m_open[SCENE_HIERARCHY]);
			ImGui::MenuItem("Component Hierarchy", NULL, &m_open[COMPONENT_HIERARCHY]);
			ImGui::MenuItem("Explorer", NULL, &m_open[EXPLORER]);
			ImGui::MenuItem("Threading Manager", NULL, &m_open[THREADING_MANAGER]);
			ImGui::MenuItem("Console", NULL, &m_open[CONSOLE]);
			ImGui::EndMenu();
		}

		ImGui::MenuItem("About", NULL, &m_open[ABOUT], dissableInPlay);
		/*if (ImGui::BeginMenu("About", dissableInPlay))
		{
			ImGui::EndMenu();
		}*/

		ImGui::EndMainMenuBar();
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
#include <imgui_internal.h>
void ComponentEngine::UIManager::PlayPause()
{
	//static int window_height = 370;
	//ImGui::SetNextWindowSize(ImVec2(420, window_height));
	ImGuiStyle& style = ImGui::GetStyle();
	int titlebarHeight = ImGui::GetFontSize() + (style.FramePadding.y * 2);
	//style.tit
	static const int windowWidth = 47;
	ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2)- windowWidth, titlebarHeight));
	if (ImGui::Begin("Play Pause", &m_open[PLAY_PAUSE], ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking))
	{
		if(Engine::Singlton()->GetPlayState() == PlayState::Playing)
		{
			if (ImGui::Button("||"))
			{
				Engine::Singlton()->SetPlayState(PlayState::Paused);
			}
		}
		else
		{
			if (ImGui::ArrowButton("##left", ImGuiDir_Right))
			{
				Engine::Singlton()->SetPlayState(PlayState::Playing);
			}
			ImGui::SameLine();
			ImGui::Checkbox("Fullscreen on play", &m_fullscreenOnPlay);
		}
	}
	ImGui::End();
}

void ComponentEngine::UIManager::AboutPage()
{
	//static int window_height = 370;
	//ImGui::SetNextWindowSize(ImVec2(420, window_height));
	ImGuiStyle& style = ImGui::GetStyle();
	int titlebarHeight = ImGui::GetFontSize() + (style.FramePadding.y * 2);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, titlebarHeight + (ImGui::GetIO().DisplaySize.y) / 3));
	ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
	if (ImGui::Begin("About", &m_open[ABOUT], ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking))
	{

		ImGui::Text("Developed by: John Green");

		ImGui::Text("");
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), "Special Thanks To");
		ImGui::Text("");
		{
			ImGui::Text("Vulkan API:");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "https://www.khronos.org/vulkan/");
		}
		{
			ImGui::Text("ImGui: ");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "https://github.com/ocornut/imgui");
		}
		{
			ImGui::Text("Bullet3: ");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "https://pybullet.org/");
		}
		{
			ImGui::Text("SDL2: ");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "https://www.libsdl.org/");
		}
		{
			ImGui::Text("GLM: ");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "https://glm.g-truc.net/");
		}
		{
			ImGui::Text("PugiXML: ");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "https://pugixml.org/");
		}
		{
			ImGui::Text("LodePNG: ");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "https://lodev.org/lodepng/");
		}
		{
			ImGui::Text("Tiny OBJ Loader: ");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "https://github.com/syoyo/tinyobjloader");
		}
		ImGui::Text("");
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), "Laurent Noel");
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), "Nicky Danino");

	}
	ImGui::End();
}


void ComponentEngine::UIManager::RendererExplorer()
{
	static int window_height = 370;
	ImGui::SetNextWindowSize(ImVec2(420, window_height));
	if (ImGui::Begin("Explorer", &m_open[EXPLORER], ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
	{
		if (sceneFolder.path.longForm != Engine::Singlton()->GetCurrentSceneDirectory())
		{
			sceneFolder = Folder();
			sceneFolder.path.longForm = Engine::Singlton()->GetCurrentSceneDirectory();
			sceneFolder.path.shortForm = "/";
			LoadFolder(sceneFolder);
		}
		RendererFolder(sceneFolder);
	}
	ImGui::End();
}

void ComponentEngine::UIManager::RendererFolder(Folder & folder)
{
	LoadFolder(folder);

	ImGui::PushID(&folder);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
	
	bool open = ImGui::TreeNodeEx("Folder", flags, "%s", folder.path.shortForm.c_str());


	if (open)
	{

		for (auto& childFolder : folder.folders)
		{
			RendererFolder(childFolder);
		}

		for (auto& childFiles : folder.files)
		{
			ImGui::PushID(childFiles.longForm.c_str());
			ImGui::TreeNodeEx("File", ImGuiTreeNodeFlags_Leaf, "%s", childFiles.shortForm.c_str());


			{ // File drag
				DropPayload("File", childFiles.shortForm.c_str(), &childFiles, sizeof(FileForms));
			}
			ImGui::TreePop();
			ImGui::PopID();

		}


		ImGui::TreePop();
	}
	ImGui::PopID();
}

void ComponentEngine::UIManager::RenderSceneHierarchy()
{
	static int window_height = 370;
	ImGui::SetNextWindowSize(ImVec2(420, window_height));
	if (ImGui::Begin("Scene Hierarchy", &m_open[SCENE_HIERARCHY], ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
	{
		EntityManager& em = m_engine->GetEntityManager();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

		{

			ImGui::BeginChild("Child1", ImVec2(0.0f,0.0f), false);
			{

				bool open = ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick);

				AddEntityDialougeMenu(nullptr);

				if (ElementClicked())
				{
					ResetSceneFocusEntity();
					m_current_scene_focus.entity = nullptr;
				}

				const ImGuiPayload* payload = nullptr;
				if (DropTarget("Entity", payload))
				{
					IM_ASSERT(payload->DataSize == sizeof(Entity*));
					Entity* child = *(Entity**)payload->Data;

					child->GetComponent<Transformation>().SetParent(nullptr);
				}

				if (open)
				{

					for (auto entity : em.GetEntitys())
					{
						if (entity->GetComponent<Transformation>().GetParent() == nullptr)
						{
							RenderEntityTreeNode(entity);
						}
					}


					ImGui::TreePop();
				}
				ImGui::EndChild();
			}
		}

		ImGui::Columns(1);
		ImGui::PopStyleVar();

	}
	ImGui::End();
}

void ComponentEngine::UIManager::RenderComponentHierarchy()
{

	static int window_height = 370;
	ImGui::SetNextWindowSize(ImVec2(420, window_height));
	static bool v_borders = false;
	if (ImGui::Begin("Components", &m_open[COMPONENT_HIERARCHY], ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
	{
		EntityManager& em = m_engine->GetEntityManager();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

		{
			

			ImGui::BeginChild("Child2", ImVec2(0.0f, 0.0f), false);
			{
				{
					if (m_current_scene_focus.entity != nullptr)
					{
						bool entityBeenDestroyed = false;
						// Title
						{
							ImGui::Columns(2, NULL, v_borders);
							ImGui::Separator();
							ImGui::PushID(m_current_scene_focus.entity); // Button scope 1
							bool hasIndestructable = m_current_scene_focus.entity->HasComponent<Indestructable>();

							entityBeenDestroyed = !hasIndestructable && ImGui::SmallButton("X");

							if (entityBeenDestroyed)
							{
								// Loop through all children and destroy them
								DestroyEntity(m_current_scene_focus.entity);

								ResetSceneFocusEntity();
								//ResetSceneFocusComponent();
							}
							else
							{
								if (!hasIndestructable)ImGui::SameLine();
								ImGui::Text("Entity");
								ImGui::NextColumn();

								EdiableText(m_current_scene_focus.entity->GetName(), m_current_scene_focus.entity_temp_name, 20, !hasIndestructable);

								ImGui::Columns(1);
								ImGui::Separator();
							}
							ImGui::PopID();



						}
						if (!entityBeenDestroyed)
						{// Body
							RenderEntity(m_current_scene_focus.entity);
						}

						if (!entityBeenDestroyed)
						{ // Components
							m_current_scene_focus.entity->ForEach<UI>([this](Entity* entity, BaseComponentWrapper& wrapper, UI* ui)
							{
								bool componentBeenDestroyed = false;
								// Title
								{
									ImGui::Columns(2);
									ImGui::Separator();
									ImGui::PushID(wrapper.GetComponentPtr());

									bool hasIndestructable = m_current_scene_focus.entity->HasComponent<Indestructable>() || wrapper.GetName() == "Transformation";

									componentBeenDestroyed = !hasIndestructable && ImGui::SmallButton("X");

									if (componentBeenDestroyed)
									{
										m_current_scene_focus.entity->RemoveComponent(wrapper.GetComponentPtr());
									}
									else
									{
										if (!hasIndestructable)ImGui::SameLine();
										ImGui::Text("Component");
										ImGui::NextColumn();
										ImGui::Text(wrapper.GetName().c_str());
										ImGui::Columns(1);
										ImGui::Separator();
									}
									ImGui::PopID();
								}
								if(!componentBeenDestroyed)
								{ // Body
									ui->Display();
								}
							});
						}

						




					}
				}

				ImGui::EndChild();
			}


		}

		ImGui::Columns(1);
		ImGui::PopStyleVar();


	}
	ImGui::End();

}

void ComponentEngine::UIManager::ThreadingWindow()
{


	static int window_height = 370;
	ImGui::SetNextWindowSize(ImVec2(420, window_height));
	if (ImGui::Begin("Worker Threads", &m_open[THREADING_MANAGER], ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
	{


		if (Engine::Singlton()->GetThreadManager()->GetThreadMode() == ThreadMode::Threading)
		{
			std::vector<WorkerThread*>& threads = Engine::Singlton()->GetThreadManager()->GetThreads();

			for (int i = 0; i < threads.size(); i++)
			{
				ImGui::PushID(i);
				WorkerThread* thread = threads[i];
				ImGui::Text("Worker #%i", i);
				
				ImGui::PlotLines("", thread->GetThreadActivity().data(), thread->GetThreadActivity().size(), 0, "", 0.0f, 1.0f);

				ImGui::SameLine();

				ImGui::Text("%i%%", (int)(thread->GetThreadActivity()[thread->GetThreadActivity().size() - 1] * 100));

				ImGui::PopID();
			}
		}
		else
		{

			{
				ImGui::Text("Main Thread");

				std::vector<float> activity = Engine::Singlton()->GetThreadManager()->GetActivity();
				ImGui::PlotLines("", activity.data(), activity.size(), 0, "", 0.0f, 1.0f);

				ImGui::SameLine();

				ImGui::Text("%i%%", (int)(activity[activity.size() - 1] * 100));

			}

		}

		{
			std::vector<WorkerTask*>& tasks = Engine::Singlton()->GetThreadManager()->GetSchedualedTasks();

			for (int i = 0; i < tasks.size(); i++)
			{
				ImGui::PushID(i);
				WorkerTask*& task = tasks[i];
				ImGui::Text("Task: %s", task->name.c_str());
				int ups = (int)task->ups;
				if (ImGui::InputInt("", &ups, 1, 5, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (ups < 1) ups = 1;
					task->ups = ups;
				}

				float processTime = task->taskActivity[task->taskActivity.size() - 1];

				ImGui::PlotLines("", task->taskActivity.data(), task->taskActivity.size(), 0, "", 0.0f, task->ups);

				ImGui::SameLine();
				ImGui::Text("UPS:%.2f", processTime);



				ImGui::PopID();
			}
		}



	}
	ImGui::End();


}


void ComponentEngine::UIManager::RenderEntityTreeNode(Entity * entity)
{
	ImGui::PushID(entity);
	Transformation& entityTeansformation = entity->GetComponent<Transformation>();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (!entityTeansformation.HasChildren())
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}
	if (entity == m_current_scene_focus.entity)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	bool open = ImGui::TreeNodeEx("GameObject", flags, "%s", entity->GetName().c_str());

	AddEntityDialougeMenu(entity);

	if (ElementClicked())
	{
		ResetSceneFocusEntity();
		m_current_scene_focus.entity = entity;
		//ResetSceneFocusComponent();
	}

	DropPayload("Entity", entity->GetName().c_str(), &entity, sizeof(Entity*));



	const ImGuiPayload* payload = nullptr;
	if (DropTarget("Entity", payload))
	{
		IM_ASSERT(payload->DataSize == sizeof(Entity*));
		Entity* child = *(Entity**)payload->Data;

		bool notLooped = true;
		Entity* currentParent = entity->GetComponent<Transformation>().GetParent();
		while (currentParent != nullptr)
		{
			if (currentParent == child)
			{
				notLooped = false;
				break;
			}
			currentParent = currentParent->GetComponent<Transformation>().GetParent();
		}

		if (notLooped)
		{
			child->GetComponent<Transformation>().SetParent(entity);
		}

	}


	if (open)
	{

		std::vector<Transformation*> children = entityTeansformation.GetChildren();
		for (auto child : children)
		{
			ImGui::PushID(child->GetEntity());

			RenderEntityTreeNode(child->GetEntity());

			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}

void ComponentEngine::UIManager::RenderEntity(Entity * entity)
{
	bool hasIndestructable = m_current_scene_focus.entity->HasComponent<Indestructable>();

	ImGui::Text("Component Count:%i", entity->GetComponentCount());
	AddComponentDialougeMenu();
}

void ComponentEngine::UIManager::DestroyEntity(Entity * entity)
{
	Transformation& transformation = entity->GetComponent<Transformation>();
	if (transformation.HasChildren())
	{
		for (auto t : transformation.GetChildren())
		{
			DestroyEntity(t->GetEntity());
		}
	}
	entity->Destroy();
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
					temp_data[i] = m_current_scene_focus.entity->GetName().at(i);
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

void ComponentEngine::UIManager::ResetSceneFocusEntity()
{
	m_current_scene_focus.entity = nullptr;

	if (m_current_scene_focus.entity_temp_name != nullptr)
	{
		delete m_current_scene_focus.entity_temp_name;
		m_current_scene_focus.entity_temp_name = nullptr;
	}
}

// Use this after creating a component to add a right click menu to it
void ComponentEngine::UIManager::AddEntityDialougeMenu(Entity * parent)
{
	if (ImGui::BeginPopupContextItem("Add Entity Menu"))
	{
		if (ImGui::Selectable("Add Entity"))
		{
			AddEntity(parent);
		}
		ImGui::EndPopup();
	}
}

void ComponentEngine::UIManager::AddEntity(Entity* parent)
{
	EntityManager& em = m_engine->GetEntityManager();
	enteez::Entity* entity = em.CreateEntity("New Entity");
	Transformation& a = entity->AddComponent<Transformation>(entity)->Get();
	a.SetParent(parent);
}

void ComponentEngine::UIManager::AddComponentDialougeMenu()
{
	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponentPopup");
	if (ImGui::BeginPopup("AddComponentPopup"))
	{
		if (m_current_scene_focus.entity == nullptr) // No entity selected
		{
			ImGui::Text("No Entity Selected");
		}
		else // Entity selected
		{
			static std::string item_current = "Renderer";
			if (ImGui::BeginCombo("Component", item_current.c_str()))
			{
				for (auto it : m_engine->m_component_register)
				{
					if (it.second.default_initilizer != nullptr)
					{
						bool is_selected = (item_current == it.first);
						if (ImGui::Selectable(it.first.c_str(), is_selected))
						{
							item_current = it.first;
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			bool selected = ImGui::Button("Add");
			if (selected)
			{
				auto it = m_engine->m_component_register.find(item_current);
				if (it != m_engine->m_component_register.end())
				{
					if (it->second.default_initilizer != nullptr)
					{
						// In-case we replace the current component with a new one, we want to forget the old one now
						it->second.default_initilizer(*m_current_scene_focus.entity);
					}
				}
				ImGui::CloseCurrentPopup();
			}

		}


		ImGui::EndPopup();
	}
}

void ComponentEngine::UIManager::AddConsole()
{
	static int window_height = 370;
	ImGui::SetNextWindowSize(ImVec2(420, window_height));
	if (ImGui::Begin("Console", &m_open[CONSOLE], ImGuiWindowFlags_NoCollapse))
	{


		std::lock_guard<std::mutex> guard(m_engine->m_locks[m_engine->CONSOLE_LOCK]);
		for (int i = 0; i < m_engine->m_console.size(); i++)
		{
			ImGui::PushID(i);
			ConsoleMessage& message = m_engine->m_console[i];
			switch (message.state)
			{
			case Default:
				ImGui::Text(message.message.c_str(), message.count);
				break;
			case Info:
				ImGui::TextColored(ImVec4(0.5f, 1.0f, 1.0f, 1.0f), message.message.c_str(), message.count);
				break;
			case Warning:
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), message.message.c_str(), message.count);
				break;
			case Error:
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), message.message.c_str(), message.count);
				break;
			}

			ImGui::PopID();
		}



	}
	ImGui::End();
}

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

bool ComponentEngine::UIManager::DropTarget(const char * type,const ImGuiPayload *& payload)
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

void ComponentEngine::UIManager::LoadFolder(Folder & folder)
{
	if (folder.readFolder) return;
	folder.readFolder = true;
	//std::experimental::filesystem::file
	for (const auto & entry : std::experimental::filesystem::directory_iterator(folder.path.longForm))
	{
		if (std::experimental::filesystem::is_directory(entry.path()))
		{
			Folder childFolder;
			childFolder.path.GenerateFileForm(entry.path().string());
			folder.folders.push_back(childFolder);
		}
		else
		{
			FileForms file;
			file.GenerateFileForm(entry.path().string());
			folder.files.push_back(file);
		}
	}
}
