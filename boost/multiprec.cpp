/**
 * multiprecision tests
 * @author Tobias Weber
 * @date aug-2021
 * @license: see 'LICENSE.GPL' file
 *
 * References:
 *	* https://www.boost.org/doc/libs/1_77_0/libs/multiprecision/doc/html/index.html
 */

#include <iostream>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>
namespace multiprec = boost::multiprecision;


/**
 * @see https://en.wikipedia.org/wiki/Collatz_conjecture
 */
template<class t_int = unsigned int>
t_int collatz(const t_int& start, const t_int& idx)
{
	if(idx == 0)
		return start;

	auto coll = [](const t_int& num) -> t_int
	{
		if(num % 2 == 0)
			return num/2;
		return 3*num + 1;
	};

	return coll(collatz<t_int>(start, idx-1));
}


int main()
{
	//using t_int = multiprec::uint1024_t;
	using t_int = multiprec::cpp_int;
	//using t_int = multiprec::mpz_int;

	const t_int max_idx = 1000;
	t_int start = 12345;

	for(t_int idx=0; idx < max_idx; ++idx)
	{
		t_int num = collatz<t_int>(start, idx);

		std::cout << num << ", ";
		std::cout.flush();

		if(num == 1)
			break;
	}

	std::cout << std::endl;

	return 0;
}
