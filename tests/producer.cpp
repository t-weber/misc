/**
 * producer/consumer test
 * @author Tobias Weber
 * @date 28-mar-19
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++11 -o producer producer.cpp -lpthread
 */

#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
//#include <semaphore>


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

			/*while(m_ctr <= 0)
			{
				//m_cond.wait(_ul);
				m_cond.wait_for(_ul, std::chrono::nanoseconds(10));
			}*/

			--m_ctr;
		}

		void release()
		{
			++m_ctr;

			// lock mutex in case of spurious release of wait() in pass()
			std::scoped_lock _sl{m_mtxcond};
			m_cond.notify_one();
		}

	private:
		std::atomic<int> m_ctr{0};
		std::condition_variable m_cond;
		std::mutex m_mtxcond;
};


using t_sema = Sema;
//using t_sema = std::counting_semaphore<10>;

std::list<int> lst;
const int MAX_ELEMS = 10;
std::mutex mtx;
t_sema sem_free{MAX_ELEMS};
t_sema sem_occu{0};


void produce()
{
	int i=0;

	while(1)
	{
		sem_free.acquire();

		{
			std::scoped_lock _sl{mtx};
			lst.push_back(i++);

			std::cout << "Inserted " << (i-1) << ", number of elements now: " << lst.size() << std::endl;
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



int main(int argc, char **argv)
{
	std::thread prod{&produce};
	std::thread cons{&consume};

	prod.join();
	cons.join();

	return 0;
}
