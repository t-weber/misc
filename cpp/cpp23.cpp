/**
 * C++23 compatibility tests
 * @author Tobias Weber
 * @date may-2022
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++23 -o cpp23 cpp23.cpp
 */

//#include <iostream>
#include <print>
#include <format>
#include <memory>
#include <expected>
#include <string>
#include <cmath>
//#include <stacktrace>

#include <boost/type_index.hpp>
namespace ty = boost::typeindex;


enum class E : int
{
	Val1,
	Val2,
};


class A
{
public:
	// multiple arguments for operator[]
	int operator[](int i1, int i2, int i3)
	{
		return i1 * i2 * i3;
	}
};


class B
{
public:
	std::shared_ptr<B> create()
	{
		//using T = B;
		using T = decltype(auto{*this});

		std::println("{}", ty::type_id<T>().pretty_name());
		return std::make_shared<T>();
	}
};


template<class t_real = double>
std::expected<t_real, std::string> tst_sqrt(t_real f)
{
	if(f >= 0.)
		return std::sqrt(f);
	else
		return std::unexpected("negative number");
}


int main()
{
	// --------------------------------------------------------------------
	std::println("formatted printing");

	double d1 = 1.2345678901, d2 = 2.3456789012;
	int len = 10, prec = 4;
	std::println("{:>10}{:>10}", "val1", "val2");
	std::println("{:10.4f}{:10.4f}", d1, d2);
	std::println("{:{}.{}f}{:{}.{}f}", d1, len, prec, d2, len, prec);
	std::println("{2:{0}.{1}f}{3:{0}.{1}f}", len, prec, d1, d2);

	//std::cout << std::format("{2:{0}.{1}f}{3:{0}.{1}f}", len, prec, d1, d2) << std::endl;
	// --------------------------------------------------------------------


	std::println();


	// --------------------------------------------------------------------
	std::println("std::to_underlying enum type");

	std::print("{}\n{}\n", std::to_underlying(E::Val1), std::to_underlying(E::Val2));
	// --------------------------------------------------------------------


	std::println();


	// --------------------------------------------------------------------
	std::println("multi-argument operator[]");

	A a;
	std::println("{}", a[1, 2, 3]);
	// --------------------------------------------------------------------


	std::println();


	// --------------------------------------------------------------------
	std::println("auto{{}} cast");

	B b1;
	auto b2 = b1.create();
	// --------------------------------------------------------------------


	std::println();


	// --------------------------------------------------------------------
	std::println("lambda attributes");

	auto fkt = [][[nodiscard("return value is discarded")]](auto i, auto j) -> decltype(i + j)
	{
		return (i + j) * (i - j);
	};
	std::println("{}", fkt(2, 3));
	//fkt(3, 4);  // shows warning for discarded return value
	// --------------------------------------------------------------------


	std::println();


	// --------------------------------------------------------------------
	std::println("expected");

	double d = 16.;
	//double d = -16.;
	auto val = tst_sqrt(d);
	if(val.has_value())
		std::println("sqrt({}) = {}", d, *val);
	else
		std::println(stderr, "Error: {}.", val.error());

	tst_sqrt(d).and_then([d](double val) -> std::expected<double, std::string>
	{
		std::println("sqrt({}) = {}", d, val);
		return val;
	}).or_else([](const std::string& str) -> std::expected<double, std::string>
	{
		std::println("Error: {}.", str);
		return 0.;
	});
	// --------------------------------------------------------------------


	std::println();

	return 0;
}
