/**
 * jl test
 * @author Tobias Weber
 * @date 16-feb-2020
 * @license: see 'LICENSE.GPL' file
 *
 * g++ -std=c++17 -o jl jl2.cpp -lboost_system -lboost_filesystem -ldl
 */

#include <iostream>
#include <string>

#include <boost/system/error_code.hpp>
#include <boost/dll/shared_library.hpp>
namespace sys = boost::system;
namespace dll = boost::dll;


template<class t_sym, bool use_native_dl=false>
inline t_sym get_sym(std::shared_ptr<dll::shared_library> lib, const std::string& name)
{
	try
	{
		if(!lib->has(name))
		{
			std::cerr << "Cannot find address of symbol \"" << name << "\"." << std::endl;
			return nullptr;
		}


		t_sym sym = nullptr;

		if constexpr(use_native_dl)
		{
			void *pHandle = lib->native();
			sym = reinterpret_cast<t_sym>(::dlsym(pHandle, name.c_str()));

			if(!sym)
			{
				std::cerr << "Cannot get address of symbol \"" << name << "\": "
					<< ::dlerror() << "." << std::endl;
				return nullptr;
			}
		}
		else
		{
			sym = lib->get<std::remove_pointer_t<t_sym>>(name);
			//sym = reinterpret_cast<t_sym>(lib->get<void*>(name));
			if(!sym)
				std::cerr << "Cannot get address of symbol \"" << name << "\"." << std::endl;
		}

		std::cout << "Address of " << name << ": " << std::hex << (void*)sym << std::endl;
		return sym;
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return nullptr;
	}
}



/**
 * test using boost.dll
 */
void jl_tst()
{
	try
	{
		sys::error_code ec;
		std::shared_ptr<dll::shared_library> lib = std::make_shared<dll::shared_library>(
			"libjulia",
			dll::load_mode::search_system_folders | dll::load_mode::append_decorations |
			dll::load_mode::rtld_lazy | dll::load_mode::rtld_global | dll::load_mode::rtld_deepbind,
			ec);

		if(ec || !lib->is_loaded())
		{
			std::cerr << "Could not load libjulia, error code: " << ec << std::endl;
			return;
		}

		std::cout << lib->location() << " loaded." << std::endl;

		void (*jl_init)() = get_sym<void(*)()>(lib, "jl_init");
		if(!jl_init)
			jl_init = get_sym<void(*)()>(lib, "jl_init__threading");

		auto* jl_get_global = get_sym<void*(*)(void* pMod, void* pSym)>(lib, "jl_get_global");
		auto* jl_symbol = get_sym<void*(*)(const char*)>(lib, "jl_symbol");

		auto* jl_call0 = get_sym<void*(*)(void* pFkt)>(lib, "jl_call0");
		auto* jl_call1 = get_sym<void*(*)(void* pFkt, const void* pArg)>(lib, "jl_call1");

		auto* jl_eval_string = get_sym<void*(*)(const char*)>(lib, "jl_eval_string");
		auto* jl_string_ptr = get_sym<const char*(*)(void*)>(lib, "jl_string_ptr");
		auto* jl_pchar_to_string = get_sym<void*(*)(const char*, std::size_t)>(lib, "jl_pchar_to_string");

		auto* jl_cpu_threads = get_sym<int(*)()>(lib, "jl_cpu_threads");


		jl_init();

		std::cout << "Number of CPU threads: " << jl_cpu_threads() << std::endl;

		jl_eval_string("for x in range(0, stop=2*pi, length=32) print(sin(x), \", \"); end; print(\"\n\")");


		auto *pPrintLn = jl_eval_string("Base.println");
		jl_call1(pPrintLn, jl_pchar_to_string("Test", 4));
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}
}



int main(int argc, char **argv)
{
	jl_tst();
	return 0;
}
