/**
 * synchronisation using busy waits
 * @author Tobias Weber
 * @date 19-sep-2020
 * @license see 'LICENSE.EUPL' file
 *
 * @desc see: https://en.wikipedia.org/wiki/Peterson%27s_algorithm
 *
 * g++ -std=c++17 -o busysync busysync.cpp -lpthread
 */

#include <iostream>
#include <thread>
#include <chrono>


#define YIELD_THREAD 1
#define MAX_ITERS 1000


static void thproc(int this_thread, int* which_thread, bool* this_thread_interest, bool* other_thread_interest)
{
	for(std::size_t i=0; i<MAX_ITERS; ++i)
	{
		// ------------------------------------------------------------
		// enter critical section
		int other_thread = (this_thread == 1 ? 2 : 1);

		*which_thread = other_thread;
		*this_thread_interest = true;

		while(*which_thread==other_thread && *other_thread_interest)
		{
#if YIELD_THREAD == 1
			//std::this_thread::sleep_for(std::chrono::milliseconds{50});
			std::this_thread::yield();
#endif
		}
		// ------------------------------------------------------------


		std::cout << "In thread " << this_thread << "." << std::endl;


		// ------------------------------------------------------------
		// leave critical section
		*this_thread_interest = false;
		// ------------------------------------------------------------
	}
}


int main()
{
	int which_thread = 1;
	bool th1_interest = false;
	bool th2_interest = false;

	std::thread th1{&thproc, 1, &which_thread, &th1_interest, &th2_interest};
	std::thread th2{&thproc, 2, &which_thread, &th2_interest, &th1_interest};

	th1.join();
	th2.join();

	return 0;
}
