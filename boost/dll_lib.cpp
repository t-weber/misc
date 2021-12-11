/**
 * dll tests and snippets
 * @author Tobias Weber
 * @date 9-dec-17
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++17 -shared -fPIC -o dll_lib.so dll_lib.cpp
 * x86_64-w64-mingw32-g++ -std=c++17 -shared -fPIC -Wl,--export-all-symbols -o dll_lib.so dll_lib.cpp
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_65_1/doc/html/boost_dll.html
 *  * https://github.com/boostorg/dll/tree/develop/example
 */

#include <iostream>
#include <string>
#include <vector>


void print()
{
	std::cout << "In library." << std::endl;
}


template<class T> T calc(T t1, T t2)
{
	return t1*t1 + t2*t2;
}


void print_str(const std::string& str)
{
	std::cout << "got string: " << str << std::endl;
}


std::vector<int> get_vec()
{
	return std::vector<int>{{1,2,3,4,5}};
}


// ----------------------------------------------------------------------------
// export

#ifndef __MINGW32__
	#include <boost/dll/alias.hpp>

	// using default section "boostdll"
	//BOOST_DLL_ALIAS(print, lib_print);
	//BOOST_DLL_ALIAS(calc<double>, lib_calc_d);
	//BOOST_DLL_ALIAS(calc<int>, lib_calc_i);

	// using an explicit section name
	// WARNING: function name and alias cannot be the same!
	BOOST_DLL_ALIAS_SECTIONED(print, lib_print, TheSec);
	BOOST_DLL_ALIAS_SECTIONED(calc<double>, lib_calc_d, TheSec);
	BOOST_DLL_ALIAS_SECTIONED(calc<int>, lib_calc_i, TheSec);
	BOOST_DLL_ALIAS_SECTIONED(print_str, lib_print_str, TheSec);
	BOOST_DLL_ALIAS_SECTIONED(get_vec, lib_get_vec, TheSec);
#else
	// mingw needs explicit exports
	extern "C" __declspec(dllexport) __cdecl void lib_print() { print(); }
	extern "C" __declspec(dllexport) __cdecl double lib_calc_d(double d1, double d2) { return calc<double>(d1, d2); }
	extern "C" __declspec(dllexport) __cdecl int lib_calc_i(int i1, int i2) { return calc<int>(i1, i2); }
	extern "C" __declspec(dllexport) __cdecl void lib_print_str(const std::string& str) { print_str(str); }
	extern "C" __declspec(dllexport) __cdecl std::vector<int> lib_get_vec() { return get_vec(); }
#endif

// ----------------------------------------------------------------------------
