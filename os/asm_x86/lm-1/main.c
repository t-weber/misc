/**
 * long mode test
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#include "defines.h"
#include "string.h"


static const u8 attr_bold = 0b00001111;
static const u8 attr_inv = 0b01110000;
static const u8 attr_norm = 0b00000111;


u64 fact(u64 num)
{
	if(num < 1)
		return 1;
	if(num == 2)
		return 2;

	return num * fact(num-1);
}


u64 fibo(u64 num)
{
	if(num < 2)
		return 1;

	return fibo(num-1) + fibo(num-2);
}


void calc(u64 num_start, u64 num_end)
{
	const u64 spacing = 16;

	memset(CHAROUT, 0, SCREEN_SIZE*2);
	write_str("                                 Long Mode Test                                 ", attr_inv, CHAROUT);

	write_str("Number", attr_bold, CHAROUT + (SCREEN_COL_SIZE*2) * 2);
	write_str("Factorial", attr_bold, CHAROUT + (SCREEN_COL_SIZE*2 + spacing) * 2);
	write_str("Fibonacci", attr_bold, CHAROUT + (SCREEN_COL_SIZE*2 + spacing*2) * 2);

	for(u64 num=num_start; num<=num_end; ++num)
	{
		u64 num_fact = fact(num);
		u64 num_fibo = fibo(num);

		char buf_num[16], buf_fact[16], buf_fibo[16];
		int_to_str(num, 10, buf_num);
		int_to_str(num_fact, 10, buf_fact);
		int_to_str(num_fibo, 10, buf_fibo);

		write_str(buf_num, attr_norm, CHAROUT + (SCREEN_COL_SIZE*(num-num_start+3)) * 2);
		write_str(buf_fact, attr_norm, CHAROUT + ((SCREEN_COL_SIZE*(num-num_start+3)) + spacing)*2);
		write_str(buf_fibo, attr_norm, CHAROUT + ((SCREEN_COL_SIZE*(num-num_start+3)) + spacing*2)*2);
	}
}


void keyb_event(u64 ch)
{
	// scan codes
	switch(ch)
	{
		case 0x4f: case 0x02: calc(1, 1); break;	// 1
		case 0x50: case 0x03: calc(2, 2); break;	// 2
		case 0x51: case 0x04: calc(3, 3); break;	// 3
		case 0x4b: case 0x05: calc(4, 4); break;	// 4
		case 0x4c: case 0x06: calc(5, 5); break;	// 5
		case 0x4d: case 0x07: calc(6, 6); break;	// 6
		case 0x47: case 0x08: calc(7, 7); break;	// 7
		case 0x48: case 0x09: calc(8, 8); break;	// 8
		case 0x49: case 0x0a: calc(9, 9); break;	// 9
		case 0x52: case 0x0b: calc(10, 10); break;	// 0
		case 0x39: calc(0, 12); break;	// space
	}

	// write scan code
	char buf_ch[16];
	uint_to_str(ch, 16, buf_ch);
	write_str("Key:", attr_bold, CHAROUT + (SCREEN_COL_SIZE*(SCREEN_ROW_SIZE-1)) * 2);
	write_str(buf_ch, attr_norm, CHAROUT + (SCREEN_COL_SIZE*(SCREEN_ROW_SIZE-1) + 5) * 2);
}


void timer_event()
{
	static u64 rtc = 0;
	static const u64 div_sec = 468;

	static u64 last_val = 0xffffffff;
	u64 val = rtc / div_sec;

	if(val != last_val)
	{
		char buf_val[64];
		uint_to_str(val, 10, buf_val);
		u64 buflen = strlen(buf_val);

		// add decimal point
		buf_val[buflen] = buf_val[buflen-1];
		buf_val[buflen-1] = '.';

		write_str("Uptime:", attr_bold, CHAROUT + (SCREEN_COL_SIZE*(SCREEN_ROW_SIZE-1) + 9) * 2);
		write_str(buf_val, attr_norm, CHAROUT + (SCREEN_COL_SIZE*(SCREEN_ROW_SIZE-1) + 9 + 8) * 2);

		last_val = val;
	}

	++rtc;
}


void rtc_event()
{
}


void entrypoint()
{
	calc(0, 12);
}
