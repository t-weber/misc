/**
 * float test
 * @author Tobias Weber
 * @date 19-mar-19
 * @license: see 'LICENSE.EUPL' file
 *
 * @see https://en.wikipedia.org/wiki/IEEE_754
 */

#include <bitset>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdint>


template<typename T = unsigned>
constexpr T pow2(T n)
{
	if(n == 0)
		return 1;
	else if(n == 1)
		return 2;
	else if(n > 1)
		return 2 * pow2(n-1);
	else if(n < 0)
		return 0.5 * pow2(n+1);

	return 0;
}


template<class t_float>
struct float_traits {};

template<>
struct float_traits<float>
{
	// large enough ints to hold the float's bits
	using t_uint = std::uint64_t;
	using t_int = std::int64_t;

	static constexpr t_int total_len = sizeof(float)*8;
	static constexpr t_int exp_len = 8;
	static constexpr t_int mant_len = total_len - exp_len - 1;
	static constexpr t_int bias = pow2(exp_len - 1) - 1;

	static constexpr t_uint sign_mask = 1 << (total_len-1);
	static constexpr t_uint exp_mask = 0xff << mant_len;
	static constexpr t_uint mant_mask = ~(sign_mask | exp_mask);
};


template<>
struct float_traits<double>
{
	// large enough ints to hold the double's bits
	// TODO: need to use class with more bits for arithmetic functions
	using t_uint = std::uint64_t;
	using t_int = std::int64_t;

	static constexpr t_int total_len = sizeof(double)*8;
	static constexpr t_int exp_len = 11;
	static constexpr t_int mant_len = total_len - exp_len - 1;
	static constexpr t_int bias = pow2(exp_len - 1) - 1;

	static constexpr t_uint sign_mask = 1ul << (total_len-1ul);
	static constexpr t_uint exp_mask = 0x7fful << mant_len;
	static constexpr t_uint mant_mask = ~(sign_mask | exp_mask);
};


template<class t_float>
struct float_field
{
	using t_uint = typename float_traits<t_float>::t_uint;
	using t_int = typename float_traits<t_float>::t_int;

	t_uint mant : float_traits<t_float>::mant_len;
	t_uint exp : float_traits<t_float>::exp_len;
	bool sign : 1;
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
	using t_uint = typename float_traits<t_float>::t_uint;
	using t_int = typename float_traits<t_float>::t_int;

	constexpr t_int total_len = float_traits<t_float>::total_len;
	constexpr t_int exp_len = float_traits<t_float>::exp_len;
	constexpr t_int mant_len = float_traits<t_float>::mant_len;
	constexpr t_int bias = float_traits<t_float>::bias;

	constexpr t_uint sign_mask = float_traits<t_float>::sign_mask;
	constexpr t_uint exp_mask = float_traits<t_float>::exp_mask;
	constexpr t_uint mant_mask = float_traits<t_float>::mant_mask;

	t_uint bits = *reinterpret_cast<t_uint*>(&f);
	auto field = *reinterpret_cast<float_field<t_float>*>(&f);

	std::cout << "float value: " << f << std::endl;
	std::cout << "float value: 0x" << std::hex << bits << std::dec << std::endl;
	std::cout << "float value: 0b" << std::bitset<total_len>(bits) << std::endl;
	std::cout << "\n";

	std::cout << "total length: " << total_len << " bits" << std::endl;
	std::cout << "exponent length: " << exp_len << " bits" << std::endl;
	std::cout << "mantissa length: " << mant_len << " bits" << std::endl;
	std::cout << "bias: " << bias << std::endl;
	std::cout << "\n";

	bool sign = (bits & sign_mask) >> (total_len - 1);
	t_int exp = (bits & exp_mask) >> mant_len;
	t_uint mant = (bits & mant_mask);

	std::cout << "mant mask: 0x" << std::hex << mant_mask << std::dec << std::endl;
	std::cout << "exp mask:  0x" << std::hex << exp_mask << std::dec << std::endl;
	std::cout << "sign mask: 0x" << std::hex << sign_mask << std::dec << std::endl;
	std::cout << "mant mask: 0b" << std::bitset<total_len>(mant_mask) << std::endl;
	std::cout << "exp mask:  0b" << std::bitset<total_len>(exp_mask) << std::endl;
	std::cout << "sign mask: 0b" << std::bitset<total_len>(sign_mask) << std::endl;
	std::cout << "\n";

