/**
 * simple libc string replacement functions
 * @author Tobias Weber
 * @date march-2021
 * @license see 'LICENSE.GPL' file
 */

#ifndef __MY_STRLIB_H__
#define __MY_STRLIB_H__


namespace str {


template<typename t_char = char, typename t_uint = unsigned int>
void reverse_str(t_char* buf, t_uint len) noexcept
{
	for(t_uint i = 0; i < len/2; ++i)
	{
		t_uint j = len - i - 1;

		t_char c = buf[i];
		if(c == 0)
			break;

		buf[i] = buf[j];
		buf[j] = c;
	}
}


template<typename t_char = char, typename t_uint = unsigned int>
t_char digit_to_char(t_uint num, t_uint base = 10) noexcept
{
	t_uint mod = num % base;

	if(mod <= 9)
		return (t_char)mod + '0';
	else
		return (t_char)(mod - 10) + 'a';
}


template<typename t_char = char, typename t_uint = unsigned int>
void uint_to_str(t_uint num, t_char* buf, t_uint base = 10) noexcept
{
	t_uint idx = 0;
	while(true)
	{
		t_uint mod = num % base;
		num /= base;

		if(num == 0 && mod == 0)
		{
			if(idx == 0)
				buf[idx++] = '0';
			break;
		}

		if(mod <= 9)
			buf[idx] = (t_char)mod + '0';
		else
			buf[idx] = (t_char)(mod - 10) + 'a';
		++idx;
	}

	buf[idx] = 0;

	reverse_str(buf, idx);
}


template<typename t_char = char, typename t_int = int, typename t_uint = unsigned int>
void int_to_str(t_int num, t_char* buf, t_uint base = 10) noexcept
{
	t_uint idx = 0;
	t_uint beg = 0;

	if(num < 0)
	{
		buf[idx] = '-';
		num = -num;

		++idx;
		++beg;
	}

	while(true)
	{
		t_int mod = num % base;
		num /= base;

		if(num == 0 && mod == 0)
		{
			if(idx == 0)
				buf[idx++] = '0';

			break;
		}

		if(mod <= 9)
			buf[idx] = (t_char)mod + '0';
		else
			buf[idx] = (t_char)(mod - 10) + 'a';
		++idx;
	}

	buf[idx] = 0;

	reverse_str(buf + beg, idx - beg);
}


template<typename t_char = char, typename t_uint = unsigned int>
t_uint my_strlen(const t_char *str) noexcept
{
	t_uint len = 0;

	while(str[len])
		++len;

	return len;
}


template<typename f64 = double, typename t_char = char,
	typename t_int = int, typename t_uint = unsigned int>
void real_to_str(f64 num, t_char* buf, t_uint base = 10, t_uint decimals = 8) noexcept
{
	const f64 eps = 1e-8;

	// negative number?
	t_uint idx = 0;
	if(num < 0)
	{
		buf[idx++] = '-';
		num = -num;
	}

	// get number before decimal point
	uint_to_str((t_uint)num, buf + idx, base);

	// get number after decimal point
	t_char buf_decimals[64];
	for(t_uint dec = 0; dec < decimals; ++dec)
	{
		// strip away digits before decimal point
		num -= (t_uint)num;

		// get next decimal
		num *= base;
		// for numeric stability
		if(num >= base - eps)
			num = 0;

		t_uint digit = (t_uint)num;
		// for numeric stability
		if(num >= (f64)digit + 1 - eps)
			++digit;

		buf_decimals[dec] = digit_to_char(digit, base);
	}
	buf_decimals[decimals] = 0;

	// strip away trailing '0's
	for(t_int dec=decimals - 1; dec >= 0; --dec)
	{
		if(buf_decimals[dec] == '0')
			buf_decimals[dec] = 0;
		else
			break;
	}

	if(my_strlen<t_char, t_uint>(buf_decimals))
	{
		strncat_char(buf, '.', 64);
		my_strncat(buf, buf_decimals, 64);
	}
}


template<typename t_char = char, typename t_uint = unsigned int>
void my_memset(t_char* mem, t_char val, t_uint size) noexcept
{
	for(t_uint i = 0; i < size; ++i)
		mem[i] = val;
}


template<typename t_char = char, typename t_uint = unsigned int>
void my_memset_interleaved(t_char* mem, t_char val, t_uint size, t_uint interleave) noexcept
{
	for(t_uint i = 0; i < size; i += interleave)
		mem[i] = val;
}


template<typename t_char = char, typename t_uint = unsigned int>
void my_memcpy(t_char* mem_dst, t_char* mem_src, t_uint size) noexcept
{
	for(t_uint i = 0; i < size; ++i)
		mem_dst[i] = mem_src[i];
}


template<typename t_char = char, typename t_uint = unsigned int>
void my_memcpy_interleaved(t_char* mem_dst, t_char* mem_src, t_uint size, t_uint interleave) noexcept
{
	for(t_uint i = 0; i < size; i += interleave)
		mem_dst[i] = mem_src[i];
}


template<typename t_char = char, typename t_uint = unsigned int>
void my_strncpy(t_char* str_dst, const t_char* str_src, t_uint max_len) noexcept
{
	for(t_uint i = 0; i < max_len; ++i)
	{
		t_char c = str_src[i];
		str_dst[i] = c;

		if(c == 0)
			break;
	}
}


template<typename t_char = char, typename t_uint = unsigned int>
void my_strncat(t_char* str_dst, const t_char* str_src, t_uint max_len) noexcept
{
	t_uint len = my_strlen<t_char, t_uint>(str_dst);
	my_strncpy(str_dst + len, str_src, max_len - len);
}


template<typename t_char = char, typename t_uint = unsigned int>
void strncat_char(t_char* str, t_char c, t_uint max_len) noexcept
{
	t_uint len = my_strlen<t_char, t_uint>(str);
	if(len + 1 < max_len)
	{
		str[len] = c;
		str[len + 1] = 0;
	}
}


template<typename t_char = char, typename t_uint = unsigned int>
t_char my_strncmp(const t_char* str1, const t_char* str2, t_uint max_len) noexcept
{
	for(t_uint i = 0; i < max_len; ++i)
	{
		t_char c1 = str1[i];
		t_char c2 = str2[i];

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


template<typename t_char = char, typename t_uint = unsigned int>
t_char my_strcmp(const t_char* str1, const t_char* str2) noexcept
{
	t_uint len1 = my_strlen<t_char, t_uint>(str1);
	t_uint len2 = my_strlen<t_char, t_uint>(str2);

	return my_strncmp(str1, str2, my_max(len1, len2));
}


template<typename t_int = int>
t_int my_max(t_int a, t_int b) noexcept
{
	if(b > a)
		return b;
	return a;
}


template<typename t_char = char>
bool my_isupperalpha(t_char c) noexcept
{
	return (c >= 'A' && c <= 'Z');
}


template<typename t_char = char>
bool my_isloweralpha(t_char c) noexcept
{
	return (c >= 'a' && c <= 'z');
}


template<typename t_char = char>
bool my_isalpha(t_char c) noexcept
{
	return my_isupperalpha(c) || my_isloweralpha(c);
}


template<typename t_char = char>
bool my_isdigit(t_char c, bool hex = false) noexcept
{
	t_char is_num = (c >= '0' && c <= '9');
	if(hex)
	{
		is_num = is_num &&
			((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
	}

	return is_num;
}


template<typename t_char = char, typename t_int = int, typename t_uint = unsigned int>
t_int my_atoi(const t_char* str, t_int base = 10) noexcept
{
	t_uint len = my_strlen<t_char, t_uint>(str);
	t_int num = 0;

	for(t_uint i = 0; i < len; ++i)
	{
		t_int digit = 0;
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


template<typename f64 = double, typename t_char = char, typename t_uint = unsigned int>
f64 my_atof(const t_char* str, t_uint base = 10) noexcept
{
	t_uint len = my_strlen<t_char, t_uint>(str);
	f64 num = 0, decimal = 0;
	t_char in_num = 1;
	t_uint denom_pos = 1;

	for(t_uint i = 0; i < len; ++i)
	{
		if(str[i] == '.')
		{
			in_num = 0;
			continue;
		}

		f64 digit = 0;
		if(my_isdigit(str[i], 0))
			digit = str[i] - '0';
		else if(my_isupperalpha(str[i]))
			digit = (str[i] - 'A') + 10;
		else if(my_isloweralpha(str[i]))
			digit = (str[i] - 'a') + 10;

		// before decimal point
		if(in_num)
		{
			num = num*((f64)base) + digit;
		}

		// after decimal point
		else
		{
			for(t_uint j=0; j<denom_pos; ++j)
				digit /= (f64)base;
			decimal += digit;

			++denom_pos;
		}
	}

	return num + decimal;
}


template<typename t_char = char, typename t_uint = unsigned int>
void write_char(t_char ch, t_uint attrib, t_char *addr) noexcept
{
	*addr++ = ch;
	*addr++ = attrib;
}


template<typename t_char = char, typename t_uint = unsigned int>
void write_str(t_char *str, t_uint attrib, t_char *addr) noexcept
{
	t_uint len = my_strlen<t_char, t_uint>(str);

	for(t_uint i = 0; i < len; ++i)
	{
		*addr++ = str[i];
		*addr++ = attrib;
	}
}


template<typename t_char = char, typename t_uint = unsigned int>
void read_str(t_char *str, const t_char *addr, t_uint len) noexcept
{
	for(t_uint i = 0; i < len; ++i)
	{
		str[i] = *addr;
		addr += 2;
	}
}


template<typename t_char = char, typename t_uint = unsigned int>
void clear_scr(t_uint attrib, t_char *addr, t_uint size) noexcept
{
	for(t_uint i = 0; i < size; ++i)
	{
		*addr++ = 0;
		*addr++ = attrib;
	}
}


/**
 * print a byte size
 */
template<typename t_char = char, typename t_uint = unsigned int>
void write_size(t_uint size, t_char* str, t_uint max_len) noexcept
{
	const t_uint sizes[4] = { 1024*1024*1024, 1024*1024, 1024, 1 };
	const t_char* size_names[4] = { " GB ", " MB ", " kB ", " B" };

	for(t_uint i = 0; i < sizeof(sizes)/sizeof(*sizes); ++i)
	{
		t_uint sz = size / sizes[i];
		size %= sizes[i];

		if(!sz)
			continue;

		uint_to_str(sz, str, 10);
		my_strncat(str, size_names[i], max_len);
		t_uint len = my_strlen<t_char, t_uint>(str);
		str += len;
		max_len -= len;
	}
}


}  // namespace str

#endif
