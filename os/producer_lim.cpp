/**
 * producer/consumer test
 * @author Tobias Weber
 * @date 28-mar-19
 * @license see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
 *
 * g++ -std=c++17 -o producer_lim producer_lim.cpp -lpthread
 */

#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>

#include "sema.h"


using t_sema = Sema<unsigned int>;
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



int main()
{
	std::thread prod{&produce};
	std::thread cons{&consume};

	prod.join();
	cons.join();

	return 0;
}
