/**
 * @author Tobias Weber
 * @date apr-20
 * @license: see 'LICENSE.EUPL' file
 *
 * @see https://de.wikipedia.org/wiki/Ackermannfunktion
 */

#include <iostream>
#include <iomanip>


template<class t_int=unsigned long>
t_int ack(t_int n, t_int m)
{
	if(n == 0)
		return m + 1;
	else if(n >= 1 && m == 0)
		return ack<t_int>(n-1, 1);

	return ack<t_int>(n-1, ack(n, m-1));
}


template<class t_int=unsigned long>
t_int ack_mod(t_int k, t_int n)
{
	if(k == 1 && n >= 1)
		return 2*n;
	else if(k >= 2 && n == 1)
		return 2;

	return ack_mod<t_int>(k-1, ack_mod(k, n-1));
}


int main(int argc, char **argv)
{
	for(unsigned long n=1; n<4; ++n)
	{
		std::cout
			<< std::setw(5) << n << " "
			<< std::setw(5) << std::right << ack(n,n) << " "
			<< std::setw(5) << std::right << ack_mod(n,n) << std::endl;
	}

	return 0;
}
