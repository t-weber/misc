/**
 * threading tests
 * @author Tobias Weber
 * @date jan-19
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++17 -o thread thread.cpp -lpthread
 */

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <future>
#include <chrono>
#include <iostream>


int main()
{
	// normal threads
	{
		auto func0 = [](std::mutex *mtx)
		{
			std::lock_guard lock0{*mtx};
			std::cout << "In thread " << std::hex << std::this_thread::get_id() << std::endl;
		};

		std::mutex mtx;
		std::thread thread0{func0, &mtx};
		//thread0.detach();

		{
			std::lock_guard lock0{mtx};
			std::cout << "In main thread " << std::hex << std::this_thread::get_id() << std::endl;
		}

		thread0.join();
	}


	// (packaged) tasks
	{
		auto func1 = [](std::string a, std::string b) -> std::string
		{
			return "Task: a = " + a + ", b = " + b;
		};

		auto fut1a = std::async(std::launch::async, func1, "123", "abc");
		auto fut1b = std::async(std::launch::async, func1, "xyz", "987");
		std::cout << fut1a.get() << ", " << fut1b.get() << std::endl;


		// packaged tasks, deferred
		std::packaged_task<std::string(std::string, std::string)> task2a{func1};
		auto fut2a = task2a.get_future();
		task2a("aaa", "bbb");
		std::cout << fut2a.get() << std::endl;


		// packaged tasks, threaded
		std::packaged_task<std::string(std::string, std::string)> task2b{func1};
		auto fut2b = task2b.get_future();
		std::thread th2b{std::move(task2b), "xxx", "yyy"};
		std::cout << fut2b.get() << std::endl;
		th2b.join();
	}


	// promises
	{
		// don't directly return result, but save it to a promise
		auto func3 = [](std::string a, std::string b, std::promise<std::string>&& prom) -> void
		{
			std::string c = "Task: a = " + a + ", b = " + b;
			//prom.set_value(c);
			prom.set_value_at_thread_exit(c);
		};

		std::promise<std::string> prom3;
		auto fut3 = prom3.get_future();
		std::thread th3{func3, "qwe", "asd", std::move(prom3)};
		std::cout << fut3.get() << std::endl;
		th3.join();
	}


	// shared_mutex
	{
		int data{};
		std::shared_mutex mtx;

		auto func_read = [&mtx, &data]() -> void
		{
			for(int i=0; i<20; ++i)
			{
				{
					// exclude writers, but not readers
					std::shared_lock lock{mtx};
					std::cout << "Read thread " << std::hex << std::this_thread::get_id()
						<< ": " << std::dec << data << std::endl;
				}

				// 0.5 s
				std::this_thread::sleep_for(std::chrono::duration<long, std::ratio<1,2>>{1});
			}
		};

		auto func_write = [&mtx, &data]() -> void
		{
			for(int i=0; i<10; ++i)
			{
				{
					// exclude everything
					//std::unique_lock lock{mtx};
					std::scoped_lock lock{mtx};	// for locking several mutices
					++data;
					std::cout << "Write thread " << std::hex << std::this_thread::get_id()
						<< ": " << std::dec << data << std::endl;
				}

				// 1 s
				std::this_thread::sleep_for(std::chrono::seconds{1});
			}
		};

		std::thread th_rd1{func_read};
		std::thread th_rd2{func_read};

		std::thread th_wr1{func_write};
		std::thread th_wr2{func_write};

		for(auto *th : { &th_rd1, &th_rd2, &th_wr1, &th_wr2 })
			th->join();
	}

	return 0;
}
