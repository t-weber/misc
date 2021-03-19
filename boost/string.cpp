/**
 * boost string tests
 * @author Tobias Weber
 * @date 18-mar-18
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://github.com/boostorg/algorithm/tree/develop/string/example
 *  * http://www.boost.org/doc/libs/1_66_0/doc/html/string_algo/usage.html
 *
 * gcc -o string string.cpp -std=c++17 -lstdc++ -lm
 */

#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <iostream>
#include <vector>



template<class t_str=std::string, template<class...> class t_cont=std::vector>
t_cont<t_str> tokenise(const t_str& _str, const t_str& strSeparators=" \t")
{
	t_cont<t_str> vec;

	// separator predicate
	auto is_sep = [&strSeparators](auto c) -> bool
	{
		for(auto csep : strSeparators)
		{
			if(c == csep)
				return true;
		}
		return false;
	};

	// trim whitespaces
	t_str str = boost::trim_copy_if(_str, is_sep);
	boost::split(vec, str, is_sep, boost::token_compress_on);


	// remove empty
	/*auto newend = std::remove_if(vec.begin(), vec.end(), [&is_sep](const t_str& s) -> bool
	{
		if(boost::trim_copy_if(s, is_sep) == "")
			return true;
		return false;
	});
	vec.resize(newend - vec.begin());*/


	return vec;
}



int main()
{
	auto is_whitespace = [](char c) -> bool {return c==' ' || c=='\t';};


	// conversions
	{
		std::string str = " \tABCDEFGH  ";
		boost::trim(str);
		std::cout << "\"" <<  boost::to_lower_copy(str) << "\"\n";
	}


	// splitting
	{
		std::vector<std::string> vec;
		std::string str = " ABC def \t  \t ghi\tjkl  ";

		boost::split(vec, str, is_whitespace, boost::token_compress_off);

		std::cout << "no token compress: \"" << str << "\" -> ";
		for(const auto& s : vec)
			std::cout << "\"" << s << "\", ";
		std::cout << "\n";



		vec.clear();
		boost::split(vec, str, is_whitespace, boost::token_compress_on);

		std::cout << "token compress: \"" << str << "\" -> ";
		for(const auto& s : vec)
			std::cout << "\"" << s << "\", ";
		std::cout << "\n";



		vec.clear();
		vec = tokenise(str);

		std::cout << "tokenise: \"" << str << "\" -> ";
		for(const auto& s : vec)
			std::cout << "\"" << s << "\", ";
		std::cout << "\n";
	}


	return 0;
}
