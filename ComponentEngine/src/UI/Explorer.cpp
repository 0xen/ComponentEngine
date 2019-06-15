#include <ComponentEngine\UI\Explorer.hpp>
#include <ComponentEngine\Engine.hpp>
#include <mutex>

using namespace ComponentEngine;

ComponentEngine::Explorer::Explorer(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open) :
	UIBase(title, flags, displayFlags, open)
{

}

void ComponentEngine::Explorer::LoadFolder(Folder & folder)
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

void ComponentEngine::Explorer::RendererFolder(Folder & folder)
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
				UIManager::DropPayload("File", childFiles.shortForm.c_str(), &childFiles, sizeof(FileForms));
			}
			ImGui::TreePop();
			ImGui::PopID();

		}


		ImGui::TreePop();
	}
	ImGui::PopID();
}

void ComponentEngine::Explorer::RendererFolder(Folder & folder, std::function<void(const char* path)> doubleClickCallBack)
{
	LoadFolder(folder);

	ImGui::PushID(&folder);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

	bool open = ImGui::TreeNodeEx("Folder", flags, "%s", folder.path.shortForm.c_str());

	if (open)
	{
		for (auto& childFolder : folder.folders)
		{
			RendererFolder(childFolder, doubleClickCallBack);
		}

		for (auto& childFiles : folder.files)
		{
			ImGui::PushID(childFiles.longForm.c_str());
			ImGui::TreeNodeEx("File", ImGuiTreeNodeFlags_Leaf, "%s", childFiles.shortForm.c_str());
			if (UIManager::ElementClicked(false))
			{
				doubleClickCallBack(childFiles.longForm.c_str());
			}

			ImGui::TreePop();
			ImGui::PopID();

		}


		ImGui::TreePop();
	}
	ImGui::PopID();
}

void ComponentEngine::Explorer::Contents()
{
	if (m_sceneFolder.path.longForm != Engine::Singlton()->GetCurrentSceneDirectory())
	{
		m_sceneFolder = Folder();
		m_sceneFolder.path.longForm = Engine::Singlton()->GetCurrentSceneDirectory();
		m_sceneFolder.path.shortForm = "/";
		LoadFolder(m_sceneFolder);
	}
	RendererFolder(m_sceneFolder);
}
