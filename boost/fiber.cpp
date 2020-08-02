/**
 * user thread test
 * @author Tobias Weber
 * @date 1-aug-2020
 * @license: see 'LICENSE.EUPL' file
 *
 * Reference:
 *  * https://www.boost.org/doc/libs/1_73_0/libs/fiber/doc/html/fiber/fiber_mgmt.html
 *  * https://www.boost.org/doc/libs/1_73_0/libs/fiber/doc/html/fiber/scheduling.html
 *
 * g++ -o fiber fiber.cpp -lboost_fiber -lboost_context -lpthread
 */

#include <chrono>
#include <vector>
#include <queue>
#include <mutex>
#include <iostream>

#include <boost/fiber/all.hpp>
namespace fiber = boost::fibers;
namespace this_fiber = boost::this_fiber;


template<class t_algo>
class SchedBase : public t_algo
{
public:
	// --------------------------------------------------------------------
	/**
	 * suspend the scheduler thread
	 */
	virtual void suspend_until(const std::chrono::steady_clock::time_point& tp) noexcept override
	{
		//std::cout << __func__ << std::endl;
		std::unique_lock<std::mutex> lock{m_mtxSched};
		m_condSched.wait_until(lock, tp);
		//m_condSched.wait(lock);
	}


	/**
	 * resume the scheduler thread
	 */
	virtual void notify() noexcept override
	{
		//std::cout << __func__ << std::endl;
		m_condSched.notify_one();
	}
	// --------------------------------------------------------------------

protected:
	std::mutex m_mtxSched{};
	std::condition_variable m_condSched{};
};


class SchedFCFS : public SchedBase<fiber::algo::algorithm>
{
public:
	SchedFCFS() = default;
	virtual ~SchedFCFS() = default;


	// --------------------------------------------------------------------
	virtual bool has_ready_fibers() const noexcept override
	{
		return !m_fibers.empty();
	}


	/**
	 * register a new fiber
	 */
	virtual void awakened(fiber::context* fib) noexcept override
	{
		m_fibers.push(fib);
	}


	/**
	 * schedule a fiber
	 */
	virtual fiber::context* pick_next() noexcept override
	{
		if(!m_fibers.empty())
		{
			fiber::context *fib = m_fibers.front();
			m_fibers.pop();
			return fib;
		}

		return nullptr;
	}
	// --------------------------------------------------------------------


private:
	std::queue<fiber::context*> m_fibers{};
};


// ============================================================================


struct FiberProps : public fiber::fiber_properties
{
	FiberProps(fiber::context* fib) noexcept
		: fiber::fiber_properties{fib}
	{}

	int prio = 0;
};


class SchedPrio : public SchedBase<fiber::algo::algorithm_with_properties<FiberProps>>
{
public:
	SchedPrio() = default;
	virtual ~SchedPrio() = default;


	// --------------------------------------------------------------------
	virtual bool has_ready_fibers() const noexcept override
	{
		return !m_fibers.empty();
	}


	/**
	 * register a new fiber
	 */
	virtual void awakened(fiber::context* fib, FiberProps& props) noexcept override
	{
		//std::cout << __func__ << ", prio: " << props.prio << std::endl;
		m_fibers.emplace(PrioFiber{.fib = fib, .prio = props.prio});
	}


	/**
	 * schedule a fiber
	 */
	virtual fiber::context* pick_next() noexcept override
	{
		if(!m_fibers.empty())
		{
			PrioFiber fib{std::move(m_fibers.top())};
			m_fibers.pop();
			return fib.fib;
		}

		return nullptr;
	}
	// --------------------------------------------------------------------


private:
	struct PrioFiber
	{
		fiber::context* fib = nullptr;
		int prio = 0;	// lower number is higher prio
	};

	struct PrioFiberCmp
	{
		bool operator()(const PrioFiber& fib1, const PrioFiber& fib2) const
		{
			//std::cout << "comparing: " << fib1.prio << ", " << fib2.prio << std::endl;
			return fib1.prio >= fib2.prio;
		}
	};

	std::priority_queue<PrioFiber, std::vector<PrioFiber>, PrioFiberCmp> m_fibers{};
};


// ============================================================================


int main()
{
	auto fiberproc = [](int id, int iters, int wait)
	{
		// wait before starting
		this_fiber::sleep_for(std::chrono::milliseconds{250});

		std::cout << "Fiber " << id << " begin." << std::endl;
		for(int i=0; i<iters; ++i)
		{
			std::cout << "Fiber " << id << " running." << std::endl;

			if(wait > 0)
			{
				//this_fiber::yield();

				// automatically yields
				this_fiber::sleep_for(std::chrono::milliseconds{wait});
			}
		}
		std::cout << "Fiber " << id << " end." << std::endl;
	};

	{
		std::cout << "--------------------------------------------------------------------------------\n";
		std::cout << "Scheduler: FCFS\n";
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		//fiber::use_scheduling_algorithm<fiber::algo::round_robin>();
		fiber::use_scheduling_algorithm<SchedFCFS>();

		fiber::fiber fib1{ [&fiberproc](){ fiberproc(1, 10, 100); } };
		fiber::fiber fib2{ [&fiberproc](){ fiberproc(2, 20, 50); } };
		fiber::fiber fib3{ [&fiberproc](){ fiberproc(3, 40, 25); } };
		fiber::fiber fib4{ [&fiberproc](){ fiberproc(4, 5, 200); } };

		fib1.join();
		fib2.join();
		fib3.join();
		fib4.join();
		std::cout << "--------------------------------------------------------------------------------\n";
		std::cout << std::endl;
	}

	{
		std::cout << "--------------------------------------------------------------------------------\n";
		std::cout << "Scheduler: Prio\n";
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		fiber::use_scheduling_algorithm<SchedPrio>();

		fiber::fiber fib1{fiber::launch::post, [&fiberproc](){ fiberproc(1, 1000, 0); } };
		fiber::fiber fib2{fiber::launch::post, [&fiberproc](){ fiberproc(2, 1000, 0); } };
		fiber::fiber fib3{fiber::launch::post, [&fiberproc](){ fiberproc(3, 1000, 0); } };

		fib1.properties<FiberProps>().prio = 3;
		fib2.properties<FiberProps>().prio = 1;
		fib3.properties<FiberProps>().prio = 2;
		std::cout << "set priorities" << std::endl;

		fib1.join();
		fib2.join();
		fib3.join();
		std::cout << "--------------------------------------------------------------------------------\n";
		std::cout << std::endl;
	}

	return 0;
}
