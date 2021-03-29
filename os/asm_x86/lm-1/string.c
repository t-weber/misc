/**
 * long mode test
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#include "defines.h"


void reverse_str(char* buf, u64 len)
{
	for(u64 i=0; i<len/2; ++i)
	{
		u64 j = len-i-1;
		//if(j <= i) break;

		char c = buf[i];
		buf[i] = buf[j];
		buf[j] = c;
	}
}


void uint_to_str(u64 num, u64 base, char* buf)
{
	u64 idx = 0;
	while(1)
	{
		u64 mod = num % base;
		num /= base;

		if(num==0 && mod==0)
		{
			if(idx == 0)
				buf[idx++] = '0';
			break;
		}

		if(mod <= 9)
			buf[idx] = (char)mod + '0';
		else
			buf[idx] = (char)(mod-10) + 'a';
		++idx;
	}

	buf[idx] = 0;
	//printf("len = %d\n", idx);

	reverse_str(buf, idx);
}


void int_to_str(i64 num, u64 base, char* buf)
{
	u64 idx = 0;
	u64 beg = 0;

	if(num < 0)
	{
		buf[idx] = '-';
		num = -num;

		++idx;
		++beg;
	}

	while(1)
	{
		i64 mod = num % base;
		num /= base;

		if(num==0 && mod==0)
		{
			if(idx == 0)
				buf[idx++] = '0';

			break;
		}

		if(mod <= 9)
			buf[idx] = (char)mod + '0';
		else
			buf[idx] = (char)(mod-10) + 'a';
		++idx;
	}

	buf[idx] = 0;
	//printf("len = %d\n", idx);

	reverse_str(buf+beg, idx-beg);
}


u64 strlen(char *str)
{
	u64 len = 0;

	while(str[len])
		++len;

	return len;
}


void memset(i8* mem, i8 val, u64 size)
{
	for(u64 i=0; i<size; ++i)
		mem[i] = val;
}


void write_str(char *str, u8 attrib, i8 *addr)
{
	u64 len = strlen(str);

	for(u64 i=0; i<len; ++i)
	{
		*addr++ = str[i];
		*addr++ = attrib;
	}
}
