/**
 * producer/consumer test
 * @author Tobias Weber
 * @date 28-mar-19
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++17 -o producer producer_prio.cpp -lpthread
 */

#include <iostream>
#include <list>
#include <vector>
#include <queue>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
//#include <semaphore>


/**
 * semaphore with priority queue
 */
class Sema
{
	public:
		Sema(int ctr) : m_ctr{ctr} {}

		void acquire(int prio=0)
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

	private:
		std::atomic<int> m_ctr{0};
		std::condition_variable m_cond{};
		std::mutex m_mtxcond{};

		std::priority_queue<int> m_prio{};
};


using t_sema = Sema;
//using t_sema = std::counting_semaphore<10>;

std::list<int> lst;
const int MAX_ELEMS = 10;
std::mutex mtx;
t_sema sem_free{MAX_ELEMS};
t_sema sem_occu{0};

std::mt19937 rng{std::random_device{}()};


void produce()
{
	int i=0;

	while(1)
	{
		int prio = std::uniform_int_distribution<int>{0,9}(rng);
		sem_free.acquire();

		{
			std::scoped_lock _sl{mtx};
			lst.push_back(i++);

			std::cout << "Inserted " << (i-1) << " (priority: " << prio << "), number of elements now: " << lst.size() << std::endl;
			if(lst.size() > MAX_ELEMS)
			{
				std::cerr << "Maximum number of elements exceeded (should not happen)!" << std::endl;
				exit(-1);
			}
		}

		sem_occu.release();
	}
}


void consume()
{
	while(1)
	{
		sem_occu.acquire();

		{
			std::scoped_lock _sl{mtx};
			if(lst.size())
			{
				if(lst.size() > MAX_ELEMS)
				{
					std::cerr << "Maximum number of elements exceeded (should not happen)!" << std::endl;
					exit(-1);
				}

				int i = *lst.begin();
				lst.pop_front();

				std::cout << "Removed " << i << ", number of elements now: " << lst.size() << std::endl;
			}
		}

		sem_free.release();
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
