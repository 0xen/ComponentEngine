#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <functional>

#include <SDL.h>
#include <SDL_syswm.h>

#include <condition_variable>
#include <queue>

enum ThreadMode
{
	Threading,
	Joined
};

enum TaskType
{
	Single, // Single time
	Multi   // Multi
};

struct WorkerTask
{
	TaskType type;
	std::function<void(float)> funcPtr;
	unsigned int ups;
	float deltaTime; // How long since last update
	float lastDelta; // How long it took before the function was calld
	bool queued = false;
	std::string name;
};


class ThreadManager;
class WorkerThread
{
public:
	WorkerThread(ThreadManager* thread_manager, ThreadMode mode);
	void Loop();
	void ChangeMode(ThreadMode mode);
	std::vector<float> GetThreadActivity();
	float GetDeltaTime();
private:
	bool Running();


	// SDL time
	Uint64 m_delta_time;
	// Time since last seccond
	float m_seccond_delta;


	float m_active_time;
	ThreadManager* m_thread_manager;
	ThreadMode m_mode;
	std::thread* thread;
	std::mutex m_worker_lock;
	std::vector<float> m_thread_activity;
};


class ThreadManager
{
public:
	std::mutex m_schedualed_task_lock;
	std::mutex m_task_pool_lock;
	std::mutex m_activity_lock;


	ThreadManager(ThreadMode mode);

	bool GetTask(WorkerTask& task);

	void ChangeMode(ThreadMode mode);

	void AddTask(std::function<void(float)> funcPtr, std::string name = "");

	void AddTask(std::function<void(float)> funcPtr, unsigned int ups, std::string name = "");

	void Update();

	std::vector<WorkerThread*>& GetThreads();

	std::vector<WorkerTask*>& GetSchedualedTasks();


	std::vector<float> GetActivity();
private:
	float GetDeltaTime();
	ThreadMode m_mode;
	std::vector<WorkerTask*> m_schedualed_tasks;
	std::vector<WorkerTask*> m_task_pool;
	std::vector<WorkerThread*> m_threads;
	// SDL time
	Uint64 m_delta_time;
	// Time since last seccond
	float m_seccond_delta;
	float m_active_time;
	std::vector<float> m_thread_activity;

};