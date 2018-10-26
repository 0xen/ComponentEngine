#include <ComponentEngine\ThreadHandler.hpp>

ThreadHandler::ThreadHandler() : std::thread()
{
}

ThreadHandler::ThreadHandler(void(*entry_point)()) : thread(entry_point)
{

}

ordered_lock & ThreadHandler::ThreadLock()
{
	return m_thread_lock;
}

