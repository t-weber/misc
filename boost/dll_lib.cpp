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


void print()
{
	std::cout << "In library." << std::endl;
}

template<class T> T calc(T t1, T t2)
{
	return t1*t1 + t2*t2;
}


// ----------------------------------------------------------------------------
// export
#include <boost/dll/alias.hpp>

// using default section "boostdll"
//BOOST_DLL_ALIAS(print, lib_print);
//BOOST_DLL_ALIAS(add<double>, lib_add_d);
//BOOST_DLL_ALIAS(add<int>, lib_add_i);

// using an explicit section name
BOOST_DLL_ALIAS_SECTIONED(print, lib_print, TheSec);
BOOST_DLL_ALIAS_SECTIONED(calc<double>, lib_calc_d, TheSec);
BOOST_DLL_ALIAS_SECTIONED(calc<int>, lib_calc_i, TheSec);
// ----------------------------------------------------------------------------
