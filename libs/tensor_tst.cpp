/**
 * tensor class test
 * @author Tobias Weber
 * @date November 2021
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o tensor_tst tensor_tst.cpp
 */

#include "tensor.h"

#include <iostream>


using t_real = double;


int main()
{
	Tensor<t_real, 2,3> t1{}, t2{};
	std::cout << "dims: " << t1.size<0>() << " " << t1.size<1>() << std::endl;
	std::cout << "total size: " << t1.size() << std::endl;

	t1(0,0) = 9;
	t1(0,1) = 8;
	t1(1,1) = 5;

	t2(0, 1) = -1;

	t1 += 2*t2;

	std::cout << "elements: ";
	for(std::size_t i=0; i<t1.size(); ++i)
		std::cout << t1[i] << " ";
	std::cout << std::endl;

	return 0;
}
