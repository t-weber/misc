/**
 * producer/consumer test
 * @author Tobias Weber
 * @date 28-mar-19
 * @license see 'LICENSE.EUPL' file
 * @references
 *	- https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
 *	- https://en.wikipedia.org/wiki/Peterson%27s_algorithm
 *
 * g++ -std=c++17 -o producer producer_busy.cpp -lpthread
 */

#include <iostream>
#include <list>
#include <thread>
#include <chrono>


bool producer_active = 0;
bool producer_wants_to_run = 0;
bool consumer_wants_to_run = 0;

std::list<int> lst;


void produce()
{
	int i=0;

	while(1)
	{
		producer_active = true;
		producer_wants_to_run = true;

		// busy wait
		while(producer_active && consumer_wants_to_run)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

		{
			lst.push_back(i++);

			std::cout << "Inserted " << (i-1) << ", number of elements now: " << lst.size() << std::endl;
		}

		producer_wants_to_run = false;

		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
}


void consume()
{
	while(1)
	{
		producer_active = false;
		consumer_wants_to_run = true;

		// busy wait
		while(!producer_active && producer_wants_to_run)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

		{
			if(lst.size())
			{
				int i = *lst.begin();
				lst.pop_front();

				std::cout << "Removed " << i << ", number of elements now: " << lst.size() << std::endl;
			}
		}

		consumer_wants_to_run = false;

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
