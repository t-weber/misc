/**
 * simple libc string replacement functions
 * @author Tobias Weber
 * @date mar-21
 * @license 'LICENSE.GPL'
 */

#include "string.h"


void reverse_str(int8_t* buf, uint32_t len)
{
	for(uint32_t i=0; i<len/2; ++i)
	{
		uint32_t j = len-i-1;

		int8_t c = buf[i];
		buf[i] = buf[j];
		buf[j] = c;
	}
}


int8_t digit_to_char(uint8_t num, uint32_t base)
{
	uint32_t mod = num % base;

	if(mod <= 9)
		return (int8_t)mod + '0';
	else
		return (int8_t)(mod-10) + 'a';
}


void uint_to_str(uint32_t num, uint32_t base, int8_t* buf)
{
	uint32_t idx = 0;
	while(1)
	{
		uint32_t mod = num % base;
		num /= base;

		if(num==0 && mod==0)
		{
			if(idx == 0)
				buf[idx++] = '0';
			break;
		}

		if(mod <= 9)
			buf[idx] = (int8_t)mod + '0';
		else
			buf[idx] = (int8_t)(mod-10) + 'a';
		++idx;
	}

	buf[idx] = 0;

	reverse_str(buf, idx);
}


void int_to_str(int32_t num, uint32_t base, int8_t* buf)
{
	uint32_t idx = 0;
	uint32_t beg = 0;

	if(num < 0)
	{
		buf[idx] = '-';
		num = -num;

		++idx;
		++beg;
	}

	while(1)
	{
		int32_t mod = num % base;
		num /= base;

		if(num==0 && mod==0)
		{
			if(idx == 0)
				buf[idx++] = '0';

			break;
		}

		if(mod <= 9)
			buf[idx] = (int8_t)mod + '0';
		else
			buf[idx] = (int8_t)(mod-10) + 'a';
		++idx;
	}

	buf[idx] = 0;

	reverse_str(buf+beg, idx-beg);
}


void real_to_str(float num, uint32_t base, int8_t* buf, uint8_t decimals)
{
	const float eps = 1e-8;

	// negative number?
	uint32_t idx = 0;
	if(num < 0)
	{
		buf[idx++] = '-';
		num = -num;
	}

	// get number before decimal point
	uint_to_str((uint32_t)num, base, buf+idx);

	// get number after decimal point
	int8_t buf_decimals[64];
	for(uint8_t dec=0; dec<decimals; ++dec)
	{
		// strip away digits before decimal point
		num -= (uint32_t)num;

		// get next decimal
		num *= base;
		// for numeric stability
		if(num >= base - eps)
			num = 0;

		uint8_t digit = (uint8_t)num;
		// for numeric stability
		if(num >= (float)digit + 1 - eps)
			++digit;

		buf_decimals[dec] = digit_to_char(digit, base);
	}
	buf_decimals[decimals] = 0;

	// strip away trailing '0's
	for(int16_t dec=decimals-1; dec>=0; --dec)
	{
		if(buf_decimals[dec] == '0')
			buf_decimals[dec] = 0;
		else
			break;
	}

	if(my_strlen(buf_decimals))
	{
		strncat_char(buf, '.', 64);
		my_strncat(buf, buf_decimals, 64);
	}
}


uint32_t my_strlen(const int8_t *str)
{
	uint32_t len = 0;

	while(str[len])
		++len;

	return len;
}


void my_memset(int8_t* mem, int8_t val, uint32_t size)
{
	for(uint32_t i=0; i<size; ++i)
		mem[i] = val;
}


void my_memset_interleaved(int8_t* mem, int8_t val, uint32_t size, uint8_t interleave)
{
	for(uint32_t i=0; i<size; i+=interleave)
		mem[i] = val;
}


void my_memcpy(int8_t* mem_dst, int8_t* mem_src, uint32_t size)
{
	for(uint32_t i=0; i<size; ++i)
		mem_dst[i] = mem_src[i];
}


