/**
 * @author Tobias Weber
 * @date jan-2021
 * @license: see 'LICENSE.EUPL' file
 */

#include <iostream>
#include <sstream>
#include <cmath>


using t_int = int;
using t_real = double;


int main(int argc, char** argv)
{
	if(argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " <number> <divisor> [shift_left]" << std::endl;
		return -1;
	}


	t_int num{}, div{}, shift{};
	std::istringstream{argv[1]} >> num;
	std::istringstream{argv[2]} >> div;

	if(argc >= 4)
	{
		// use given shift
		std::istringstream{argv[3]} >> shift;
	}
	else
	{
		// get minimum amount of bits to be shifted
		shift = t_int(std::ceil(std::log2(t_real(div))));
	}


	// exact result
	std::cout.precision(8);
	std::cout << num << " / " << div << " = " << t_real(num)/t_real(div) << std::endl;


	// see: https://surf-vhdl.com/how-to-divide-an-integer-by-constant-in-vhdl
	t_int shiftdiv = std::pow(2, shift);
	t_int mult = shiftdiv / div;
	t_int prevpow = std::pow(2, shift-1);
	t_int res = (num * mult) >> shift;
	t_int res2 = (num * mult + prevpow) >> shift;

	std::cout << "(" << num << " * " << shiftdiv << " / " << div << ") >> "
		<< shift << " = " << res << std::endl;
	std::cout << "(" << num << " * " << shiftdiv << " / " << div << " + " << prevpow << ") >> "
		<< shift << " = " << res2 << std::endl;


	return 0;
}
