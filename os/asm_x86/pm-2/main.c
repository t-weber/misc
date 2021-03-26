/**
 * protected mode test
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#define u8  unsigned char
#define i8  char
#define u16 unsigned short
#define i16 short
#define u32 unsigned int
#define i32 int

#define SCREEN_ROW_SIZE 25
#define SCREEN_COL_SIZE 80
#define SCREEN_SIZE     (SCREEN_ROW_SIZE * SCREEN_COL_SIZE)

#define CHAROUT ((i8*)0x000b8000) /* see https://jbwyatt.com/253/emu/memory.html */


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


void uint_to_str(u32 num, char* buf)
{
	u32 idx = 0;
	while(1)
	{
		u32 mod = num % 10;
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


void int_to_str(i32 num, char* buf)
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
		i32 mod = num % 10;
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

	for(u32 num=1; num<12; ++num)
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
