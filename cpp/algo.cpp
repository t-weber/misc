/**
 * algos
 * @author Tobias Weber
 * @date 11-nov-17
 * @license: see 'LICENSE' file
 *
 * gcc -o algo algo.cpp -std=c++17 -lstdc++ -lstdc++fs
 */

#include <iostream>
#include <iterator>
#include <vector>
#include <numeric>
#include <algorithm>
#include <experimental/iterator>
#include <random>


int main()
{
	// --------------------------------------------------------------------
	// misc
	std::cout << "gcd(10, 4) = " << std::gcd(10,4) << std::endl;
	std::cout << "lcm(10, 4) = " << std::lcm(10,4) << std::endl;

	std::cout
		<< std::clamp(10, 0, 5, [](auto a, auto b) -> bool { return a<b; })
		<< std::endl;
	// --------------------------------------------------------------------



	// --------------------------------------------------------------------
	// accumulate / reduce
	using T = int;
	std::vector<T> v{{1,2,3,4,5}};
	T init = 1;

	auto op_red = [](T a, T b) -> T
	{
		return a*b;
	};

	T sum = std::accumulate(v.begin(), v.end(), init, op_red);
	std::cout << sum << std::endl;

	T sum2 = std::reduce(std::execution::par/*seq*/, v.begin(), v.end(), init, op_red);
	std::cout << sum2 << std::endl;
	// --------------------------------------------------------------------



	// --------------------------------------------------------------------
	// transform
	auto op_trafo = [](T a) -> T
	{
		return a*2;
	};

	std::vector w(v.size(), 0);
	std::transform(v.begin(), v.end(), w.begin(), op_trafo);
	T sum3 = std::accumulate(w.begin(), w.end(), init, op_red);
	std::cout << sum3 << std::endl;

	T sum4 = std::transform_reduce(std::execution::par, v.begin(), v.end(), init, op_red, op_trafo);
	std::cout << sum4 << std::endl;
	// --------------------------------------------------------------------



	// --------------------------------------------------------------------
	// sample
	std::mt19937 rndeng(123);
	auto joiner = std::experimental::make_ostream_joiner(std::cout, ", ");
	std::sample(v.begin(), v.end(), joiner, 3, rndeng);
	std::cout << std::endl;
	// --------------------------------------------------------------------

	return 0;
}