	std::cout << "sign: " << std::boolalpha << sign << std::endl;
	std::cout << "\n";

	t_float expval = pow2<t_float>(exp - bias);
	t_float themant = decode_mant<t_float, t_uint>(mant);

	std::cout << "biased exponent: " << exp << std::endl;
	std::cout << "biased exponent: 0x" << std::hex << field.exp << std::dec << std::endl;
	std::cout << "biased exponent: 0b" << std::bitset<exp_len>(field.exp) << std::endl;
	std::cout << "biased exponent (from bitfield): " << field.exp << std::endl;
	std::cout << "unbiased exponent: " << exp - bias << std::endl;
	std::cout << "2^exponent: " << expval << std::endl;
	std::cout << "\n";

	std::cout << "raw mantissa (from bitfield): " << field.mant << std::endl;
	std::cout << "raw mantissa: " << mant << std::endl;
	std::cout << "raw mantissa: 0x" << std::hex << field.mant << std::dec << std::endl;
	std::cout << "raw mantissa: 0b" << std::bitset<mant_len>(field.mant) << std::endl;
	std::cout << "mantissa: " << themant << std::endl;
	std::cout << "\n";

	t_float val = themant * expval;
	if(sign)
		val = -val;
	std::cout << "recalculated float value: " << val << std::endl;
}


template<class t_float>
t_float float_mult(t_float a, t_float b)
{
	using t_uint = typename float_traits<t_float>::t_uint;
	using t_int = typename float_traits<t_float>::t_int;

	constexpr t_int bias = float_traits<t_float>::bias;
	constexpr t_int mant_len = float_traits<t_float>::mant_len;
	constexpr t_uint mant_mask = float_traits<t_float>::mant_mask;

	auto field_a = *reinterpret_cast<float_field<t_float>*>(&a);
	auto field_b = *reinterpret_cast<float_field<t_float>*>(&b);

	std::size_t mant_a = field_a.mant | 1<<mant_len;
	std::size_t mant_b = field_b.mant | 1<<mant_len;
	std::size_t mant_c = (mant_a * mant_b) >> mant_len;

	t_int exp_c = (field_a.exp - bias) + (field_b.exp - bias);

	// normalise
	while(mant_c > (mant_mask | 1<<mant_len))
	{
		mant_c >>= 1;
		exp_c += 1;
	}
	while(mant_c && (mant_c & 1<<mant_len) == 0)
	{
		mant_c <<= 1;
		exp_c -= 1;
	}

	float_field<t_float> field_c
	{
		.mant = static_cast<t_uint>(mant_c & mant_mask),
		.exp = static_cast<t_uint>(exp_c + bias),
		.sign = static_cast<bool>(field_a.sign ^ field_b.sign)
	};

	return *reinterpret_cast<t_float*>(&field_c);
}


template<class t_float>
t_float float_div(t_float a, t_float b)
{
	using t_uint = typename float_traits<t_float>::t_uint;
	using t_int = typename float_traits<t_float>::t_int;

	constexpr t_int bias = float_traits<t_float>::bias;
	constexpr t_int mant_len = float_traits<t_float>::mant_len;
	constexpr t_uint mant_mask = float_traits<t_float>::mant_mask;

	auto field_a = *reinterpret_cast<float_field<t_float>*>(&a);
	auto field_b = *reinterpret_cast<float_field<t_float>*>(&b);

	t_int exp_a = field_a.exp - bias;
	t_int exp_b = field_b.exp - bias;

	std::size_t mant_a = field_a.mant | 1<<mant_len;
	std::size_t mant_b = field_b.mant | 1<<mant_len;

	// shift the dividend to not lose significant digits
	t_int shift_len = mant_len / 2;  // TODO: check
	if(exp_a - exp_b < shift_len)
	{
		//shift_len -= exp_b - exp_a;
		mant_a <<= shift_len;
		exp_a -= shift_len;
	}
	else if(exp_b - exp_a < shift_len)
	{
		//shift_len -= exp_a - exp_b;
		mant_b <<= shift_len;
		exp_b -= shift_len;
	}

	std::size_t mant_c = (mant_a / mant_b) << mant_len;
	t_int exp_c = exp_a - exp_b;

	// normalise
	while(mant_c > (mant_mask | 1<<mant_len))
	{
		mant_c >>= 1;
		exp_c += 1;
	}
	while(mant_c && (mant_c & 1<<mant_len) == 0)
	{
		mant_c <<= 1;
		exp_c -= 1;
	}

	float_field<t_float> field_c
	{
		.mant = static_cast<t_uint>(mant_c & mant_mask),
		.exp = static_cast<t_uint>(exp_c + bias),
		.sign = static_cast<bool>(field_a.sign ^ field_b.sign)
	};

	return *reinterpret_cast<t_float*>(&field_c);
}


