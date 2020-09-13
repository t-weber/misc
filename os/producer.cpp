/**
 * producer/consumer test
 * @author Tobias Weber
 * @date 28-mar-19
 * @license see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
 *
 * g++ -std=c++17 -o producer producer.cpp -lpthread
 */

#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>

#if __has_include(<semaphore>)
	#pragma message("Using standard semaphore.")
	#include <semaphore>

	using t_sema = std::counting_semaphore<16>;
#else
	#pragma message("Using custom semaphore.")

	/**
 	 * simple semaphore
 	*/
	class Sema
	{
	public:
		Sema(int ctr) : m_ctr{ctr} {}

		void acquire()
		{
			std::unique_lock _ul{m_mtxcond};
			m_cond.wait(_ul, [this]()->bool { return m_ctr > 0; });
			//while(!m_cond.wait_for(_ul, std::chrono::nanoseconds{10}, [this]()->bool { return m_ctr > 0; }));

			--m_ctr;
		}

		void release()
		{
			++m_ctr;

			// lock mutex in case of spurious release of wait() in acquire()
			std::scoped_lock _sl{m_mtxcond};
			m_cond.notify_one();
		}

	private:
		std::atomic<int> m_ctr{0};
		std::condition_variable m_cond{};
		std::mutex m_mtxcond{};
	};

	using t_sema = Sema;
#endif


std::list<int> lst;
t_sema sem_access{1};
t_sema sem_elems{0};


void produce()
{
	int i = 0;

	while(1)
	{
		sem_access.acquire();
		{
			lst.push_back(i++);
			std::cout << "Inserted " << (i-1) << ", number of elements now: " << lst.size() << std::endl;
		}
		sem_access.release();

		sem_elems.release();
		//std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}


void consume()
{
	while(1)
	{
		sem_elems.acquire();

		sem_access.acquire();
		{
			int i = *lst.begin();
			lst.pop_front();

			std::cout << "Removed " << i << ", number of elements now: " << lst.size() << std::endl;
		}
		sem_access.release();
	}
}



int main()
{
	std::thread prod{&produce};
	std::thread cons{&consume};

	prod.join();
	cons.join();

	return 0;
}
