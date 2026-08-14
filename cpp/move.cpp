/**
 * move operator test
 * @author Tobias Weber
 * @date aug-2026
 * @license: see 'LICENSE.EUPL' file
 */

// clang++ -Wall -Wextra -Weffc++ -std=c++20 -o move move.cpp


#include <vector>
#include <utility>
#include <iostream>


struct A
{
	A() : val{123}
	{
		std::cout << "A::A(), this = "
			<< reinterpret_cast<void*>(this)
			<< "." << std::endl;
	}

	A(const A& other) : val{other.val}
	{
		std::cout << "A::A() copy, this = "
			<< reinterpret_cast<const void*>(this) << ", other = "
			<< reinterpret_cast<const void*>(&other)
			<< "." << std::endl;
	}

	A(A&& other) noexcept : val{std::exchange(other.val, -1)}
	{
		std::cout << "A::A() move, this = "
			<< reinterpret_cast<const void*>(this) << ", other = "
			<< reinterpret_cast<const void*>(&other)
			<< "." << std::endl;
	}

	~A()
	{
		std::cout << "A::~A(), this = "
			<< reinterpret_cast<const void*>(this) << ", val = "
			<< val << "." << std::endl;
	}

	A& operator=(const A& other)
	{
		val = other.val;
		std::cout << "A copy, this = "
			<< reinterpret_cast<const void*>(this) << ", other = "
			<< reinterpret_cast<const void*>(&other)
			<< "." << std::endl;

		return *this;
	}

	A& operator=(A&& other)
	{
		val = std::exchange(other.val, -1);
		std::cout << "A move, this = "
			<< reinterpret_cast<const void*>(this) << ", other = "
			<< reinterpret_cast<const void*>(&other)
			<< "." << std::endl;

		return *this;
	}

	int val{};
};



struct B
{
	B()
	{
		std::cout << "B::B(), this = "
			<< reinterpret_cast<void*>(this)
			<< "." << std::endl;
	}

	B(const B& other) : a{other.a}
	{
		std::cout << "B::B() copy, this = "
			<< reinterpret_cast<const void*>(this) << ", other = "
			<< reinterpret_cast<const void*>(&other)
			<< "." << std::endl;
	}

	B(B&& other) noexcept : a{std::move(other.a)}
	{
		std::cout << "B::B() move, this = "
			<< reinterpret_cast<const void*>(this) << ", other = "
			<< reinterpret_cast<const void*>(&other)
			<< "." << std::endl;
	}

	~B()
	{
		std::cout << "B::~B(), this = "
			<< reinterpret_cast<const void*>(this)
			<< "." << std::endl;
	}

	B& operator=(const B& other)
	{
		a = other.a;

		std::cout << "B copy, this = "
			<< reinterpret_cast<const void*>(this) << ", other = "
			<< reinterpret_cast<const void*>(&other)
			<< "." << std::endl;

		return *this;
	}

	B& operator=(B&& other)
	{
		a = std::forward<A>(other.a);

		std::cout << "B move, this = "
			<< reinterpret_cast<const void*>(this) << ", other = "
			<< reinterpret_cast<const void*>(&other)
			<< "." << std::endl;

		return *this;
	}

	A a{};
};



int main()
{
	std::cout << "B a;" << std::endl;
	B a;

	std::cout << "\nB b{a};" << std::endl;
	B b{a};

	std::cout << "\nB c{std::move(a)};" << std::endl;
	B c{std::move(a)};

	std::cout << "\nB d; d = std::move(b);" << std::endl;
	B d; d = std::move(b);

	std::cout << "\nscope end" << std::endl;
	return 0;
}
