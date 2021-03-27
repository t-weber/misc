/**
 * protected mode test
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#include "defines.h"


void reverse_str(char* buf, u32 len)
{
	for(u32 i=0; i<len/2; ++i)
	{
		u32 j = len-i-1;
		//if(j <= i) break;

		char c = buf[i];
		buf[i] = buf[j];
		buf[j] = c;
	}
}


void uint_to_str(u32 num, u32 base, char* buf)
{
	u32 idx = 0;
	while(1)
	{
		u32 mod = num % base;
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


void int_to_str(i32 num, u32 base, char* buf)
{
	u32 idx = 0;
	u32 beg = 0;

	if(num < 0)
	{
		buf[idx] = '-';
		num = -num;

		++idx;
		++beg;
	}

	while(1)
	{
		i32 mod = num % base;
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


u32 strlen(char *str)
{
	u32 len = 0;

	while(str[len])
		++len;

	return len;
}


void memset(i8* mem, i8 val, u32 size)
{
	for(u32 i=0; i<size; ++i)
		mem[i] = val;
}


void write_str(char *str, u8 attrib, i8 *addr)
{
	u32 len = strlen(str);

	for(u32 i=0; i<len; ++i)
	{
		*addr++ = str[i];
		*addr++ = attrib;
	}
}