void my_memcpy_interleaved(int8_t* mem_dst, int8_t* mem_src, uint32_t size, uint8_t interleave)
{
	for(uint32_t i=0; i<size; i+=interleave)
		mem_dst[i] = mem_src[i];
}


void my_strncpy(int8_t* str_dst, const int8_t* str_src, uint32_t max_len)
{
	for(uint32_t i=0; i<max_len; ++i)
	{
		int8_t c = str_src[i];
		str_dst[i] = c;

		if(c == 0)
			break;
	}
}


void my_strncat(int8_t* str_dst, const int8_t* str_src, uint32_t max_len)
{
	uint32_t len = my_strlen(str_dst);
	my_strncpy(str_dst + len, str_src, max_len - len);
}


void strncat_char(int8_t* str, int8_t c, uint32_t max_len)
{
	uint32_t len = my_strlen(str);
	if(len+1 < max_len)
	{
		str[len] = c;
		str[len+1] = 0;
	}
}


int8_t my_strncmp(const int8_t* str1, const int8_t* str2, uint32_t max_len)
{
	for(uint32_t i=0; i<max_len; ++i)
	{
		int8_t c1 = str1[i];
		int8_t c2 = str2[i];

		if(c1 == c2 && c1 != 0)
			continue;
		else if(c1 < c2)
			return -1;
		else if(c1 > c2)
			return 1;
		else if(c1 == 0 && c2 == 0)
			return 0;
	}

	return 0;
}


int8_t my_strcmp(const int8_t* str1, const int8_t* str2)
{
	uint32_t len1 = my_strlen(str1);
	uint32_t len2 = my_strlen(str2);

	return my_strncmp(str1, str2, my_max(len1, len2));
}


int32_t my_max(int32_t a, int32_t b)
{
	if(b > a)
		return b;
	return a;
}


int8_t my_isupperalpha(int8_t c)
{
	return (c>='A' && c<='Z');
}


int8_t my_isloweralpha(int8_t c)
{
	return (c>='a' && c<='z');
}


int8_t my_isalpha(int8_t c)
{
	return my_isupperalpha(c) || my_isloweralpha(c);
}


int8_t my_isdigit(int8_t c, int8_t hex)
{
	int8_t is_num = (c>='0' && c<='9');
	if(hex)
	{
		is_num = is_num &&
			((c>='a' && c<='f') || (c>='A' && c<='F'));
	}

	return is_num;
}


int32_t my_atoi(const int8_t* str, int32_t base)
{
	uint32_t len = my_strlen(str);
	int32_t num = 0;

	for(uint32_t i=0; i<len; ++i)
	{
		int32_t digit = 0;
		if(my_isdigit(str[i], 0))
			digit = str[i] - '0';
		else if(my_isupperalpha(str[i]))
			digit = (str[i] - 'A') + 10;
		else if(my_isloweralpha(str[i]))
			digit = (str[i] - 'a') + 10;

		num = num*base + digit;
	}

	return num;
}


float my_atof(const int8_t* str, int32_t base)
{
	uint32_t len = my_strlen(str);
	float num = 0, decimal = 0;
	int8_t in_num = 1;
	int32_t denom_pos = 1;

	for(uint32_t i=0; i<len; ++i)
	{
		if(str[i] == '.')
		{
			in_num = 0;
			continue;
		}

		float digit = 0;
		if(my_isdigit(str[i], 0))
			digit = str[i] - '0';
		else if(my_isupperalpha(str[i]))
			digit = (str[i] - 'A') + 10;
		else if(my_isloweralpha(str[i]))
			digit = (str[i] - 'a') + 10;

		// before decimal point
		if(in_num)
		{
			num = num*((float)base) + digit;
		}

		// after decimal point
		else
		{
			for(int32_t j=0; j<denom_pos; ++j)
				digit /= (float)base;
			decimal += digit;

			++denom_pos;
		}
	}

	return num + decimal;
}


// test
/*#include <stdio.h>
int main()
{
	int8_t buf[64];
	real_to_str(-987.01020300, 10, buf, 10);
	puts(buf);
}*/
