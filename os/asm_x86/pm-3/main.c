/**
 * protected mode test
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#include "defines.h"
#include "string.h"


u32 fact(u32 num)
{
	if(num < 1)
		return 1;
	if(num == 2)
		return 2;

	return num * fact(num-1);
}


u32 fibo(u32 num)
{
	if(num < 2)
		return 1;

	return fibo(num-1) + fibo(num-2);
}


void entrypoint()
{
	const u8 attr_bold = 0b00001111;
	const u8 attr_inv = 0b01110000;
	const u8 attr_norm = 0b00000111;
	const u32 spacing = 16;

	memset(CHAROUT, 0, SCREEN_SIZE*2);
	write_str("                              Protected Mode Test                               ", attr_inv, CHAROUT);

	write_str("Number", attr_bold, CHAROUT + (SCREEN_COL_SIZE*2) * 2);
	write_str("Factorial", attr_bold, CHAROUT + (SCREEN_COL_SIZE*2 + spacing) * 2);
	write_str("Fibonacci", attr_bold, CHAROUT + (SCREEN_COL_SIZE*2 + spacing*2) * 2);

	for(u32 num=1; num<=12; ++num)
	{
		u32 num_fact = fact(num);
		u32 num_fibo = fibo(num);

		char buf_num[16], buf_fact[16], buf_fibo[16];
		int_to_str(num, buf_num);
		int_to_str(num_fact, buf_fact);
		int_to_str(num_fibo, buf_fibo);

		write_str(buf_num, attr_norm, CHAROUT + (SCREEN_COL_SIZE*(num+2)) * 2);
		write_str(buf_fact, attr_norm, CHAROUT + ((SCREEN_COL_SIZE*(num+2)) + spacing)*2);
		write_str(buf_fibo, attr_norm, CHAROUT + ((SCREEN_COL_SIZE*(num+2)) + spacing*2)*2);
	}
}
