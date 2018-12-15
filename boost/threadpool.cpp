/**
 * thead pool and threading overhead test
 * @author Tobias Weber
 * @date 15-dec-18
 * @license: see 'LICENSE.EUPL' file
 *
 * Reference:
 *  * https://www.boost.org/doc/libs/1_69_0/doc/html/boost_asio/reference/thread_pool.html
 *
 * g++ -std=c++17 -O2 -march=native -o threadpool threadpool.cpp -lboost_system -lpthread
 */

#include <random>
#include <atomic>
#include <chrono>
#include <boost/asio.hpp>
#include <iostream>


using t_real = double;
using t_clock = std::chrono::steady_clock;
//using t_clock = std::chrono::high_resolution_clock;


t_real calc_pi(std::size_t N, unsigned int Nthreads=4)
{
	std::mt19937_64 gen;
	gen.seed(1234);

	std::atomic<std::size_t> area{};
	boost::asio::thread_pool tp{Nthreads};

	// area of circle: c: r^2 * pi = pi
	// area of square: s: a*b = 1-(-1) * 1-(-1) = 4
	// c/s = 1/4
	for(std::size_t i=0; i<N; ++i)
	{
		boost::asio::post(tp, [&gen, &area]() -> void
		{
			// generate random number is a [-1,1]*[-1,1] square
			t_real x = std::uniform_real_distribution<t_real>{-1, 1}(gen);
			t_real y = std::uniform_real_distribution<t_real>{-1, 1}(gen);

			// in unit circle?
			t_real r = std::sqrt(x*x + y*y);
			if(r <= 1.)
				++area;
		});
	}

	tp.join();
	return t_real(area)/t_real(N)*t_real(4);
}


int main()
{
	const std::size_t N = 100000;
	std::cout.precision(8);

	{	// maximum number of supported threads
		std::cout << "--------------------------------------------------------\n";
		const unsigned int Nthreads = std::thread::hardware_concurrency();
		std::cout << "Using " << Nthreads << " thread(s)." << std::endl;

		auto starttime = t_clock::now();
		std::cout << "pi = " << calc_pi(N, Nthreads) << std::endl;

		std::cout << "Duration: "
			<< std::chrono::duration<t_real>{t_clock::now() - starttime}.count()
			<< " s" << std::endl;
		std::cout << "--------------------------------------------------------\n";
	}

	{	// only 1 thread (here: much less overhead!)
		std::cout << "--------------------------------------------------------\n";
		const unsigned int Nthreads = 1;
		std::cout << "Using " << Nthreads << " thread(s)." << std::endl;

		auto starttime = t_clock::now();
		std::cout << "pi = " << calc_pi(N, Nthreads) << std::endl;

		auto dur = std::chrono::duration<t_real>{t_clock::now() - starttime};
		std::cout << "Duration: "
			<< std::chrono::duration<t_real>{t_clock::now() - starttime}.count()
			<< " s" << std::endl;
		std::cout << "--------------------------------------------------------\n";
	}
}
