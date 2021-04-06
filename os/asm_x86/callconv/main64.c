/**
 * calling convention test
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#include <stdio.h>

typedef unsigned long t_int;
typedef double t_float;


extern t_int __attribute__((__sysv_abi__)) fact_sysv_asm(t_int num, t_int pure_call);
extern t_float __attribute__((__sysv_abi__)) fact_sysv_float_asm(t_float num, t_int pure_call);
extern t_int __attribute__((__ms_abi__)) fact_ms_asm(t_int num, t_int pure_call);


t_int __attribute__((__sysv_abi__)) fact_sysv_c(t_int num, t_int pure_call)
{
	if(num <= 1)
		return 1;
	if(num == 2)
		return 2;

	// call c or asm version of this function
	if(pure_call)
		return num * fact_sysv_c(num-1, pure_call);
	else
		return num * fact_sysv_asm(num-1, pure_call);
}


t_float __attribute__((__sysv_abi__)) fact_sysv_float_c(t_float num, t_int pure_call)
{
	if(num <= 1.)
		return 1.;

	// call c or asm version of this function
	if(pure_call)
		return num * fact_sysv_float_c(num-1, pure_call);
	else
		return num * fact_sysv_float_asm(num-1, pure_call);
}


t_int __attribute__((__ms_abi__)) fact_ms_c(t_int num, t_int pure_call)
{
	if(num <= 1)
		return 1;
	if(num == 2)
		return 2;

	// call c or asm version of this function
	if(pure_call)
		return num * fact_ms_c(num-1, pure_call);
	else
		return num * fact_ms_asm(num-1, pure_call);
}


int main()
{
	fprintf(stdout, "sysv_abi\n");
	for(t_int num=0; num<12; ++num)
	{
		t_int res1 = fact_sysv_c(num, 1);
		t_int res2 = fact_sysv_asm(num, 1);
		t_int res3 = fact_sysv_c(num, 0);
		t_int res4 = fact_sysv_asm(num, 0);

		fprintf(stdout, "\tc function          : %d! = %d\n", num, res1);
		fprintf(stdout, "\tasm function        : %d! = %d\n", num, res2);
		fprintf(stdout, "\tc/asm mixed function: %d! = %d\n", num, res3);
		fprintf(stdout, "\tasm/c mixed function: %d! = %d\n", num, res4);
		fprintf(stdout, "\t\n");
	}


	fprintf(stdout, "sysv_abi (float)\n");
	for(t_int num=0; num<12; ++num)
	{
		t_float res1 = fact_sysv_float_c(num, 1);
		t_float res2 = fact_sysv_float_asm(num, 1);
		t_float res3 = fact_sysv_float_c(num, 0);
		t_float res4 = fact_sysv_float_asm(num, 0);

		fprintf(stdout, "\tc function          : %d! = %.1f\n", num, res1);
		fprintf(stdout, "\tasm function        : %d! = %.1f\n", num, res2);
		fprintf(stdout, "\tc/asm mixed function: %d! = %.1f\n", num, res3);
		fprintf(stdout, "\tasm/c mixed function: %d! = %.1f\n", num, res4);
		fprintf(stdout, "\t\n");
	}


	fprintf(stdout, "ms_abi\n");
	for(t_int num=0; num<12; ++num)
	{
		t_int res1 = fact_ms_c(num, 1);
		t_int res2 = fact_ms_asm(num, 1);
		t_int res3 = fact_ms_c(num, 0);
		t_int res4 = fact_ms_asm(num, 0);

		fprintf(stdout, "\tc function          : %d! = %d\n", num, res1);
		fprintf(stdout, "\tasm function        : %d! = %d\n", num, res2);
		fprintf(stdout, "\tc/asm mixed function: %d! = %d\n", num, res3);
		fprintf(stdout, "\tasm/c mixed function: %d! = %d\n", num, res4);
		fprintf(stdout, "\t\n");
	}

	return 0;
}
