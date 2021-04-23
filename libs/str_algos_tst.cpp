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

	return 0;
}
