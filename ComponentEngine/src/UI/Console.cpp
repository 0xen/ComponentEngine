#include <ComponentEngine\UI\Console.hpp>
#include <ComponentEngine\Engine.hpp>
#include <mutex>

using namespace ComponentEngine;

ComponentEngine::Console::Console(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open) :
	UIBase(title, flags, displayFlags, open)
{

}

void ComponentEngine::Console::Contents()
{
	
	std::lock_guard<std::mutex> guard(m_engine->GetLock(EngineLock::CONSOLE));

	std::vector<ConsoleMessage>& console = m_engine->GetConsoleMessages();
	for (int i = 0; i < console.size(); i++)
	{
		ImGui::PushID(i);
		ConsoleMessage& message = console[i];
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
