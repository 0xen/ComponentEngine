#include <ComponentEngine\ThreadManager.hpp>
#include <ComponentEngine\ThreadHandler.hpp>
#include <ComponentEngine\Engine.hpp>

using namespace ComponentEngine;

ThreadManager::ThreadManager()
{
	m_seccond_delta = 0;
	m_thread_activity.resize(20);
	m_seccond_delta = 0;
	m_delta_update = 0;
	m_delta_time = SDL_GetPerformanceCounter();

	unsigned int thread_count = (unsigned int)(((float)std::thread::hardware_concurrency() * 1.5f) + 0.5f);
	m_workerCount = thread_count;
	m_workerlockGuard = new std::mutex[m_workerCount];
	m_workReady = new std::condition_variable[m_workerCount];
	m_workerTask.resize(m_workerCount);
	m_haveWork.resize(m_workerCount);


	for (unsigned int i = 0; i < thread_count; i++)
	{
		m_threads.push_back(std::thread(&ThreadManager::Worker, this, i));
	}

}

bool ThreadManager::GetTask(WorkerTask*& task)
{
	WorkerTask* poolTask = nullptr;
	{
		std::unique_lock<std::mutex> acquire(m_task_pool_lock);
		if (m_task_pool.size() == 0) return false;

		poolTask = m_task_pool[0];
		m_task_pool.erase(m_task_pool.begin());
	}


	task = poolTask;
	return true;
}

void ThreadManager::AddTask(std::function<void(float)> funcPtr, std::string name)
{
	WorkerTask* newTask = new WorkerTask();
	newTask->task = std::packaged_task<void(float)>(funcPtr);


	newTask->name = name;
	newTask->type = TaskType::Single;
	std::unique_lock<std::mutex> acquire(m_task_pool_lock);
	m_task_pool.push_back(newTask);
}

void ThreadManager::AddTask(std::function<void(float)> funcPtr, unsigned int ups, std::string name)
{
	WorkerTask* newTask = new WorkerTask();
	newTask->queued = false;
	newTask->task = std::packaged_task<void(float)>(funcPtr);


	newTask->ups = ups;
	newTask->name = name;
	newTask->deltaTime = 1.0f / ups;
	newTask->type = TaskType::Multi;
	std::unique_lock<std::mutex> acquire(m_schedualed_task_lock);
	m_schedualed_tasks.push_back(newTask);
}

void ThreadManager::Update()
{

	float delta = GetDeltaTime() + m_delta_update;// Engine::Singlton()->GetLastThreadTime();
	m_delta_update = 0.0f;

	{ // Schedualed task

		// Calculate how much preformance this thread is using
		m_seccond_delta += delta;

		std::unique_lock<std::mutex> acquire(m_schedualed_task_lock);

		bool seccondElapsed = m_seccond_delta > 1.0f;

		// Loop through all tasks
		for (int i = 0; i < m_schedualed_tasks.size(); ++i)
		{
			WorkerTask*& task = m_schedualed_tasks[i];
			task->deltaTime += delta;
			if (task->deltaTime > (1.0f / task->ups) && !task->queued)
			{
				task->queued = true;
				task->lastDelta = task->deltaTime;
				task->deltaTime = 0;
				task->totalCount++;
				task->acumalitiveTime += task->lastDelta;
				{
					std::unique_lock<std::mutex> acquire(m_task_pool_lock);
					m_task_pool.push_back(task);
				}
			}

			if (seccondElapsed)
			{
				memcpy(task->taskActivity.data(), task->taskActivity.data() + 1, sizeof(float) * 19);
				float averageUPS = task->acumalitiveTime > 0 ? 1.0f / (task->acumalitiveTime / task->totalCount) : 0.0f;
				task->acumalitiveTime = 0.0f;
				task->totalCount = 0;
				task->taskActivity[19] = averageUPS;
			}
		}

		if (seccondElapsed)
		{
			memcpy(m_thread_activity.data(), m_thread_activity.data() + 1, sizeof(float) * 19);
			m_thread_activity[19] = m_active_time;
			m_active_time = 0;
			m_seccond_delta = 0;
		}
	}




	// Push tasks
	for (int i = 0; i < m_workerCount; ++i)
	{
		std::unique_lock<std::mutex> lock(m_workerlockGuard[i]);
		if(!m_haveWork[i])
		{
			WorkerTask* task;
			if(GetTask(task))
			{
				m_workerTask[i] = task;
				m_haveWork[i] = true;
			}
			m_workReady[i].notify_one(); // Tell worker
		}
	}

	/*
	// Wait for Workers
	for (int i = 0; i < m_workerCount; ++i)
	{
		std::unique_lock<std::mutex> lock(m_workerlockGuard[i]);
		if (m_haveWork[i])
		{
			// Wait until work is done
			m_workReady[i].wait(lock);
		}
	}
	*/

	

}

unsigned int ThreadManager::ThreadCount()
{
	return m_workerCount;
}

std::vector<WorkerTask*>& ThreadManager::GetSchedualedTasks()
{
	return m_schedualed_tasks;
}

std::vector<float> ThreadManager::GetActivity()
{
	//std::unique_lock<std::mutex> acquire(m_activity_lock);
	return m_thread_activity;
}

void ThreadManager::Worker(unsigned int id)
{
	while (true)
	{
		{
			// Guard use of haveWork from other thread
			std::unique_lock<std::mutex> lock(m_workerlockGuard[id]);
			while (!m_haveWork[id])
			{ // Wait for some work
				m_workReady[id].wait(lock);
			};
		}

		//m_workerTask[id]->funcPtr(m_workerTask[id]->lastDelta);
		m_workerTask[id]->task(m_workerTask[id]->lastDelta);
		m_workerTask[id]->task.reset();

		{ // Cleanup
			m_workerTask[id]->queued = false;
			if (m_workerTask[id]->type == TaskType::Single)
			{
				delete m_workerTask[id];
			}
		}

		{
			std::unique_lock<std::mutex> lock(m_workerlockGuard[id]);
			m_haveWork[id] = false;
		}
		m_workReady[id].notify_one(); // Tell main thread

	}
}

float ThreadManager::GetDeltaTime()
{
	Uint64 now = SDL_GetPerformanceCounter();
	Uint64 last = m_delta_time;
	m_delta_time = now;
	float temp = static_cast<float>((float)(now - last) / SDL_GetPerformanceFrequency());
	return temp;
}


WorkerTask::WorkerTask()
{
	taskActivity.resize(20);
}
