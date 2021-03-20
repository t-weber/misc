/**
 * sort tests
 * @author Tobias Weber
 * @date mar-2021
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_75_0/libs/sort/doc/html/index.html
 *  * https://www.boost.org/doc/libs/1_75_0/libs/timer/doc/index.html
 *
 * g++ -O2 -march=native -std=c++20 -o sort sort.cpp -lpthread -lboost_timer
 */

#include <iostream>
#include <vector>
#include <random>

#include <boost/sort/sort.hpp>
namespace sort = boost::sort;

#include <boost/timer/timer.hpp>
namespace timer = boost::timer;

#include <boost/type_index.hpp>
namespace ty = boost::typeindex;


template<class T>
std::string get_type_str()
{
	return ty::type_id_with_cvr<T>().pretty_name();
}


template<class t_cont>
std::size_t test_stdsort(const t_cont& cont)
{
	using t_real = double;

	t_cont v = cont;
	auto w = v;
	std::size_t wall_ns{};

	{
		timer::cpu_timer ti;

		std::sort(v.begin(), v.end());

		ti.stop();
		timer::cpu_times elapsed = ti.elapsed();
		std::cout << "sort                : wall = " << elapsed.wall << "ns"
			<< ", user = " << elapsed.user << "ns"
			<< ", system = " << elapsed.system << "ns"
			<< std::endl;

		wall_ns = elapsed.wall;
	}


	{
		timer::cpu_timer ti;

		std::stable_sort(w.begin(), w.end());

		ti.stop();
		timer::cpu_times elapsed = ti.elapsed();
		std::cout << "stable_sort         : wall = " << elapsed.wall << "ns"
			<< ", user = " << elapsed.user << "ns"
			<< ", system = " << elapsed.system << "ns"
			<< ", factor = " << t_real(elapsed.wall) / t_real(wall_ns)
			<< std::endl;
	}

	return wall_ns;
}


template<class t_cont>
void test_multithreaded(const t_cont& cont, std::size_t cmp_wall_ns)
{
	using t_real = double;

	t_cont v = cont;
	auto w = v;
	auto x = v;


	// stable sort 1
	{
		timer::cpu_timer ti;

		sort::parallel_stable_sort(v.begin(), v.end());

		ti.stop();
		timer::cpu_times elapsed = ti.elapsed();
		std::cout << "parallel_stable_sort: wall = " << elapsed.wall << "ns"
			<< ", user = " << elapsed.user << "ns"
			<< ", system = " << elapsed.system << "ns"
			<< ", factor = " << t_real(elapsed.wall) / t_real(cmp_wall_ns)
			<< std::endl;
	}

	// stable sort 2
	{
		timer::cpu_timer ti;

		sort::sample_sort(w.begin(), w.end());

		ti.stop();
		timer::cpu_times elapsed = ti.elapsed();
		std::cout << "sample_sort         : wall = " << elapsed.wall << "ns"
			<< ", user = " << elapsed.user << "ns"
			<< ", system = " << elapsed.system << "ns"
			<< ", factor = " << t_real(elapsed.wall) / t_real(cmp_wall_ns)
			<< std::endl;
	}

	// non-stable sort
	{
		timer::cpu_timer ti;

		sort::block_indirect_sort(x.begin(), x.end());

		ti.stop();
		timer::cpu_times elapsed = ti.elapsed();
		std::cout << "block_indirect_sort : wall = " << elapsed.wall << "ns"
			<< ", user = " << elapsed.user << "ns"
			<< ", system = " << elapsed.system << "ns"
			<< ", factor = " << t_real(elapsed.wall) / t_real(cmp_wall_ns)
			<< std::endl;
	}

/*
	for(int i : v)
		std::cout << i << ", ";
	std::cout << std::endl;

	for(int i : w)
		std::cout << i << ", ";
	std::cout << std::endl;

	for(int i : x)
		std::cout << i << ", ";
	std::cout << std::endl;
*/
}


template<class t_cont>
void test_singlethreaded(const t_cont& cont, std::size_t cmp_wall_ns)
{
	using t_real = double;

	t_cont v = cont;
	auto w = v;
	auto x = v;

	//sort::spreadsort(v.begin(), v.end());

	{
		timer::cpu_timer ti;

		sort::pdqsort(v.begin(), v.end());

		ti.stop();
		timer::cpu_times elapsed = ti.elapsed();
		std::cout << "pdqsort             : wall = " << elapsed.wall << "ns"
			<< ", user = " << elapsed.user << "ns"
			<< ", system = " << elapsed.system << "ns"
			<< ", factor = " << t_real(elapsed.wall) / t_real(cmp_wall_ns)
			<< std::endl;
	}

	{
		timer::cpu_timer ti;

		sort::spinsort(w.begin(), w.end());

		ti.stop();
		timer::cpu_times elapsed = ti.elapsed();
		std::cout << "spinsort            : wall = " << elapsed.wall << "ns"
			<< ", user = " << elapsed.user << "ns"
			<< ", system = " << elapsed.system << "ns"
			<< ", factor = " << t_real(elapsed.wall) / t_real(cmp_wall_ns)
			<< std::endl;
	}

	{
		timer::cpu_timer ti;

		sort::flat_stable_sort(x.begin(), x.end());

		ti.stop();
		timer::cpu_times elapsed = ti.elapsed();
		std::cout << "flat_stable_sort    : wall = " << elapsed.wall << "ns"
			<< ", user = " << elapsed.user << "ns"
			<< ", system = " << elapsed.system << "ns"
			<< ", factor = " << t_real(elapsed.wall) / t_real(cmp_wall_ns)
			<< std::endl;
	}

/*
	for(int i : v)
		std::cout << i << ", ";
	std::cout << std::endl;

	for(int i : w)
		std::cout << i << ", ";
	std::cout << std::endl;

	for(int i : x)
		std::cout << i << ", ";
	std::cout << std::endl;
*/
}


template<template<class...> class t_cont, typename t_num>
void rng(t_cont<t_num>& cont)
{
	const t_num min = -std::numeric_limits<t_num>::max();
	const t_num max = std::numeric_limits<t_num>::max();

	std::mt19937 rng{std::random_device{}()};
	std::generate(cont.begin(), cont.end(), [min, max, &rng]() -> t_num
	{
		t_num r{};
		if constexpr(std::is_integral_v<t_num>)
			r = std::uniform_int_distribution<t_num>(min, max)(rng);
		if constexpr(std::is_floating_point_v<t_num>)
			r = std::uniform_real_distribution<t_num>(min, max)(rng);
		return r;
	});
}


template<class t_num>
void run_tests(std::size_t N)
{
	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << "Testing for type " << get_type_str<t_num>() << "..." << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	std::vector<t_num> v;
	v.resize(N);
	rng<std::vector, t_num>(v);

	std::cout << "C++ standard sort:" << std::endl;
	std::size_t wall_ns = test_stdsort(v);
	std::cout << std::endl;

	std::cout << "Multi-threaded sort:" << std::endl;
	test_multithreaded(v, wall_ns);
	std::cout << std::endl;

	std::cout << "Single-threaded sort:" << std::endl;
	test_singlethreaded(v, wall_ns);
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;
}


int main(int argc, char** argv)
{
	constexpr const std::size_t N = 1e7;

	run_tests<int>(N);
	run_tests<long>(N);
	run_tests<float>(N);
	run_tests<double>(N);

	return 0;
}
