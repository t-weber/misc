/**
 * C++20 compatibility tests
 * @author Tobias Weber
 * @date apr-20
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++2a -o cpp20 cpp20.cpp
 */

#include <iostream>
#include <compare>


int main()
{
	// --------------------------------------------------------------------
	// operator <=>
	{
		struct A
		{
			int x;

			// TODO
			std::strong_ordering operator <=>(const A& a) const
			{
				return x <=> a.x;
			}
		};

		A a{.x = 1}, b{.x = 2};

		auto cmp = (a <=> b);
	}

	// --------------------------------------------------------------------


	return 0;
}
