/**
 * C++17 compatibility tests
 * @author Tobias Weber
 * @date 11-nov-17
 *
 * gcc -o cpp17 cpp17.cpp -std=c++17 -lstdc++
 */

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <array>
#include <any>
#include <functional>
#include <tuple>
#include <boost/type_index.hpp>
namespace ty = boost::typeindex;


void print(const auto&... a)
{
	(std::cout << "types: " << ... << (ty::type_id_with_cvr<decltype(a)>().pretty_name() + ", "));
	(std::cout << "values: " << ... << a) << std::endl;
}


// ----------------------------------------------------------------------------
std::array<int, 3> arr()
{
	return {1, 2, 3};
}

std::array<int, 3>& arr2()
{
	static std::array<int, 3> a{{1, 2, 3}};
	return a;
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// constant expressions
void modify(auto& a)
{
	if constexpr(std::is_same_v<decltype(a), int&>)
		++a;
	else
		--a;
}
// ----------------------------------------------------------------------------



int main()
{
	// --------------------------------------------------------------------
	// byte type
	{
		std::byte by{0x12};
		print(std::to_integer<int>(by));
	}
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	// [] = ...
	{
		auto [i, j, k] = arr();
		print(i, " ", j, " ", k);
		auto& [l, m, n] = arr2();
		l = 10;
		print(l, " ", m, " ", n);
	}
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	// new if statement
	if(std::ifstream ifstr("./cpp17.cpp"); ifstr.is_open())
	{
		std::string str;
		std::getline(ifstr, str);
		std::cout << "File open: " << str << std::endl;
	}
	else
	{
		std::cout << "File could not be opened." << std::endl;
	}
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	{
		int x = 0;
		long y = 0;
		modify(x);
		modify(y);
		print(x, " ", y);
	}
	// --------------------------------------------------------------------



	// --------------------------------------------------------------------
	// any
	{
		std::any a(123);
		print(std::any_cast<int>(a));
		a = std::string("123");
		print(std::any_cast<std::string&>(a));
	}
	// --------------------------------------------------------------------



	// --------------------------------------------------------------------
	// string_view
	{
		const char* pc = "Test 123";
		std::basic_string_view<char> sv(pc, 4);
		std::cout << sv << std::endl;
	}
	// --------------------------------------------------------------------



	// --------------------------------------------------------------------
	// invoke, apply, make_from_tuple
	{
		std::invoke(::print<int, char, int>, 123, ' ', 456);


		auto tup = std::make_tuple(456, 789.);
		std::apply(::print<int, double>, tup);


		struct A
		{
			int i; double d;
			A(int i, double d) : i(i), d(d) {}
		};
		A a{std::make_from_tuple<A>(tup)};
		print(a.i, " ", a.d);
	}
	// --------------------------------------------------------------------


	return 0;
}
