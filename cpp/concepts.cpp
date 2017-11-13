/**
 * concepts test
 * @author Tobias Weber
 * @date 11-nov-17
 *
 * gcc -o concepts concepts.cpp -std=c++17 -fconcepts -lstdc++
 */

#include <iostream>
#include <string>
#include <type_traits>
#include <boost/type_index.hpp>
namespace ty = boost::typeindex;


// ----------------------------------------------------------------------------
// implicit templates

void print(const auto& a)
{
	using T = decltype(a);
	std::cout << "type: " << ty::type_id_with_cvr<T>().pretty_name();
	std::cout << ", value: " << a << std::endl;
}

void print(const auto& a, const auto&... rest)
{
	print(a);
	print(rest...);
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// constrained functions

template<typename T> concept bool c_onlyint = std::is_integral<T>::value;

void print_constrained(const c_onlyint& i)
{
	std::cout << "int: " << i << std::endl;
}

// alternate form
template<class T> requires std::is_floating_point<T>::value
void print_constrained(const T& f)
{
	std::cout << "float: " << f << std::endl;
}


// only 'bool' type allowed
//template<typename T> concept int c_num = 12;
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// requirement constraints

// requires a class which as a member named "fkt()"
template<class T>
concept bool has_func = requires(const T& a) { a.fkt(); };

struct HasFkt { int fkt() const { return 159; }};
struct NoFkt { };

void print_fkt(const auto& a) requires has_func<decltype(a)>
{
	std::cout << a.fkt() << std::endl;
}


// constrain to classes with a value_type
template<class T>
concept bool has_value_type = requires { typename T::value_type; };

template<has_value_type T> void print_value_type()
{
	std::cout << ty::type_id_with_cvr<typename T::value_type>().pretty_name() << std::endl;
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// variadic folding

auto addall(const auto&& ...a)
{
	return (0 + ... + a);
}
// ----------------------------------------------------------------------------



int main()
{
	print("Test", " ", 123, ", ", 4.56);

	print_constrained(987);
	print_constrained(987.);

	HasFkt a; print_fkt(a);
	//NoFkt b; print_fkt(b);

	print_value_type<std::string>();
	//print_value_type<int>();


	// ----------------------------------------------------------------------------
	// constrained template lambda
	auto lam = [](const c_onlyint& a) -> void
	{
		std::cout << "lam: " << a << std::endl;
	};
	lam(123);
	//lam(123.);
	// ----------------------------------------------------------------------------

	std::cout << "add: " << addall(1,2,3,4) << std::endl;

	return 0;
}
