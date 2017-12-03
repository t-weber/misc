/**
 * signals/slots tests and snippets
 * @author Tobias Weber
 * @date 18-nov-17
 * @license: see 'LICENSE' file
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_65_1/doc/html/signals2.html
 *  * https://github.com/boostorg/signals2/tree/develop/example
 *
 * gcc -o sig sig.cpp -std=c++17 -lstdc++
 */

#include <iostream>

#include <boost/signals2/signal.hpp>
namespace sig = boost::signals2;


struct A
{
	void operator()() { std::cout << "In struct.\n"; }
};


struct B
{
	using t_sig = sig::signal<void()>;
	t_sig m_sig;

	template<class t_slot> void connect(const t_slot& slot)
	{ m_sig.connect(slot); }
	template<class t_slot> void disconnect(const t_slot& slot)
	{ m_sig.disconnect(slot); }

	void emit() { m_sig(); }
};

struct C
{
	void slot()
	{
		std::cout << "In C::slot();\n";
	}
};


int main()
{
	// signal / slot
	{
		sig::signal<void()> sig;
		sig.connect([]() { std::cout << "Signal 1.\n"; });
		{
			sig::scoped_connection _sc(sig.connect([]() { std::cout << "Temporary signal 2.\n"; }));
			sig();
		}
		sig();
	}

	std::cout << "\n";

	// self-disconnecting slot
	{
		sig::signal<void()> sig;
		sig.connect_extended([](sig::connection conn) { std::cout << "Signal 3.\n"; conn.disconnect(); });
		sig();
		sig();
	}

	std::cout << "\n";

	// signal / multiple slots
	{
		sig::signal<int(int)> sig;
		sig.connect([](int i) -> int { return i+1; });
		sig.connect([](int i) -> int { return i+2; });
		auto optRet = sig(123);
		std::cout << *optRet << "\n";
	}

	std::cout << "\n";

	// member functions
	{
		A a;
		// direct
		void(A::*p)() = &A::operator();
		(a.*p)();

		sig::signal<void()> sig;
		sig.connect([&a]() { a(); });
		sig();
	}

	std::cout << "\n";

	// more member functions
	{
		B b;
		b.connect([]() { std::cout << "Member signal.\n"; });

		C c;
		b.connect([&c]() { c.slot(); });

		b.emit();
	}

	return 0;
}
