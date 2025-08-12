/**
 * C++23 compatibility tests
 * @author Tobias Weber
 * @date may-2022
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -g -Wall -Wextra -Weffc++ -std=c++23 -fmodules -o cpp23 cpp23.cpp -lstdc++exp
 */

#define USE_MODULES 1


#if USE_MODULES == 0
	//#include <iostream>
	#include <print>
	#include <format>
	#include <memory>
	#include <expected>
	#include <string>
	#include <cmath>

	#if __has_include(<stacktrace>)
		#include <stacktrace>
		#define USE_STACKTRACE
	#endif
#endif


#include <boost/type_index.hpp>
namespace ty = boost::typeindex;


#if USE_MODULES != 0
	import std;
	#define USE_STACKTRACE
#endif


enum class E : int
{
	Val1,
	Val2,
};


struct A
{
	// multiple arguments for operator[]
	int operator[](int i1, int i2, int i3)
	{
		return i1 * i2 * i3;
	}
};


struct B
{
	std::shared_ptr<B> create()
	{
		//using T = B;
		using T = decltype(auto{*this});

		std::println("{}", ty::type_id<T>().pretty_name());
		return std::make_shared<T>();
	}
};


struct C
{
	int a{}, b{};

	C(int a, int b) : a{a}, b{b}
	{}
};


/**
 * formatter for struct C
 * for example formatters, see: https://github.com/llvm/llvm-project/blob/main/libcxx/include/__format/formatter_tuple.h
 */
template<class t_C, class t_char>
requires requires(const t_C& c)
{
	// constraint for structs with two members, 'a' and 'b'
	c.a;
	c.b;
}
// partially specialise std::formatter
struct std::formatter<t_C, t_char>
{
public:
	/**
	 * print struct t_C
	 */
	template<class t_context, class t_iter = typename t_context::iterator>
	t_iter format(const t_C& c, t_context& context) const
	{
		constexpr const int format_func = 1;

		// version 1: print to string and copy it to out()
		if constexpr(format_func == 0)
		{
			using t_str = std::basic_string<t_char>;

			// get string representation of struct t_C
			t_str str;
			if(m_print_a && m_print_b)
				str = std::format("[ {}, {} ]", c.a, c.b);
			else if(m_print_a)
				str = std::format("[ {} ]", c.a);
			else if(m_print_b)
				str = std::format("[ {} ]", c.b);
			else
				str = std::format("[ ]");

			// write out the string
			t_iter iter = context.out();
			for(t_char c : str)
				*iter++ = c;

			return iter;
		}

		// version 2: directly write to out()
		else if constexpr(format_func == 1)
		{
			if(m_print_a && m_print_b)
				std::format_to(context.out(), "[ {}, {} ]", c.a, c.b);
			else if(m_print_a)
				std::format_to(context.out(), "[ {} ]", c.a);
			else if(m_print_b)
				std::format_to(context.out(), "[ {} ]", c.b);
			else
				std::format_to(context.out(), "[ ]");
		}

		return context.out();
	}


	/**
	 * parse format specifier after the ":" in, e.g., "{:ab}"
	 */
	template<class t_context, class t_iter = typename t_context::iterator>
	constexpr t_iter parse(t_context& context)
	{
		t_iter iter = context.begin();

		if(iter != context.end() && *iter == t_char('a'))
		{
			m_print_a = true;
			std::advance(iter, 1);
		}

		if(iter != context.end() && *iter == t_char('b'))
		{
			m_print_b = true;
			std::advance(iter, 1);
		}

		// ignore closing '}'
		context.advance_to(context.end());
		return iter;
	}

private:
	bool m_print_a{ false }, m_print_b{ false };
};


template<class t_real = double>
std::expected<t_real, std::string> tst_sqrt(t_real f)
{
	if(f >= 0.)
		return std::sqrt(f);
	else
		return std::unexpected("negative number");
}


// if consteval test (for rvalues)
template<class T>
constexpr T tst_consteval(T&& t)
{
	if consteval  // is the context consteval?
	{
		// compile-time code
		return std::forward<T>(t) + std::forward<T>(t);
	}
	else
	{
		// run-time code
		return 3 * std::forward<T>(t);
	}
}


// if consteval test (for lvalues)
template<class T>
constexpr T tst_consteval(const T& t)
{
	if consteval  // is the context consteval?
	{
		// compile-time code
		return t + t + t + t;
	}
	else
	{
		// run-time code
		return 5 * t;
	}
}


// compile-time constant
constinit int g_i = tst_consteval<int>(100);


#ifdef USE_STACKTRACE
void tst_stacktrace()
{
	std::stacktrace trace = std::stacktrace::current();

	for(std::size_t i = 0; i < trace.size(); ++i)
	{
		const std::stacktrace_entry& entry = trace[i];
		if(!entry)
			continue;

		std::println("file \"{}\", line {}: {}",
			entry.source_file(), entry.source_line(), entry.description());
	}
}
#endif


// container with multi-dim operator[]
template<class t_val, std::size_t... SIZES>
struct Cont
{
	t_val vals[(SIZES * ...)]{};

	template<class ...t_idx>
	t_val& operator[](t_idx ..._indices)
	{
		constexpr std::size_t DIM = sizeof...(SIZES);
		constexpr std::array<std::size_t, DIM> sizes{{ SIZES... }};
		const std::array<std::size_t, DIM> indices
			{{ static_cast<std::size_t>(_indices)... }};
		//std::println("sizes: {}, indices: {}.", sizes, indices);

		std::size_t lin_idx{};
		for(std::size_t i = 0; i < DIM - 1; ++i)
		{
			std::size_t size = 1;
			for(std::size_t j = i + 1; j < DIM; ++j)
				size *= sizes[j];
			lin_idx += indices[i] * size;
		}
		lin_idx += indices[DIM - 1];
		//std::println("linear index: {}.", lin_idx);

		return vals[lin_idx];
	}
};


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

	C c{123, 987};
	std::println("c = {}.", c);
	std::println("c = {:ab}.", c);
	std::println("c = {:a}.", c);
	std::println("c = {:b}.", c);
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


	// --------------------------------------------------------------------
	std::println("if consteval");
	//if !consteval { std::println("not consteval"); }

	constexpr int i = 100;
	constexpr int j = tst_consteval(i);
	int x = 100;
	std::println("tst_consteval([constinit rval] 100) = {}.", g_i);
	std::println("tst_consteval([const     rval] 100) = {}.", tst_consteval(100));
	std::println("tst_consteval([constexpr lval] 100) = {}.", j);
	std::println("tst_consteval([non-const lval] 100) = {}.", tst_consteval<int>(x));
	// --------------------------------------------------------------------


	std::println();


	#ifdef USE_STACKTRACE
	// --------------------------------------------------------------------
	std::println("stacktrace");

	tst_stacktrace();
	// --------------------------------------------------------------------


	std::println();
	#endif


	// --------------------------------------------------------------------
	std::println("operator[](...)");

	Cont<double, 2, 2> cont;
	cont[0, 0] = 1.; cont[0, 1] = 2.;
	cont[1, 0] = 3.; cont[1, 1] = 4.;
	std::println("{}", cont.vals);
	// --------------------------------------------------------------------


	std::println();

	return 0;
}
