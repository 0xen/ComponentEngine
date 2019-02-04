#pragma once

#include <thread>
#include <mutex>


#include <condition_variable>
#include <queue>

// SInspration for using condition variables from https://stackoverflow.com/questions/14792016/creating-a-lock-that-preserves-the-order-of-locking-attempts-in-c11
struct condition_packet
{
	std::condition_variable con;
	std::thread::id t_id;
	unsigned int recursions;
};
class ordered_lock
{
	std::queue<condition_packet> cvar;
	std::mutex cvar_lock;
public:
	ordered_lock() {};
	void lock()
	{
		std::unique_lock<std::mutex> acquire(cvar_lock);
		bool empty = cvar.empty();
		// If the current thread who is requesting access is already the one who the lock is prioritised to, ignore the lock request and let them continue as they have priority
		// This fixes recursion
		if (!empty && cvar.front().t_id == std::this_thread::get_id())
		{
			cvar.front().recursions++;
			return;
		}
		cvar.emplace();
		cvar.back().recursions = 1;
		cvar.back().t_id = std::this_thread::get_id();
		if (!empty)
		{
			cvar.back().con.wait(acquire);
		}
	}
	void unlock()
	{
		std::unique_lock<std::mutex> acquire(cvar_lock);
		if (cvar.empty()) return;

		condition_packet& packet = cvar.front();
		if (packet.t_id != std::this_thread::get_id())
		{
			return;
		}

		packet.recursions--;

		if (packet.recursions != 0)return;



		cvar.pop();
		if (!cvar.empty())
		{
			cvar.front().con.notify_one();
		}
	}
};

/*
class ThreadHandler : public std::thread
{
public:
	ThreadHandler();
	ThreadHandler(void(*entry_point)());
	ordered_lock& ThreadLock();
private:
	ordered_lock m_thread_lock;
};
*/
