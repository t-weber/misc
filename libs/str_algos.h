/**
 * string algos
 * @author Tobias Weber
 * @date apr-2021
 * @license see 'LICENSE.EUPL' file
 *
 * references:
 *   - (FUH 2021) "Effiziente Algorithmen" (2021), Kurs 1684, Fernuni Hagen
 *                (https://vu.fernuni-hagen.de/lvuweb/lvu/app/Kurs/01684).
 */

#ifndef __STR_ALGOS_H__
#define __STR_ALGOS_H__


#include <vector>


/**
 * KMP pattern matching algo
 * @see (FUH 2021), Kurseinheit 3, p. 9 and p. 11.
 * @see https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm
 */
template<class t_str>
std::size_t find_pattern(const t_str& str, const t_str& pattern)
requires requires(const t_str& str, std::size_t idx)
{
	// needed t_str interface
	str[idx];
	str.size();
}
{
	std::size_t len_str = str.size();
	std::size_t len_pattern = pattern.size();


	auto get_prefix = [](const t_str& pattern, std::size_t len_pattern)
		-> std::vector<std::size_t>
	{
		std::vector<std::size_t> prefix{0};

		for(std::size_t pattern_pos=1; pattern_pos<len_pattern; ++pattern_pos)
		{
			std::size_t prefix_pos = prefix[pattern_pos-1];

			while(prefix_pos > 0 && pattern[prefix_pos] != pattern[pattern_pos])
				prefix_pos = prefix[prefix_pos-1];

			prefix.push_back(pattern[prefix_pos] == pattern[pattern_pos] ? prefix_pos+1 : 0);
		}

		prefix.insert(prefix.begin(), 0);
		return prefix;
	};


	auto prefix = get_prefix(pattern, len_pattern);
	//for(std::size_t i : prefix)
	//	std::cout << i << " ";
	//std::cout << std::endl;


	std::size_t str_pos = 0;
	std::size_t start_pos = 0;

	while(str_pos < len_str)
	{
		if(pattern[str_pos-start_pos] == str[str_pos])
		{
			if(str_pos-start_pos+1 == len_pattern)
				return start_pos;
			else
				++str_pos;
		}
		else
		{
			if(start_pos != str_pos)
			{
				start_pos = str_pos - prefix[str_pos-start_pos];
			}
			else
			{
				++start_pos;
				++str_pos;
			}
		}
	}

	// pattern not found
	return len_str;
}


#endif
