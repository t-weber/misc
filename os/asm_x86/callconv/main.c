/**
 * calling convention test
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#include <stdio.h>

typedef unsigned int t_int;


extern t_int __attribute__((__cdecl__)) fact_cdecl_asm(t_int num, t_int pure_call);
extern t_int __attribute__((__stdcall__)) fact_stdcall_asm(t_int num, t_int pure_call);
extern t_int __attribute__((__fastcall__)) fact_fastcall_asm(t_int num, t_int pure_call);


t_int __attribute__((__cdecl__)) fact_cdecl_c(t_int num, t_int pure_call)
{
	if(num < 1)
		return 1;
	if(num == 2)
		return 2;

	// call c or asm version of this function
	if(pure_call)
		return num * fact_cdecl_c(num-1, pure_call);
	else
		return num * fact_cdecl_asm(num-1, pure_call);
}


t_int __attribute__((__stdcall__)) fact_stdcall_c(t_int num, t_int pure_call)
{
	if(num < 1)
		return 1;
	if(num == 2)
		return 2;

	// call c or asm version of this function
	if(pure_call)
		return num * fact_stdcall_c(num-1, pure_call);
	else
		return num * fact_stdcall_asm(num-1, pure_call);
}


t_int __attribute__((__fastcall__)) fact_fastcall_c(t_int num, t_int pure_call)
{
	if(num < 1)
		return 1;
	if(num == 2)
		return 2;

	// call c or asm version of this function
	if(pure_call)
		return num * fact_fastcall_c(num-1, pure_call);
	else
		return num * fact_fastcall_asm(num-1, pure_call);
}


int main()
{
	fprintf(stdout, "cdecl\n");
	for(t_int num=0; num<12; ++num)
	{
		t_int res1 = fact_cdecl_c(num, 1);
		t_int res2 = fact_cdecl_asm(num, 1);
		t_int res3 = fact_cdecl_c(num, 0);

		fprintf(stdout, "\tc function          : %d! = %d\n", num, res1);
		fprintf(stdout, "\tasm function        : %d! = %d\n", num, res2);
		fprintf(stdout, "\tc/asm mixed function: %d! = %d\n", num, res3);
		fprintf(stdout, "\t\n");
	}


	fprintf(stdout, "stdcall\n");
	for(t_int num=0; num<12; ++num)
	{
		t_int res1 = fact_stdcall_c(num, 1);
		t_int res2 = fact_stdcall_asm(num, 1);
		t_int res3 = fact_stdcall_c(num, 0);

		fprintf(stdout, "\tc function          : %d! = %d\n", num, res1);
		fprintf(stdout, "\tasm function        : %d! = %d\n", num, res2);
		fprintf(stdout, "\tc/asm mixed function: %d! = %d\n", num, res3);
		fprintf(stdout, "\t\n");
	}


	fprintf(stdout, "fastcall\n");
	for(t_int num=0; num<12; ++num)
	{
		t_int res1 = fact_fastcall_c(num, 1);
		t_int res2 = fact_fastcall_asm(num, 1);
		t_int res3 = fact_fastcall_c(num, 0);

		fprintf(stdout, "\tc function          : %d! = %d\n", num, res1);
		fprintf(stdout, "\tasm function        : %d! = %d\n", num, res2);
		fprintf(stdout, "\tc/asm mixed function: %d! = %d\n", num, res3);
		fprintf(stdout, "\t\n");
	}

	return 0;
}
