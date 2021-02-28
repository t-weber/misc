/**
 * asm/gcc test
 * @author Tobias Weber
 * @date 02-apr-20
 * @license: see 'LICENSE.EUPL' file
 * @see: https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
 *
 * g++ -O2 -march=native -S -o 0.asm asmtst.cpp
 * clang++ -O2 -march=native -S -emit-llvm -o 0.asm asmtst.cpp
 */

#include <cstdint>
#include <iostream>


void tst0()
{
	std::uint64_t in_val = 0x1122334455667788;
	std::uint64_t out_val = 0;
	std::uint64_t shift_val = 8;

	asm(
		"movq %0, %%mm0\n"			// mm0 = in_val
		"movq %1, %%mm1\n"			// mm1 = shift_val
			: : "r" (in_val), "r" (shift_val)
	);
	asm("psllq %mm1, %mm0");			// mm0 <<= mm1
	asm("movq %%mm0, %0" : "=r" (out_val));		// out_val = mm0

	std::cout << "in:  " << std::hex << in_val << std::endl;
	std::cout << "out: " << std::hex << out_val << std::endl;
	std::cout << "tst: " <<  std::hex << (in_val << shift_val) << std::endl;
}


int main()
{
	tst0();
	return 0;
}
