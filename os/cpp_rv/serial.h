/**
 * output to qemu's serial terminal
 * @author Tobias Weber
 * @date 24-aug-2025
 * @license see 'LICENSE.GPL' file
 */

#ifndef __SERIAL_TERM_H__
#define __SERIAL_TERM_H__


#include "string.h"


namespace serial {

/**
 * writes a char to the serial output register, 'thr',
 * if the 'thre' bit of the 'lsr' status register is set
 * @see https://github.com/qemu/qemu/blob/master/hw/char/serial.c
 * @see https://wiki.osdev.org/RISC-V_Bare_Bones
 */
template<typename t_char = char>
void print_char(t_char c) noexcept
{
	// register addresses
	volatile t_char* serial_out = reinterpret_cast<volatile t_char*>(0x10000000);    // thr
	volatile t_char* serial_status = reinterpret_cast<volatile t_char*>(0x10000005); // lsr

	// wait till the serial terminal is ready
	while(true)
	{
		if(*serial_status & static_cast<t_char>(1 << 5))  // lsr -> thre
			break;
	}

	// write the char to the output register
	*serial_out = c;
}


template<typename t_char = char>
void print(const t_char* c) noexcept
{
	while(*c)
	{
		print_char<t_char>(*c);
		++c;
	}
}


template<typename t_char = char>
void print(t_char* c) noexcept
{
	print<t_char>(const_cast<const t_char*>(c));
}


template<typename t_char>
void print(unsigned int i) noexcept
{
	char buf[16];
	str::uint_to_str<t_char, unsigned int>(i, buf, 10);
	print<t_char>(buf);
}


template<typename t_char>
void print(int i) noexcept
{
	char buf[16];
	str::int_to_str<t_char, int>(i, buf, 10);
	print<t_char>(buf);
}


template<typename t_char>
void print(const void* p) noexcept
{
	unsigned long l = reinterpret_cast<unsigned long>(p);

	char buf[sizeof(l)/8 + 1];
	str::int_to_str<t_char, unsigned long>(l, buf, 16);
	print<t_char>(buf);
}


template<typename t_char = char>
void print(void* c) noexcept
{
	print<t_char>(const_cast<const void*>(c));
}


template<typename t_char, typename t_arg, typename ...t_args>
void print(t_arg&& arg, t_args&& ...args) noexcept
{
	print<t_char>(arg);
	print<t_char>(args...);
}


} // namespace serial

#endif
