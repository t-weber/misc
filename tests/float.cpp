/**
 * float test
 * @author Tobias Weber
 * @date 19-mar-19
 * @license: see 'LICENSE.EUPL' file
 *
 * @see https://en.wikipedia.org/wiki/IEEE_754
 */

#include <iostream>
#include <string>
#include <sstream>


constexpr unsigned pow2(unsigned n)
{
	if(n == 0)
		return 1;
	else if(n == 1)
		return 2;
	else if(n > 1)
		return 2*pow2(n-1);

	return 0;
}



template<class t_float>
struct float_traits {};

template<>
struct float_traits<float>
{
	using t_int = unsigned;		// large enough int to hold the float's bits

	static constexpr int total_len = sizeof(float)*8;
	static constexpr int exp_len = 8;
	static constexpr int mant_len = total_len - exp_len - 1;
	static constexpr int bias = pow2(exp_len-1)-1;

	static constexpr t_int sign_mask = 1<<(total_len-1);
	static constexpr t_int exp_mask = 0xff << mant_len;
	static constexpr t_int mant_mask = ~(sign_mask | exp_mask);
};


template<>
struct float_traits<double>
{
	using t_int = unsigned long;

	static constexpr int total_len = sizeof(double)*8;
	static constexpr int exp_len = 11;
	static constexpr int mant_len = total_len - exp_len - 1;
	static constexpr int bias = pow2(exp_len-1)-1;

	static constexpr t_int sign_mask = 1ul<<(total_len-1ul);
	static constexpr t_int exp_mask = 0x7fful << mant_len;
	static constexpr t_int mant_mask = ~(sign_mask | exp_mask);
};


template<class t_float, class t_int>
t_float decode_mant(t_int mant)
{
	int len = float_traits<t_float>::mant_len;

	t_float val = t_float(1);

	t_float curpow = 0.5;
	for(int cur=len-1; cur>=0; --cur)
	{
		if(mant & t_int{1}<<cur)
			val += curpow;

		curpow /= t_float(2);
	}

	return val;
}


template<class t_float>
void float_info(t_float f)
{
	std::cout << "total length: " << float_traits<t_float>::total_len << " bits" << std::endl;
	std::cout << "exponent length: " << float_traits<t_float>::exp_len << " bits" << std::endl;
	std::cout << "mantissa length: " << float_traits<t_float>::mant_len << " bits" << std::endl;
	std::cout << "bias: " << float_traits<t_float>::bias << std::endl;
	std::cout << std::endl;

	using t_int = typename float_traits<t_float>::t_int;
	t_int bits = *reinterpret_cast<t_int*>(&f);

	const int bias = float_traits<t_float>::bias;
	const t_int sign_mask = float_traits<t_float>::sign_mask;
	const t_int exp_mask = float_traits<t_float>::exp_mask;
	const t_int mant_mask = float_traits<t_float>::mant_mask;
	std::cout << "mantissa mask: " << std::hex << mant_mask << std::dec << std::endl;

	bool sign = (bits & sign_mask) >> float_traits<t_float>::total_len-1;
	t_int exp = (bits & exp_mask) >> float_traits<t_float>::mant_len;
	t_int mant = (bits & mant_mask);

	unsigned expval = pow2(exp-bias);
	t_float themant = decode_mant<t_float, t_int>(mant);
	std::cout << "sign: " << std::boolalpha << sign << std::endl;
	std::cout << "exponent (biased): " << exp << std::endl;
	std::cout << "exponent (unbiased): " << exp-bias << std::endl;
	std::cout << "2^exponent: " << expval << std::endl;
	std::cout << "raw mantissa: " << mant << std::endl;
	std::cout << "mantissa: " << themant << std::endl;

	t_float val = themant * t_float(expval);
	if(sign)
		val = -val;
	std::cout << "float value: " << val << std::endl;
}


int main(int argc, char **argv)
{
	float_info(123.456f);
	std::cout << "\n" << std::endl;

	float_info(123.456);
	std::cout << "\n" << std::endl;


	float_info(-4561.23f);
	std::cout << "\n" << std::endl;

	float_info(-4561.23);
	std::cout << "\n" << std::endl;

	return 0;
}
