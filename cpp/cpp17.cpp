/**
 * C++17 compatibility tests
 * @author Tobias Weber
 * @date 11-nov-17
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++17 -fconcepts -Wall -Wextra -o cpp17 cpp17.cpp
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <array>
#include <optional>
#include <variant>
#include <any>
#include <algorithm>
#include <functional>
#include <tuple>
#include <utility>
#include <numeric>
#include <unordered_map>
#include <experimental/algorithm>
#include <experimental/functional>
#include <experimental/string>
#include <boost/type_index.hpp>
namespace ty = boost::typeindex;


template<class t_a, class t_b>
std::string strconcat(const t_a& a, const t_b& b)
{
	std::ostringstream ostr;
	ostr << a << b;
	return ostr.str();
}


template<class ...t_args> void print(const t_args&... a)
//void print(const auto&... a)
{
	std::cout << "types: ";
	(std::cout << ... << (ty::type_id_with_cvr<decltype(a)>().pretty_name() + ", "));

	std::cout << "values: ";
	(std::cout << ... << strconcat(a, ", ")) << std::endl;
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
constexpr void modify(auto& a)
{
	if constexpr(std::is_same_v<decltype(a), int&>)
		++a;
	else
		--a;
}


// constexpr loop using fold expressions
template</*class Func,*/ std::size_t ...seq>
constexpr void constexpr_loop(const std::index_sequence<seq...>&, /*Func*/auto func)
{
	// a sequence of function calls
	( (func(seq)), ... );
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// index_sequences
template<std::size_t ...seq>
auto mk_tup(const auto& iter, std::index_sequence<seq...>)
{
	return std::make_tuple(*(iter + seq)...);
}

template<std::size_t ...seq>
void pr_tup(const auto& tup, std::index_sequence<seq...>)
{
	using t_tup = std::decay_t<decltype(tup)>;
	std::apply(::print<std::tuple_element_t<seq, t_tup>...>, tup);
}
// ----------------------------------------------------------------------------



struct Tst
{
	Tst(int val) : val{val}
	{
		std::cout << __func__ << std::endl;
	}

	~Tst()
	{
		std::cout << __func__ << std::endl;
	}

	int val{0};
};


int main()
{
	// --------------------------------------------------------------------
	// byte type
	{
		std::byte by{0x12};
		print(std::to_integer<int>(by));
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


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


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// [] = ...
	{
		auto [i, j, k] = arr();
		print(i, j, k);
		auto& [l, m, n] = arr2();
		l = 10;
		print(l, m, n);

		std::unordered_map<std::string, int> map{{{"Test"}, 123}};
		if(auto [iter, bOk] = map.insert({"newtest", 456}); bOk)
			std::cout << "OK: " << iter->first << ", " << iter->second << std::endl;
		else
			std::cout << "failed" << std::endl;
		if(auto [iter, bOk] = map.insert({"Test", 456}); bOk)
			std::cout << "OK: " << iter->first << ", " << iter->second << std::endl;
		else
			std::cout << "failed" << std::endl;

		if(auto iter = map.find("Test"); iter != map.end())
			std::cout << "OK: " << iter->first << ", " << iter->second << std::endl;
		else
			std::cout << "failed" << std::endl;

		if(auto node = map.extract("Test"); !node.empty())
			std::cout << "node: " << node.key() << ", " << node.mapped() << std::endl;
		if(auto node = map.extract("Test"); !node.empty())
			std::cout << "node: " << node.key() << ", " << node.mapped() << std::endl;
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	{
		int x = 0;
		long y = 0;
		modify(x);
		modify(y);
		print(x, y);
	}


	// compile-time loop unrolling
	{
		std::vector<std::string> vecStr{{"0", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix", "x"}};
		std::vector<std::string> vec;
		auto seq = std::index_sequence<0, 3, 5, 7, 9>();
		//auto seq = std::make_index_sequence<11>();
		constexpr_loop(seq, [&vec, &vecStr](std::size_t idx) -> void
		{
			vec.push_back(vecStr[idx]);
		});

		for(const auto& idx : vec) std::cout << idx << ", ";
		std::cout << std::endl;
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// any
	{
		std::any a(123);
		print(std::any_cast<int>(a));
		a = std::string("123");
		print(std::any_cast<std::string&>(a));
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// string_view
	{
		const char* pc = "Test 123";
		std::basic_string_view<char> sv(pc, 4);
		std::cout << sv << std::endl;
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// invoke, apply, make_from_tuple
	{
		std::invoke(::print<int, int>, 123, 456);


		auto tup = std::make_tuple(456, 789.);
		std::apply(::print<int, double>, tup);


		struct A
		{
			int i; double d;
			A(int i, double d) : i(i), d(d) {}
		};
		A a{std::make_from_tuple<A>(tup)};
		print(a.i, a.d);


		int arr[] = { 1, 2, 3, 4, 5};
		auto seq = std::make_index_sequence<3>();
		auto tupfromarr = mk_tup(arr, seq);
		std::cout << ty::type_id_with_cvr<decltype(tupfromarr)>().pretty_name() << std::endl;
		//std::apply(::print<int,int,int>, tupfromarr);
		pr_tup(tupfromarr, seq);
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// misc
	{
		std::string str("TEST123ABCDEF");
		std::string str2("123");

		// search
		auto searcher = std::experimental::make_default_searcher(str2.begin(), str2.end());
		auto iter = std::experimental::search(str.begin(), str.end(), searcher);
		std::cout << "pos: " << iter-str.begin() << std::endl;

		// erase
		std::experimental::erase(str, 'T');
		std::cout << str << std::endl;

		// erase_if
		std::experimental::erase_if(str, [](char c)->bool { return c == 'E'; });
		std::cout << str << std::endl;
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// optional
	{
		auto opt = std::make_optional<int>(987654);
		if(opt)
			std::cout << *opt << std::endl;
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// variant
	{
		std::variant<int, char, std::string> var, var2;

		var = 'X';
		std::cout << std::get<char>(var) << std::endl;

		var = "123";
		std::cout << std::get<2>(var) << ", " << std::get<std::string>(var) << std::endl;

		var = 123;
		std::cout << std::get<0>(var) << std::endl;

		std::cout << std::holds_alternative<std::string>(var) << ", "
			<< *std::get_if<int>(&var) << std::endl;

		var2 = "345";
		std::swap(var, var2);
		std::cout << std::get<int>(var2) << std::endl;


		// visitors
                std::visit([](const auto& arg) -> void
                {
			print(arg);
                }, var);

                std::visit([](const auto& ...args) -> void
                {
			print(args...);
                }, var, var2);


		using t_var3 = std::variant<
			std::monostate /*dummy first type to default-construct*/,
				int, int>;
		t_var3 var3;
		std::cout << var3.index() << std::endl;
		var3 = t_var3{std::in_place_index<1>, 123};
		std::cout << var3.index() << std::endl;
		var3 = t_var3{std::in_place_index<2>, 123};
		std::cout << var3.index() << std::endl;
	}
	// --------------------------------------------------------------------


	std::cout << std::endl;


	// --------------------------------------------------------------------
	// numeric
	{
		std::cout << "gcd(10, 5) = " << std::gcd(10, 5) << std::endl;
		std::cout << "lcm(7, 5) = " << std::lcm(7, 5) << std::endl;
	}
	// --------------------------------------------------------------------



	std::cout << std::endl;


	// --------------------------------------------------------------------
	// placement new
	// see: https://en.cppreference.com/w/cpp/language/new
	{
		char stack_mem[sizeof(Tst)];
		Tst *tst = new(stack_mem) Tst(123);
		std::cout << tst->val << std::endl;
		std::cout << std::hex << tst << " " << static_cast<void*>(stack_mem) << std::endl;
		tst->~Tst();
		//delete tst;
	}
	// --------------------------------------------------------------------

	return 0;
}
