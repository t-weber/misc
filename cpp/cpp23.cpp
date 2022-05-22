/**
 * C++23 compatibility tests
 * @author Tobias Weber
 * @date may-2022
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++23 -o cpp23 cpp23.cpp
 */

#include <iostream>
#include <memory>
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

		std::cout << ty::type_id<T>().pretty_name() << std::endl;
		return std::make_shared<T>();
	}
};


int main()
{
	// --------------------------------------------------------------------
	std::cout << "std::to_underlying enum type" << std::endl;

	std::cout << std::to_underlying(E::Val1) << std::endl;
	std::cout << std::to_underlying(E::Val2) << std::endl;
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	std::cout << "multi-argument operator[]" << std::endl;

	A a;
	std::cout << a[1, 2, 3] << std::endl;
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	std::cout << "auto{} cast" << std::endl;

	B b1;
	auto b2 = b1.create();
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	std::cout << "lambda attributes" << std::endl;

	auto fkt = [][[nodiscard("return value is discarded")]](auto i, auto j) -> decltype(i+j)
	{
		return (i + j) * (i - j);
	};
	std::cout << fkt(2, 3) << std::endl;
	//fkt(3, 4);  // shows warning for discarded return value
	// --------------------------------------------------------------------


	return 0;
}
