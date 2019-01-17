/**
 * quick tests
 * @author Tobias Weber
 * @date jan-19
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -o misc misc.cpp -std=c++17
 */

#include <iostream>


struct Tst1
{
	int a = 123;
	int b = 456;
	int c = 789;
};


struct Tst2
{
	Tst2()
	{
		std::cout << __FUNCTION__ << std::endl;
	}

	Tst2(int a)
	{
		std::cout << __FUNCTION__ << ", " << a << std::endl;
	}

	~Tst2()
	{
		std::cout << __FUNCTION__ << std::endl;
	}
};


struct Tst3 : public Tst2
{
	Tst3() : Tst2{111}
	{
		std::cout << __FUNCTION__ << std::endl;
	}

	using Tst2::Tst2;

	~Tst3()
	{
		std::cout << __FUNCTION__ << std::endl;
	}
};


int main()
{
	Tst1 tst1{.a = 5, .c = 10};
	std::cout << tst1.a << " " << tst1.b << " " << tst1.c << std::endl;

	Tst3 tst3;
	Tst3 tst3b{222};

	return 0;
}
