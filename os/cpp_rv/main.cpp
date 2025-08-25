/**
 * bare c++ program test
 * @author Tobias Weber
 * @date 24-aug-2025
 * @license see 'LICENSE.GPL' file
 */

#include "string.h"
#include "serial.h"


#define DO_MEMTEST   1
#define DO_MAINPROG  1

#define RESULT_ADDR  0xff00 // address that is watched in the sv testbench
#define SERIAL_PRINT 1      // will block the testbench, because the 0x10000005 status reg is not set


/**
 * example calculation
 */
template<typename t_int>
t_int fac(t_int i) noexcept
{
	if(i == 0 || i == 1)
		return 1;
	return i * fac<t_int>(i - 1);
}


extern "C" int main() noexcept
{
#if DO_MEMTEST != 0
	extern const volatile void* _mem_base;
	unsigned long mem_base = reinterpret_cast<unsigned long>(&_mem_base);

	// inspect in qemu mon, for 64 bit: x /8c 0x8000ff00
	volatile char* buf = reinterpret_cast<volatile char*>(mem_base + RESULT_ADDR);
	buf[0] = 'A'; buf[1] = 'B'; buf[2] = 'C'; buf[3] = '\n'; buf[4] = 0;

#if SERIAL_PRINT != 0
	serial::print<char>(const_cast<char*>(buf));
#endif
#endif

#if DO_MAINPROG != 0
	for(unsigned int val = 0; val <= 10; ++val)
	{
		unsigned int res = fac<unsigned int>(val);
#if DO_MEMTEST != 0
		// write the result to memory
		*reinterpret_cast<volatile unsigned int*>(mem_base + RESULT_ADDR) = res;
#endif
		// print the result on the serial console
#if SERIAL_PRINT != 0
		serial::print<char>("Result: ", val, "! = ", res, ".\n");
#endif
	}
#endif

	return 0;
}


/**
 * main function for interrupt service routines
 * TODO
 */
extern "C" void isr_main() noexcept
{
	
}
