/**
 * stacktrace test
 * @author Tobias Weber
 * @date may-2022
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_79_0/doc/html/stacktrace.html
 *  * https://www.boost.org/doc/libs/1_79_0/doc/html/stacktrace/configuration_and_build.html
 *
 * g++ -std=c++20 -g -DBOOST_STACKTRACE_USE_ADDR2LINE -o stacktrace stacktrace.cpp
 */

#include <iostream>
#include <cstddef>

#include <boost/stacktrace.hpp>
namespace st = boost::stacktrace;


static void print_frame(const st::frame& frame)
{
	std::cout << "code address: " << std::hex << frame.address() << std::dec;
	if(std::string name = frame.name(); name != "")
		std::cout << ", function name: \"" << name << "\"" ;
	if(std::string file = frame.source_file(); file != "")
		std::cout << ", file: \"" << file << "\" (line " << frame.source_line() << ")";
	std::cout << "." << std::endl;
}


static void print_trace()
{
	std::cout << "call stack: " << std::endl;
	for(const auto& frame : st::stacktrace{})
		print_frame(frame);
}


int main()
{
	{
		print_trace();
		std::cout << std::endl;
	}

	{
		std::cout << "Infos about print_frame() function: " << std::endl;
		print_frame(st::frame(&print_frame));
		std::cout << std::endl;

		std::cout << "Infos about print_trace() function: " << std::endl;
		print_frame(st::frame(&print_trace));
		std::cout << std::endl;

		std::cout << "Infos about main() function: " << std::endl;
		print_frame(st::frame(&main));
		std::cout << std::endl;
	}

	{
		st::stacktrace trace1{};
		std::cout << 'T' << 'e' << 's' << 't' << '1' << '2' << '3' << '4' << std::endl;
		st::stacktrace trace2{};

		const std::uint8_t* addr1 = static_cast<const std::uint8_t*>(trace1[0].address());
		const std::uint8_t* addr2 = static_cast<const std::uint8_t*>(trace2[0].address());
		std::cout << "address 1: " << std::hex << static_cast<const void*>(addr1) << std::endl;
		std::cout << "address 2: " << std::hex << static_cast<const void*>(addr2) << std::endl;

		// get code between the two traces
		std::cout << "memory dump: " << std::endl;
		for(const std::uint8_t* iter=addr1; iter<addr2; ++iter)
			std::cout << std::hex << static_cast<std::uint16_t>(*iter) << " ";
		std::cout << std::endl;
		for(const std::uint8_t* iter=addr1; iter<addr2; ++iter)
			if(std::isprint(*iter))
				std::cout << *iter << " ";
		std::cout << std::endl;
	}

	return 0;
}
