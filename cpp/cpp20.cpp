/**
 * C++20 compatibility tests
 * @author Tobias Weber
 * @date apr-20
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++20 -DTST_RANGES -o cpp20 cpp20.cpp
 *
 * clang++ -std=c++20 -fmodules-ts --precompile -o cpp20_mod.pcm cpp20_mod.cppm && clang++ -fmodules-ts -std=c++20 -DTST_MODULES -fprebuilt-module-path=. -o cpp20 cpp20.cpp
 */

#include <iostream>
#include <compare>
#include <span>
#include <ranges>
#include <utility>
#include <numeric>
#include <algorithm>
#include <vector>
//#include <source_location>

#if __has_include(<format>)
	#include <format>
	#define TST_FORMAT
#endif

#ifdef TST_MODULES
	import cpp20_mod;
#endif


int main()
{
	// --------------------------------------------------------------------
	// operator <=>
	{
		struct A
		{
			int x;

			// TODO
			std::strong_ordering operator <=>(const A& a) const
			{
				return x <=> a.x;
			}
		};

		A a{.x = 1}, b{.x = 2};

		auto cmp = (a <=> b);
		if(cmp > 0 ) std::cout << ">" << std::endl;
		if(cmp > 0 ) std::cout << ">=" << std::endl;
		if(cmp < 0 ) std::cout << "<" << std::endl;
		if(cmp <= 0 ) std::cout << "<=" << std::endl;
		if(cmp == 0 ) std::cout << "==" << std::endl;
		if(cmp != 0 ) std::cout << "!=" << std::endl;
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// ranges, see, e.g.: https://en.cppreference.com/w/cpp/ranges/filter_view
	{
	#ifdef TST_RANGES
		using t_val = double;
		t_val *p = new t_val[128];


		// span
		std::span</*const*/ t_val/*, 128*/> span{p, p+128};

		std::iota(span.begin(), span.end(), t_val{});
		std::cout << span[5] << " " << p[5] << std::endl;


		// range view
		auto map_func = []/*<typename t_val>*/(const t_val& val) -> t_val { return val*val; };
		auto filter_func = [](const t_val& val) -> bool { return val <= 10; };

		auto view_ops =
				std::ranges::views::transform(map_func)
			|	std::ranges::views::filter(filter_func)
			;

		//auto range_sorted = std::experimental::ranges::sort(range);
		for(const t_val& val : span | view_ops)
			std::cout << val << ", ";
		std::cout << std::endl;


		// infinite view
		auto infinite_view = std::ranges::views::iota(10);
		for(auto i : infinite_view | std::ranges::views::take(50))
			std::cout << i << ", ";
		std::cout << std::endl;


		delete[] p;
	#endif
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// modules
	{
	#ifdef TST_MODULES
		std::cout << module_fact(3) << std::endl;
		std::cout << module_fact(5) << std::endl;
		std::cout << module_prod<unsigned>(1,2,3,4,5,6) << std::endl;
	#endif
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// format
	{
	#ifdef TST_FORMAT
		std::string str = std::format("Param 1: {1}, param 0: {0}.", 12, 34.45);
		std::cout << str << std::endl;
	#endif
	}
	// --------------------------------------------------------------------

	return 0;
}
