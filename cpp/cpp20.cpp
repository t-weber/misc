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
#include <bit>
#include <algorithm>
#include <vector>
#include <list>
#include <tuple>
#include <unordered_map>
//#include <source_location>

#if __has_include(<format>)
	#include <format>
	#define TST_FORMAT
#else
	#warning "No format library."
#endif

#ifdef TST_MODULES
	import cpp20_mod;
#endif


int main()
{
	// --------------------------------------------------------------------
	// operator <=>
	{
		std::cout << "operator <=>" << std::endl;

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
		std::cout << "ranges" << std::endl;

		using t_val = double;
		t_val *p = new t_val[128];


		// span
		std::span</*const*/ t_val/*, 128*/> span{p, p+128};

		std::iota(span.begin(), span.end(), t_val{});
		std::cout << span[5] << " " << p[5] << std::endl;


		// subrange
		//auto view = std::ranges::subrange(p+0, p+128);
		auto view = std::ranges::subrange(span.begin(), span.end());
		std::cout << "subrange: ";
		for(const auto& v : view)
			std::cout << v << " ";
		std::cout << std::endl;


		// range view
		auto view_ops =
			  std::ranges::views::transform(
				[]<class T>(const T& val) -> T { return val - 10; })
			| std::ranges::views::filter(
				[]<class T>(const T& val) -> bool { return val <= 10 && val >= 0; })
			| std::ranges::views::transform(
				[]<class T>(const T& val) -> T { return val + 10; })
			;

		//auto range_sorted = std::experimental::ranges::sort(range);
		std::cout << "transformed/filtered: ";
		for(const t_val& val : span | view_ops)
			std::cout << val << ", ";
		std::cout << "\n" << std::endl;


		// infinite view
		auto infinite_view = std::ranges::views::iota(10);
		std::cout << "infinite view: ";
		for(auto i : infinite_view | std::ranges::views::take(50))
			std::cout << i << ", ";
		std::cout << "\n" << std::endl;


		// join view
		std::list<std::vector<std::pair<std::string, int>>> tojoin
		{
			{
				std::make_pair("Test 1", 123),
				std::make_pair("Test 2", 987),
			},
			{
				std::make_pair("Test A", 555),
				std::make_pair("Test B", 444),
			}
		};
		std::cout << "join view: ";
		for(const auto& pair : std::ranges::views::join(tojoin))
			std::cout << "(" << std::get<0>(pair) << " " << std::get<1>(pair) << "), ";
		std::cout << "\n" << std::endl;


		// elements
		std::tuple<int, double> tuparr[]
		{
			std::make_tuple(12, 5.67),
			std::make_tuple(23, 6.78),
			std::make_tuple(34, 7.89),
			std::make_tuple(45, 8.90),
		};
		std::cout << "elements: ";
		for(const auto& elem : std::ranges::views::elements<1>(tuparr))
			std::cout << elem << " ";
		std::cout << std::endl;
		for(const auto& elem : std::ranges::views::elements<1>(std::ranges::subrange(tuparr, tuparr+3)))
			std::cout << elem << " ";
		std::cout << "\n" << std::endl;


		// keys / values
		std::unordered_map<int, double> map
		{
			std::make_pair(-5, 99.8),
			std::make_pair(5, 22.3),
			std::make_pair(3, 23.8),
			std::make_pair(19, 5.1),
		};
		std::cout << "keys: ";
		for(const auto& key : std::ranges::views::keys(map))
			std::cout << key << " ";
		std::cout << "\nvalues: ";
		for(const auto& val : std::ranges::views::values(map))
			std::cout << val << " ";
		std::cout << "\n" << std::endl;


		delete[] p;
	#endif
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// modules
	{
	#ifdef TST_MODULES
		std::cout << "modules" << std::endl;

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
		std::cout << "format" << std::endl;

		std::string str = std::format("Param 1: {1}, param 0: {0}.", 12, 34.45);
		std::cout << str << std::endl;
	#endif
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// bit
	{
		std::cout << "bit" << std::endl;

		unsigned int i = 0b1011;
		std::cout << "value: " << i << std::endl;
		std::cout << "bit_width: " << std::bit_width(i) << std::endl;
		std::cout << "bit_ceil: " << std::bit_ceil(i) << std::endl;
		std::cout << "bit_floor: " << std::bit_floor(i) << std::endl;
		std::cout << "bit 1 count: " << std::popcount(i) << std::endl;
		std::cout << "rotl: " << std::rotl(i,2) << ", " << (i<<2) << std::endl;
		std::cout << "rotr: " << std::rotr(i,2) << ", " << (i>>2) << std::endl;
	}
	// --------------------------------------------------------------------

	return 0;
}
