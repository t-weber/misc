/**
 * - single dispatch only exists as virtual functions
 * - functions are only overloaded on the static type
 * @author Tobias Weber
 * @date 18-jan-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Visitor_pattern
 */

#include <iostream>


class Base
{
public:
	virtual void tst1() const
	{
		std::cout << "in base class" << std::endl;
	}

	void tst2() const
	{
		std::cout << "in base class" << std::endl;
	}
};


class Derived : public Base
{
public:
	virtual void tst1() const override
	{
		std::cout << "in derived class" << std::endl;
	}

	void tst2() const
	{
		std::cout << "in derived class" << std::endl;
	}
};


void dispatch_tst(const Base* b)
{
	std::cout << "function for base class" << std::endl;

	b->tst1();
	b->tst2();
}


void dispatch_tst(const Derived* b)
{
	std::cout << "function for derived class" << std::endl;

	b->tst1();
	b->tst2();
}


int main()
{
	Base *b = new Derived{};
	dispatch_tst(b);
	delete b;

	return 0;
}
