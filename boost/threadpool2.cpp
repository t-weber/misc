/**
 * thead pool and threading overhead test
 * @author Tobias Weber
 * @date 15-dec-18
 * @license: see 'LICENSE.EUPL' file
 *
 * Reference:
 *  * https://www.boost.org/doc/libs/1_69_0/doc/html/boost_asio/reference/thread_pool.html
 *  * https://en.wikipedia.org/wiki/Linear_congruential_generator
 *
 * g++ -std=c++17 -O2 -march=native -o threadpool2 threadpool2.cpp -lboost_system -lpthread
 */

#include <random>
#include <atomic>
#include <chrono>
#include <complex>
#include <boost/asio.hpp>
#include <iostream>


using t_real = double;
using t_clock = std::chrono::steady_clock;


/**
 * simplistic rng test
 */
class RandGen
{
public:
	using result_type = unsigned int;

private:
	constexpr static result_type m_mult = 1234567;
	constexpr static result_type m_inc = 0;
	constexpr static result_type m_mod = std::numeric_limits<result_type>::max()-1;
	mutable result_type m_seed = std::random_device{}();

public:
	constexpr result_type operator()() const
	{
		m_seed = (m_mult*m_seed + m_inc) % m_mod;
		return m_seed;
	};

	constexpr result_type min() const
	{
		return 0;
	}

	constexpr result_type max() const
	{
		return std::numeric_limits<result_type>::max();
	}
};


std::tuple<t_real, t_real> goat(std::size_t N, unsigned int Nthreads=4)
{
	//std::mt19937_64 gen{std::random_device{}()};
	RandGen gen;

	std::atomic<std::size_t> won_stayed = 0, won_changed = 0;
	boost::asio::thread_pool tp{Nthreads};

	for(std::size_t i=0; i<N; ++i)
	{
		boost::asio::post(tp, [&gen, &won_stayed, &won_changed]() -> void
		{
			int where_nogoat = std::uniform_int_distribution<int>{0, 2}(gen);
			bool goats[] = {1, 1, 1};
			goats[where_nogoat] = 0;

			int picked = std::uniform_int_distribution<int>{0, 2}(gen);
			if(!goats[picked])
				++won_stayed;

			int revealed_goat = 0;
			while(1)
			{
				revealed_goat = std::uniform_int_distribution<int>{0, 2}(gen);
				// skip the picked door
				if(revealed_goat == picked)
					continue;
				// only reveal goats
				if(!goats[revealed_goat])
					continue;
				break;
			}

			// pick remaining door
			int changedpick = 0;
			for(; changedpick<3; ++changedpick)
			{
				if(changedpick == picked)
					continue;
				if(changedpick == revealed_goat)
					continue;
				break;
			}

			if(!goats[changedpick])
				++won_changed;
		});
	}

	tp.join();

	return std::make_tuple(t_real(won_stayed)/t_real(N), t_real(won_changed)/t_real(N));
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
		auto [prob_stay, prob_change] = goat(N, Nthreads);
		std::cout << "P_stay = " << prob_stay
			<< ", P_change = " << prob_change << std::endl;

		std::cout << "Duration: "
			<< std::chrono::duration<t_real>{t_clock::now() - starttime}.count()
			<< " s" << std::endl;
		std::cout << "--------------------------------------------------------\n";
	}

	{	// only 1 thread (here: still much less overhead!)
		std::cout << "\n--------------------------------------------------------\n";
		const unsigned int Nthreads = 1;
		std::cout << "Using " << Nthreads << " thread(s)." << std::endl;

		auto starttime = t_clock::now();
		auto [prob_stay, prob_change] = goat(N, Nthreads);
		std::cout << "P_stay = " << prob_stay
			<< ", P_change = " << prob_change << std::endl;

		std::cout << "Duration: "
			<< std::chrono::duration<t_real>{t_clock::now() - starttime}.count()
			<< " s" << std::endl;
		std::cout << "--------------------------------------------------------\n";
	}

	return 0;
}
