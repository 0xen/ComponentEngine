#include <ComponentEngine\UI\Threading.hpp>
#include <ComponentEngine\Engine.hpp>

#include <vector>

using namespace ComponentEngine;

ComponentEngine::ThreadingWindow::ThreadingWindow(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open) :
	UIBase(title, flags, displayFlags, open)
{

}

void ComponentEngine::ThreadingWindow::Contents()
{
	/*
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
	}*/


	{
		std::vector<WorkerTask*>& tasks = Engine::Singlton()->GetThreadManager()->GetSchedualedTasks();

		std::unique_lock<std::mutex> acquire(Engine::Singlton()->GetThreadManager()->m_schedualed_task_lock);

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
