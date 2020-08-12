/**
 * concepts test
 * @author Tobias Weber
 * @date aug-20
 * @license: see 'LICENSE.EUPL' file
 *
 * enabled concepts:
 * g++ -std=c++20 -o concepts4 concepts4.cpp
 */

#include <iostream>
#include <type_traits>
#include <concepts>
#include <complex>
#include <ranges>
#include <algorithm>
#include <vector>
#include <deque>


template<class t_real>
void tst()
{
	// selection using type traits
	if constexpr(std::is_same<t_real, float>::value)
		std::cout << "float" << std::endl;
	else if constexpr(std::is_same<t_real, double>::value)
		std::cout << "double" << std::endl;

	// selection using concepts
	if constexpr(std::same_as<t_real, float>)
		std::cout << "float" << std::endl;
	else if constexpr(std::same_as<t_real, double>)
		std::cout << "double" << std::endl;
}


struct Fkt
{
	void operator()(int) const
	{
		std::cout << "Fkt()" << std::endl;
	}
};


class A {};
class B : public A {};


template<class t_range>
void print_range(const t_range& range)
{
	for(const auto& elem : range)
		std::cout << elem << ", ";
	std::cout << std::endl;
}


template<class t_range>
void rangetst(t_range range)
{
	if constexpr(std::sortable<typename t_range::iterator>)
		std::ranges::stable_sort(range);

	print_range(range);
}


template<class t_range>
void heaptst(t_range range)
{
	std::cout << "make_heap: ";
	std::ranges::make_heap(range);
	print_range(range);

	// popped element is moved to last position
	std::cout << "pop_heap: ";
	std::ranges::pop_heap(range);
	print_range(range);
	std::cout << *range.rbegin() << std::endl;

	// reinsert last element in range
	std::cout << "push_heap: ";
	std::ranges::push_heap(range);
	print_range(range);

	std::cout << "sort_heap: ";
	std::ranges::sort_heap(range);
	print_range(range);
}


int main()
{
	tst<float>();
	tst<double>();


	std::cout << "totally ordered(double): " << std::boolalpha
		<< std::totally_ordered<double> << std::endl;
	std::cout << "totally ordered(complex): " << std::boolalpha
		<< std::totally_ordered<std::complex<double>> << std::endl;

	std::cout << "totally ordered with(double, long): " << std::boolalpha
		<< std::totally_ordered_with<double, long> << std::endl;

	std::cout << "invocable(Fkt(int)) " << std::boolalpha
		<< std::invocable<Fkt, int> << std::endl;
	std::cout << "invocable(Fkt()) " << std::boolalpha
		<< std::invocable<Fkt> << std::endl;

	auto fkt = [](){};
	std::cout << "invocable(lam()) " << std::boolalpha
		<< std::invocable<decltype(fkt)> << std::endl;

	std::cout << "B derived from A: " << std::boolalpha
		<< std::derived_from<B,A> << std::endl;
	std::cout << "A derived from B: " << std::boolalpha
		<< std::derived_from<A,B> << std::endl;
	std::cout << "B base of A: " << std::boolalpha
		<< std::is_base_of_v<B,A> << std::endl;
	std::cout << "A base of B: " << std::boolalpha
		<< std::is_base_of_v<A,B> << std::endl;


	rangetst(std::vector<int>{{5, 3, 8, 6}});
	std::cout << std::endl;
	rangetst(std::deque<int>{{5, 3, 8, 6}});
	std::cout << std::endl;

	heaptst(std::vector<int>{{5, 3, 8, 6}});
	std::cout << std::endl;
	heaptst(std::deque<int>{{5, 3, 8, 6}});
	std::cout << std::endl;

	return 0;
}
