/**
 * combinatorics test
 * @author Tobias Weber
 * @date 23-mar-2025
 * @license see 'LICENSE.GPL' file
 *
 * g++ -std=c++20 -I../libs -Wall -Wextra -Weffc++ -o comb comb.cpp
 */

#include <algorithm>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>


/**
 * prints the elements of a container
 */
template<class t_cont>
void print_cont(const t_cont& cont, std::ostream& ostr = std::cout)
{
	for(auto iter = cont.begin(); iter != cont.end(); std::advance(iter, 1))
	{
		ostr << *iter;
		if(std::next(iter, 1) != cont.end())
			ostr << ", ";
	}
	ostr << std::endl;
}


/**
 * calculates the expected number of permutations
 */
template<class t_cont>
std::size_t num_perm(const t_cont& cont)
{
	auto fac = []<typename t_int>(t_int n) -> t_int
	{
		t_int res = 1;

		for(t_int i = 2; i <= n; ++i)
			res *= i;

		return res;
	};

	using t_elem = typename t_cont::value_type;

	// number of element repetitions
	std::unordered_map<t_elem, std::size_t> num_rep;

	for(auto iter = cont.begin(); iter != cont.end(); std::advance(iter, 1))
	{
		if(auto map_iter = num_rep.find(*iter); map_iter != num_rep.end())
			++map_iter->second;
		else
			num_rep.insert(std::make_pair(*iter, 1));
	}

	// full permutation without repetitions
	std::size_t num = fac(cont.size());

	// take repetitions into account
	for(const auto& pair : num_rep)
	{
		if(pair.second <= 1)
			continue;

		num /= fac(pair.second);
	}

	return num;
}


/**
 * gets a vector with the given element type from a string
 */
template<typename t_elem, template<class...> class t_cont = std::vector>
t_cont<t_elem> get_seq(const std::string& strseq)
{
	std::vector<std::string> strvec;
	boost::split(strvec, strseq, [](auto c) -> bool
	{
		return c == ',' || c == ' ';
	}, boost::token_compress_on);

	t_cont<t_elem> vec;
	for(const std::string& str : strvec)
	{
		if(str.length() == 0)
			continue;

		t_elem num{};
		std::istringstream{str} >> num;
		vec.emplace_back(std::move(num));
	}

	return vec;
}


int main()
{
	using t_elem = int;
	using t_vec = std::vector<t_elem>;
	using t_iter = typename t_vec::iterator;

	std::cout << "Enter an integer sequence: ";
	std::string strseq;
	std::getline(std::cin, strseq);

	t_vec vec = get_seq<t_elem>(strseq);

	// initial permutation
	print_cont(vec);

	std::size_t num_perms = 1;
	while(true)
	{
		std::ranges::in_found_result<t_iter> res =
			std::ranges::next_permutation(vec,
		[](const t_elem& elem1, const t_elem& elem2) -> bool
		{
			return elem1 < elem2;
		});
		if(!res.found)
			break;

		print_cont(vec);
		++num_perms;
	}

	std::cout << num_perms << " permutations"
		<< " (expected " << num_perm(vec) << ")."
		<< std::endl;
	return 0;
}
