#include <ComponentEngine\ThreadManager.hpp>
#include <ComponentEngine\ThreadHandler.hpp>
#include <ComponentEngine\Engine.hpp>

using namespace ComponentEngine;

ThreadManager::ThreadManager(ThreadMode mode)
{


	unsigned int thread_count = (unsigned int)(((float)std::thread::hardware_concurrency() * 1.5f) + 0.5f);

	for (unsigned int i = 0; i < thread_count; i++)
	{
		m_threads.push_back(new WorkerThread(this, mode));
	}

	m_mode = mode;

}

bool ThreadManager::GetTask(WorkerTask& task)
{
	WorkerTask* poolTask = nullptr;
	{
		std::unique_lock<std::mutex> acquire(m_task_pool_lock);
		if (m_task_pool.size() == 0) return false;

		poolTask = m_task_pool[0];
		poolTask->queued = false;
		m_task_pool.erase(m_task_pool.begin());
	}


	task = *poolTask;
	if (poolTask->type == TaskType::Single)
	{
		delete poolTask;
	}
	return true;
}

void ThreadManager::ChangeMode(ThreadMode mode)
{
	m_mode = mode;
	for (auto thread : m_threads)
	{
		thread->ChangeMode(mode);
	}
}

void ThreadManager::AddTask(std::function<void(float)> funcPtr, std::string name)
{
	WorkerTask* newTask = new WorkerTask();
	newTask->funcPtr = funcPtr;
	newTask->type = TaskType::Single;
	newTask->name = name;
	std::unique_lock<std::mutex> acquire(m_task_pool_lock);
	m_task_pool.push_back(newTask);
}

void ThreadManager::AddTask(std::function<void(float)> funcPtr, unsigned int ups, std::string name)
{
	WorkerTask* newTask = new WorkerTask();
	newTask->funcPtr = funcPtr;
	newTask->ups = ups;
	newTask->name = name;
	newTask->deltaTime = 1.0f / ups;
	newTask->type = TaskType::Multi;
	std::unique_lock<std::mutex> acquire(m_schedualed_task_lock);
	m_schedualed_tasks.push_back(newTask);
}

void ThreadManager::Update()
{

	float delta = Engine::Singlton()->GetLastThreadTime();

	std::unique_lock<std::mutex> acquire(m_schedualed_task_lock);


	for (int i = 0 ; i < m_schedualed_tasks.size(); ++i)
	{
		WorkerTask*& task = m_schedualed_tasks[i];
		task->deltaTime += delta;
		if (task->deltaTime > (1.0f / task->ups) && !task->queued)
		{
			task->queued = true;
			task->lastDelta = task->deltaTime;
			task->deltaTime = 0;
			{
				std::unique_lock<std::mutex> acquire(m_task_pool_lock);
				m_task_pool.push_back(task);
			}
		}
	}
	if (m_mode == Joined)
	{
		WorkerTask task;
		while (GetTask(task))
		{
			task.funcPtr(delta);
		}
	}


}

std::vector<WorkerThread*>& ThreadManager::GetThreads()
{
	return m_threads;
}

std::vector<WorkerTask*>& ThreadManager::GetSchedualedTasks()
{
	return m_schedualed_tasks;
}

WorkerThread::WorkerThread(ThreadManager* thread_manager, ThreadMode mode)
{
	m_delta_time = SDL_GetPerformanceCounter();
	m_seccond_delta = 0;
	m_thread_activity.resize(20);
	m_mode = mode;
	m_thread_manager = thread_manager;
	if (mode == ThreadMode::Threading)thread = new std::thread(&WorkerThread::Loop, this);
}

void WorkerThread::Loop()
{
	WorkerTask task;
	do{
		float lastDelta = GetDeltaTime();
		if (m_thread_manager->GetTask(task))
		{
			task.funcPtr(task.lastDelta);
			float end = GetDeltaTime();
			m_active_time += end;
			lastDelta += end;
		}
		else if (m_mode == ThreadMode::Threading)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			lastDelta += GetDeltaTime();
		}

		m_seccond_delta += lastDelta;
		if (m_seccond_delta > 1.0f)
		{
			std::unique_lock<std::mutex> acquire(m_worker_lock);
			memcpy(m_thread_activity.data(), m_thread_activity.data() + 1, sizeof(float) * 19);
			m_thread_activity[19] = m_active_time;
			m_active_time = 0;
			m_seccond_delta = 0;
		}

	} while (Running());
	/*if (thread == nullptr && m_mode == ThreadMode::Threading )
	{
		thread = new std::thread(&WorkerThread::Loop, this);
	}
	else if (thread != nullptr && m_mode == ThreadMode::Joined)
	{
		thread->join();
		delete thread;
	}*/
}

void WorkerThread::ChangeMode(ThreadMode mode)
{
	{
		std::unique_lock<std::mutex> acquire(m_worker_lock);
		m_mode = mode;
	}
	if (mode == ThreadMode::Threading)
	{
		thread = new std::thread(&WorkerThread::Loop, this);
	}
	else
	{
		if (thread != nullptr)
		{
			thread->join();
			delete thread;
			thread = nullptr;
		}
	}
}

std::vector<float> WorkerThread::GetThreadActivity()
{
	std::unique_lock<std::mutex> acquire(m_worker_lock);
	return m_thread_activity;
}

float WorkerThread::GetDeltaTime()
{

	std::thread::id id = std::this_thread::get_id();
	Uint64 now = SDL_GetPerformanceCounter();


	Uint64 last = m_delta_time;
	m_delta_time = now;
	float temp = static_cast<float>((float)(now - last) / SDL_GetPerformanceFrequency());
	return temp;
}

bool WorkerThread::Running()
{
	std::unique_lock<std::mutex> acquire(m_worker_lock);
	return m_mode == ThreadMode::Threading;
}
