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


/**
 * simple semaphore
 */
class Sema
{
	public:
		Sema(int ctr) : m_ctr{ctr} {}

		void pass()
		{
			while(m_ctr == 0)
			{
				std::unique_lock<std::mutex> _ul{m_mtxcond};
				m_cond.wait(_ul);
			}

			--m_ctr;
		}

		void leave()
		{
			++m_ctr;
			m_cond.notify_one();
		}

	private:
		std::atomic<int> m_ctr{0};
		std::condition_variable m_cond;
		std::mutex m_mtxcond;
};



std::list<int> lst;
const int MAX_ELEMS = 5;
std::mutex mtx;
Sema sem_free{MAX_ELEMS};
Sema sem_occu{0};


void produce()
{
	int i=0;

	while(1)
	{
		sem_free.pass();

		{
			std::lock_guard<std::mutex> _lg{mtx};
			lst.push_back(i++);

			std::cout << "Inserted " << (i-1) << ", number of elements now: " << lst.size() << std::endl;
			if(lst.size() > MAX_ELEMS)
			{
				std::cerr << "Maximum number of elements exceeded (should not happen)!" << std::endl;
				exit(-1);
			}
		}

		sem_occu.leave();
	}
}


void consume()
{
	while(1)
	{
		sem_occu.pass();

		{
			std::lock_guard<std::mutex> _lg{mtx};
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

		sem_free.leave();
	}
}



int main(int argc, char **argv)
{
	std::thread prod(&produce);
	std::thread cons(&consume);

	prod.join();
	cons.join();

	return 0;
}
