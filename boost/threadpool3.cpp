/**
 * thead pool test
 * @author Tobias Weber
 * @date 5-apr-20
 * @license: see 'LICENSE.EUPL' file
 *
 * Reference:
 *  * https://www.boost.org/doc/libs/1_69_0/doc/html/boost_asio/reference/thread_pool.html
 *
 * g++ -std=c++17 -O2 -march=native -o threadpool threadpool3.cpp -lboost_system -lpthread
 */

#include <boost/asio.hpp>
#include <future>
#include <memory>
#include <iostream>


int main()
{
	constexpr int Nthreads = 4;
	constexpr int Ntasks = 10000;

	boost::asio::thread_pool tp{Nthreads};


	// task function
	auto task = [](int a, int b) -> int
	{ return a*b; };


	// all tasks
	std::vector<std::shared_ptr<std::packaged_task<int(int,int)>>> packages(Ntasks);

	// send tasks to thread pool
	for(int i=0; i<Ntasks; ++i)
	{
		std::cout << "task " << i << std::endl;

		// wrap task function in a packaged_task so that it's result
		// can be queried later via the std::future
		auto package = std::make_shared<std::packaged_task<int(int,int)>>(task);
		packages[i] = package;

		boost::asio::post(tp, [package, i]() -> void
		{
			(*package)(i, 100);
		});
	}


	// get individual results
	for(auto& task : packages)
	{
		// query the std::packaged_task's (future) result
		std::cout << task->get_future().get() << std::endl;
	}

	tp.join();
	return 0;
}
