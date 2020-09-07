/**
 * producer/consumer test
 * @author Tobias Weber
 * @date 6-sep-20
 * @license see 'LICENSE.EUPL' file
 * @references
 *	- https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
 *	- https://en.wikibooks.org/wiki/X86_Assembly/Data_Transfer
 *	- https://wiki.osdev.org/Inline_Assembly
 *
 * g++ -std=c++17 -o producer producer_tsl.cpp -lpthread
 */

#include <iostream>
#include <list>
#include <thread>
#include <chrono>
#include <cstdint>


std::list<int> g_lst{};
std::uint8_t g_mtx = 0;


void lock_mtx(volatile std::uint8_t* mtx)
{
	const std::uint8_t locked_state = 1;
	volatile std::uint8_t is_locked = 1;

	// busy wait on mtx variable
	while(is_locked)
	{
		/**
	 	* cmpxchgq mtx, locked_state:
	 	*	if(*mtx == rax)			// mutex free?
	 	*		*mtx = locked_state;	// acquire mutex
	 	*	else
	 	*		rax = *mtx;		// already locked by other thread, return 1 in rax
	 	*/
		asm volatile(
			"xorb %%al, %%al\n"
			"lock cmpxchgb %2, %0\n"
			"movb %%al, %1\n"
				: /*%0*/ "+m" (*mtx), /*%1*/ "=r" (is_locked)	// +m: in/out mem, "=r" out via reg
				: /*%2*/ "r" (locked_state)			// r: in via reg
				: "%al", "memory", "cc"				// indicate modified things, "cc" = %flags register
		);

		std::this_thread::yield();
	}
}


void unlock_mtx(std::uint8_t* mtx)
{
	*mtx = 0;
}


void produce()
{
	int i=0;

	while(1)
	{
		lock_mtx(&g_mtx);
		{
			g_lst.push_back(i++);

			std::cout << "Inserted " << (i-1) << ", number of elements now: " << g_lst.size() << std::endl;
		}
		unlock_mtx(&g_mtx);

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}


void consume()
{
	while(1)
	{
		lock_mtx(&g_mtx);
		{
			if(g_lst.size())
			{
				int i = *g_lst.begin();
				g_lst.pop_front();

				std::cout << "Removed " << i << ", number of elements now: " << g_lst.size() << std::endl;
			}
		}
		unlock_mtx(&g_mtx);

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
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
