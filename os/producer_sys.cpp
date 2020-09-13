/**
 * producer/consumer test
 * @author Tobias Weber
 * @date 13-sep-20
 * @license see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
 *
 * g++ -std=c++17 -o producer producer_sys.cpp -lpthread
 */

#include <iostream>
#include <list>
#include <thread>
#include <chrono>
#include <pthread.h>
#include <semaphore.h>


/**
 * simple semaphore
 */
class Sema
{
public:
	Sema(unsigned int ctr)
	{
		::sem_init(&m_sema, 0, ctr);
	}

	~Sema()
	{
		::sem_destroy(&m_sema);
	}


	void acquire()
	{
		::sem_wait(&m_sema);
	}

	void release()
	{
		::sem_post(&m_sema);
	}

private:
	::sem_t m_sema{};
};



using t_sema = Sema;


std::list<int> lst;
t_sema sem_access{1};
t_sema sem_elems{0};


void* produce(void* args)
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


void* consume(void* args)
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
	::pthread_t prod, cons;

	::pthread_create(&prod, nullptr, &produce, nullptr);
	::pthread_create(&cons, nullptr, &consume, nullptr);

	::pthread_join(prod, nullptr);
	::pthread_join(cons, nullptr);

	return 0;
}
