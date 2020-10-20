/**
 * C++ tests
 * @author Tobias Weber
 * @date oct-20
 * @license: see 'LICENSE.EUPL' file
 */

#include <vector>
#include <iostream>


int main()
{
	std::vector<int>v {{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }};

	auto iter1 = v.rbegin();
	auto iter2 = iter1 + 5;

	std::cout << *iter1 << " " << *iter2 << std::endl;

	//v.erase(iter2.base(), std::next(iter1, 1).base());
	v.erase(iter2.base(), iter1.base());
	for(int i : v)
		std::cout << i << ", ";
	std::cout << std::endl;

	return 0;
}
