/**
 * basic functions
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 *
 * gcc -m32 -O2 -S -o basic_funcs.asm basic_funcs.c
 */


#include <stdio.h>
#include <string.h>


void reverse_str(char* buf, unsigned int len)
{
	for(unsigned int i=0; i<len/2; ++i)
	{
		unsigned int j = len-i-1;
		//if(j <= i) break;

		char c = buf[i];
		buf[i] = buf[j];
		buf[j] = c;
	}
}


void uint_to_str(unsigned int num, char* buf)
{
	unsigned int idx = 0;
	while(1)
	{
		unsigned int mod = num % 10;
		num /= 10;

		if(num==0 && mod==0)
			break;

		buf[idx] = (char)mod + 0x30;
		++idx;
	}

	buf[idx] = 0;
	//printf("len = %d\n", idx);

	reverse_str(buf, idx);
}


void int_to_str(int num, char* buf)
{
	unsigned int idx = 0;
	unsigned int beg = 0;

	if(num < 0)
	{
		buf[idx] = '-';
		num = -num;

		++idx;
		++beg;
	}

	while(1)
	{
		int mod = num % 10;
		num /= 10;

		if(num==0 && mod==0)
			break;

		buf[idx] = (char)mod + 0x30;
		++idx;
	}

	buf[idx] = 0;
	//printf("len = %d\n", idx);

	reverse_str(buf+beg, idx-beg);
}


int main()
{
	char buf[16];

	uint_to_str(45678, buf);
	printf("buf = %s\n", buf);

	uint_to_str(1, buf);
	printf("buf = %s\n", buf);

	uint_to_str(98, buf);
	printf("buf = %s\n", buf);

	int_to_str(-45678, buf);
	printf("buf = %s\n", buf);

	return 0;
}
