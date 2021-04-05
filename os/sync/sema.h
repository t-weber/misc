/**
 * semaphore test
 * @author Tobias Weber
 * @date 28-mar-19
 * @license see 'LICENSE.EUPL' file
 */

#ifndef __SEMA_H__
#define __SEMA_H__


#include <atomic>
#include <mutex>
#include <condition_variable>


#ifndef USE_PTHREAD

/**
 * simple semaphore
 */
template<class t_int = unsigned int>
class Sema
{
public:
	Sema(t_int ctr) : m_ctr{ctr} {}
	~Sema() {}

	void acquire()
	{
		std::unique_lock _ul{m_mtxcond};
		m_cond.wait(_ul, [this]()->bool { return m_ctr > 0; });
		//while(!m_cond.wait_for(_ul, std::chrono::nanoseconds{10}, [this]()->bool { return m_ctr > 0; }));

		--m_ctr;
	}

	void release()
	{
		// lock mutex in case of spurious release of wait() in acquire()
		std::scoped_lock _sl{m_mtxcond};

		// are potential threads in the queue?
		if(m_ctr++ <= 0)
			m_cond.notify_one();
	}

	t_int get_counter() const
	{
		return m_ctr;
	}

private:
	std::atomic<t_int> m_ctr{0};
	std::condition_variable m_cond{};
	std::mutex m_mtxcond{};
};



#else
#pragma message("Using pthreads semaphore.")

#include <pthread.h>
#include <semaphore.h>


/**
 * simple semaphore using pthread
 */
template<class t_int>	// does nothing, just for compatibility
class Sema
{
public:
	Sema(unsigned int ctr) { ::sem_init(&m_sema, 0, ctr); }
	~Sema() { ::sem_destroy(&m_sema); }

	void acquire() { ::sem_wait(&m_sema); }
	void release() { ::sem_post(&m_sema); }

	int get_counter() const
	{
		int ctr = 0;
		::sem_getvalue(const_cast<::sem_t*>(&m_sema), &ctr);
		return ctr;
	}

private:
	::sem_t m_sema{};
};

#endif



#include <queue>

/**
 * semaphore with priority queue
 */
template<class t_int = unsigned int, class t_int_prio = int>
class SemaPrio
{
public:
	SemaPrio(t_int ctr) : m_ctr{ctr} {}

	void acquire(t_int_prio prio=0)
	{
		std::unique_lock _ul{m_mtxcond};
		m_prio.push(prio);

		m_cond.wait(_ul, [this, prio]()->bool
		{
			return m_ctr > 0 && (m_prio.top() == prio);
		});

		m_prio.pop();
		--m_ctr;
	}

	void release()
	{
		++m_ctr;

		// lock mutex in case of spurious release of wait() in acquire()
		std::scoped_lock _sl{m_mtxcond};
		m_cond.notify_all();
	}

	t_int get_counter() const
	{
		return m_ctr;
	}

private:
	std::atomic<t_int> m_ctr{0};
	std::condition_variable m_cond{};
	std::mutex m_mtxcond{};

	std::priority_queue<t_int_prio> m_prio{};
};


#endif