template<class t_float>
t_float float_add(t_float a, t_float b)
{
	using t_uint = typename float_traits<t_float>::t_uint;
	using t_int = typename float_traits<t_float>::t_int;

	constexpr t_int bias = float_traits<t_float>::bias;
	constexpr t_int mant_len = float_traits<t_float>::mant_len;
	constexpr t_uint mant_mask = float_traits<t_float>::mant_mask;

	auto field_a = *reinterpret_cast<float_field<t_float>*>(&a);
	auto field_b = *reinterpret_cast<float_field<t_float>*>(&b);

	t_int exp_a = field_a.exp - bias;
	t_int exp_b = field_b.exp - bias;

	t_int mant_a = field_a.mant | 1<<mant_len;
	t_int mant_b = field_b.mant | 1<<mant_len;

	// find a common exponent
	if(exp_a > exp_b)
	{
		mant_b >>= exp_a - exp_b;
		exp_b += exp_a - exp_b;
	}
	else if(exp_b > exp_a)
	{
		mant_a >>= exp_b - exp_a;
		exp_a += exp_b - exp_a;
	}

	// set signs
	if(field_a.sign)
		mant_a = -mant_a;
	if(field_b.sign)
		mant_b = -mant_b;

	// add mantissas
	t_uint exp_c = exp_a;
	t_int mant_c = mant_a + mant_b;

	// get sign
	bool sign_c = false;
	if(mant_c < 0)
	{
		sign_c = true;
		mant_c = -mant_c;
	}

	// normalise
	while(static_cast<std::size_t>(mant_c) > (mant_mask | 1<<mant_len))
	{
		mant_c >>= 1;
		exp_c += 1;
	}
	while(mant_c && (mant_c & 1<<mant_len) == 0)
	{
		mant_c <<= 1;
		exp_c -= 1;
	}

	float_field<t_float> field_c
	{
		.mant = static_cast<t_uint>(mant_c & mant_mask),
		.exp = exp_c + bias,
		.sign = static_cast<bool>(sign_c)
	};

	return *reinterpret_cast<t_float*>(&field_c);
}


int main()
{
	float_info(123.456f); std::cout << "\n\n\n";
	float_info(123.456); std::cout << "\n\n\n";
	float_info(-4561.23f); std::cout << "\n\n\n";
	float_info(-4561.23); std::cout << "\n\n\n";

	float_info(0.1f); std::cout << "\n\n\n";
	float_info(0.f); std::cout << "\n\n\n";
	float_info(1.f); std::cout << "\n\n\n";
	float_info(2.f); std::cout << "\n\n\n";
	float_info(3.f); std::cout << "\n\n\n";

	float_info(float_mult(123.f, -234.f)); std::cout << "\n\n\n";
	float_info(float_mult(0.5f, 12.8e2f)); std::cout << "\n\n\n";

	float_info(float_div(100.f, 5.f)); std::cout << "\n\n\n";
	float_info(float_div(1.f, 10.f)); std::cout << "\n\n\n";

	float_info(float_add(-100.5f, -0.5f)); std::cout << "\n\n\n";

	return 0;
}
