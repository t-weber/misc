/**
 * string algo tests
 * @author Tobias Weber
 * @date apr-2021
 * @license see 'LICENSE.EUPL' file
 */

#include "str_algos.h"

#include <iostream>
#include <vector>
#include <string>


int main()
{
	std::cout << find_pattern<std::string>("abcdefghijkl", "abc") << std::endl;
	std::cout << find_pattern<std::string>("abcdefghijkl", "def") << std::endl;
	std::cout << find_pattern<std::string>("abcdefghijkl", "jkl") << std::endl;
	std::cout << find_pattern<std::string>("abcdefghijkl", "defx") << std::endl;
	std::cout << std::endl;

	auto hufftree = huffman<std::string>("abbcccdddd");
	hufftree->print(std::cout);
	std::cout << std::endl;

	auto mapping = huffman_mapping<std::string>(hufftree);
	for(const auto& pair : mapping)
	{
		std::string bits;
		boost::to_string(std::get<1>(pair), bits);
		std::string bits_rev{bits.rbegin(), bits.rend()};
		std::cout << std::get<0>(pair) << ": " << bits_rev << "\n";
	}
	std::cout << std::endl;

	return 0;
}
