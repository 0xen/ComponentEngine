#pragma once

#include <thread>
#include <mutex>
#include <functional>
#include <vector>

#include <SDL.h>
#include <SDL_syswm.h>

#include <cmath>
#include <thread>
#include <future>

#include <condition_variable>
#include <queue>

enum TaskType
{
	Single, // Single time
	Multi   // Multi
};

struct WorkerTask
{
	WorkerTask();
	
	std::packaged_task<void(float)> task;


	unsigned int ups;
	float deltaTime; // How long since last update
	float lastDelta; // How long it took before the function was calld
	float acumalitiveTime; // All delta times added together from a frame to be averaged out
	unsigned int totalCount = 0; // Running total count of runs
	bool queued = false;
	TaskType type;
	std::string name;
	std::vector<float> taskActivity;
};

class ThreadManager
{
public:
	std::mutex m_schedualed_task_lock;
	std::mutex m_task_pool_lock;


	ThreadManager();

	bool GetTask(WorkerTask*& task);

	void AddTask(std::function<void(float)> funcPtr, std::string name = "");

	void AddTask(std::function<void(float)> funcPtr, unsigned int ups, std::string name = "");

	void Update();

	unsigned int ThreadCount();

	std::vector<WorkerTask*>& GetSchedualedTasks();


	std::vector<float> GetActivity();


private:

	void Worker(unsigned int id);

	float GetDeltaTime();
	std::vector<WorkerTask*> m_schedualed_tasks;
	std::vector<WorkerTask*> m_task_pool;

	std::vector<std::thread> m_threads;

	// SDL time
	Uint64 m_delta_time;
	// Time since last seccond
	float m_seccond_delta;
	float m_active_time;
	float m_delta_update; // Count up all time in update function
	std::vector<float> m_thread_activity;

	// Used to check the state of the is running mutex
	std::mutex* m_workerlockGuard;
	std::condition_variable* m_workReady;
	std::vector<bool> m_haveWork;
	std::vector<WorkerTask*> m_workerTask;

	unsigned int m_workerCount;
};