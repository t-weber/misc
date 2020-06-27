/**
 * dll tests and snippets
 * @author Tobias Weber
 * @date 9-dec-17
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -o dll_prog dll_prog.cpp -std=c++17 -lboost_filesystem -lboost_system -ldl
 * x86_64-w64-mingw32-g++ -o dll_prog dll_prog.cpp -std=c++17 -lboost_filesystem-x64 -lboost_system-x64
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_65_1/doc/html/boost_dll.html
 *  * https://github.com/boostorg/dll/tree/develop/example
 */

#include <iostream>
#include <vector>
#include <string>

#include <boost/system/error_code.hpp>
namespace sys = boost::system;

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <boost/dll/library_info.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/dll/import.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
namespace dll = boost::dll;


int main()
{
	fs::path pathLib = "dll_lib.so";
	auto loadmode = /*dll::load_mode::append_decorations*/ dll::load_mode::default_mode & ~dll::load_mode::search_system_folders;


	// get library infos
	try
	{
		dll::library_info info(pathLib, 0);

		std::vector<std::string> vecInfos = info.symbols();
		std::cout << "\nAll symbols:\n";
		std::copy(vecInfos.begin(), vecInfos.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

		std::vector<std::string> vecSec = info.sections();
		for(const std::string& strSec : vecSec)
		{
			std::vector<std::string> vecInfosSec = info.symbols(strSec);
			std::cout << "\nSymbols in section " << strSec << ":\n\t";
			std::copy(vecInfosSec.begin(), vecInfosSec.end(), std::ostream_iterator<std::string>(std::cout, "\n\t"));
		}
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	std::cout << "\n";



	// load library & functions
	sys::error_code ec;
	if(auto lib = std::make_unique<dll::shared_library>(pathLib, loadmode, ec);
		!ec && lib->is_loaded())
	{
		std::cout << "Loaded " << lib->location() << "\n";

		try
		{
			// import function
			if(lib->has("lib_print"))
			{
				using t_fkt = void(*)();
				auto* funcPrint = lib->get<t_fkt>("lib_print");
				funcPrint();

				//using t_fkt_direct = std::remove_pointer_t<t_fkt>;
				//auto* funcPrint2 = lib->get<t_fkt_direct>("lib_print");
				//funcPrint2();
			}
			else
			{
				std::cerr << "Error: Function \"lib_print\" was not found." << std::endl;
			}

			// import function
			if(lib->has("lib_calc_i"))
			{
				auto* funcCalc = lib->get<int(*)(int,int)>("lib_calc_i");
				std::cout << "calc: " << funcCalc(2,3) << "\n";
			}
			else
			{
				std::cerr << "Error: Function \"lib_calc_i\" was not found." << std::endl;
			}
		}
		catch(const std::exception& ex)
		{
			std::cerr << ex.what() << std::endl;
		}
	}
	else
	{
		std::cerr << "Could not load library." << std::endl;
	}

	std::cout << "\n";



	// direct usage of function
	{
		auto func = dll::import<void(*)()>(pathLib, "lib_print", loadmode);
		if(func)
		{
			(*func)();
		}
		else
		{
			std::cerr << "Error: Function \"lib_print\" could not be imported." << std::endl;
		}
	}

	std::cout << "\n";



	// symbol infos
	{
		std::cout << "main(): " << dll::symbol_location_ptr(&main) << "\n";
		std::cout << "exit(): " << dll::symbol_location_ptr(&exit) << "\n";

		std::cout << "program: " << dll::program_location() << "\n";
		std::cout << "line: " << dll::this_line_location() << "\n";
	}

	return 0;
}
