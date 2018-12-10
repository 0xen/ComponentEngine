#include <ComponentEngine\ThreadHandler.hpp>
#include <ComponentEngine\Engine.hpp>

ThreadHandler::ThreadHandler(int ups)
{
	m_threading = false;
	m_running = false;
	m_toggling_state = false;
	m_ups = ups;
}

ThreadHandler::~ThreadHandler()
{
	if(thread!=nullptr) delete thread;
}

void ThreadHandler::StartThread()
{
	if (!GetThreading())
	{
		SetThreading(true);
		SetTogglingState(false);
		thread = new std::thread(&ThreadHandler::ThreadMain, this);
		m_id = thread->get_id();
	}
}

void ThreadHandler::Join()
{
	if (GetThreading())
	{
		SetTogglingState(true);
		thread->join();
		delete thread;
		thread = nullptr;
		SetThreading(false);
	}
}

bool ThreadHandler::Joined()
{
	return !GetThreading();
}

std::thread::id & ThreadHandler::GetID()
{
	return m_id;
}

void ThreadHandler::ThreadMain()
{
	SetTogglingState(false);
	if (!GetRunning())
	{
		SetRunning(true);
		Initilize();
	}
	while (!GetTogglingState() && ComponentEngine::Engine::Singlton()->Running(m_ups))
	{
		Loop();
	}
	if (!GetTogglingState())
	{
		Cleanup();
	}
	SetTogglingState(false);
}

bool ThreadHandler::GetThreading()
{
	m_thread_lock.lock();
	bool res = m_threading;
	m_thread_lock.unlock();
	return res;
}

bool ThreadHandler::GetRunning()
{
	m_thread_lock.lock();
	bool res = m_running;
	m_thread_lock.unlock();
	return res;
}

bool ThreadHandler::GetTogglingState()
{
	m_thread_lock.lock();
	bool res = m_toggling_state;
	m_thread_lock.unlock();
	return res;
}

void ThreadHandler::SetThreading(bool res)
{
	m_thread_lock.lock();
	m_threading = res;
	m_thread_lock.unlock();
}

void ThreadHandler::SetRunning(bool res)
{
	m_thread_lock.lock();
	m_running = res;
	m_thread_lock.unlock();
}

void ThreadHandler::SetTogglingState(bool res)
{
	m_thread_lock.lock();
	m_toggling_state = res;
	m_thread_lock.unlock();
}

