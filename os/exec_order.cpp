/**
 * execution order
 * @author Tobias Weber
 * @date 22-sep-20
 * @license see 'LICENSE.EUPL' file
 */

#include <iostream>
#include <thread>
#include <chrono>


#if __has_include(<semaphore>)
	#pragma message("Using standard semaphore.")
	#include <semaphore>

	using t_sema = std::counting_semaphore<16>;
#else
	#pragma message("Using custom semaphore.")
	#include "sema.h"

	using t_sema = Sema<unsigned int>;
#endif


t_sema second{0}, third{0}, forth{0};


static void thproc4()
{
	//first.acquire();
	std::this_thread::sleep_for(std::chrono::milliseconds{200});

	std::cout << "This should execute first." << std::endl;
	second.release();
}


static void thproc3()
{
	second.acquire();
	std::this_thread::sleep_for(std::chrono::milliseconds{200});

	std::cout << "This should execute second." << std::endl;
	third.release();
}


static void thproc2()
{
	third.acquire();
	std::this_thread::sleep_for(std::chrono::milliseconds{200});

	std::cout << "This should execute third." << std::endl;
	forth.release();
}


static void thproc1()
{
	forth.acquire();
	std::this_thread::sleep_for(std::chrono::milliseconds{200});

	std::cout << "This should execute forth." << std::endl;
}



int main()
{
	std::thread th1{&thproc1};
	std::thread th2{&thproc2};
	std::thread th3{&thproc3};
	std::thread th4{&thproc4};

	th1.join();
	th2.join();
	th3.join();
	th4.join();

	return 0;
}

