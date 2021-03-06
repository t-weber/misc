/**
 * jl test
 * @author Tobias Weber
 * @date 16-feb-2020
 * @license: see 'LICENSE.GPL' file
 */

#include <iostream>
#include <dlfcn.h>



template<class t_sym>
t_sym get_sym(void* pHandle, const char* pcName)
{
	t_sym sym = reinterpret_cast<t_sym>(::dlsym(pHandle, pcName));

	if(!sym)
	{
		std::cerr << "Cannot get address of symbol \"" << pcName << "\": "
			<< ::dlerror() << "." << std::endl;
		return nullptr;
	}

	return sym;
}



/**
 * test using libdl
 */
void jl_tst()
{
	void* pHandleJL = ::dlopen("libjulia-debug.so", RTLD_LAZY | RTLD_GLOBAL | RTLD_DEEPBIND);
	if(!pHandleJL)
	{
		std::cerr << "Cannot open shared object file: " << ::dlerror() << "." << std::endl;
		return;
	}

	auto* jl_init = get_sym<void(*)()>(pHandleJL, "jl_init");
	if(!jl_init)
		jl_init = get_sym<void(*)()>(pHandleJL, "jl_init__threading");

	auto* jl_get_global = get_sym<void*(*)(void* pMod, void* pSym)>(pHandleJL, "jl_get_global");
	auto* jl_symbol = get_sym<void*(*)(const char*)>(pHandleJL, "jl_symbol");

	auto* jl_call0 = get_sym<void*(*)(void* pFkt)>(pHandleJL, "jl_call0");
	auto* jl_call1 = get_sym<void*(*)(void* pFkt, const void* pArg)>(pHandleJL, "jl_call1");

	auto* jl_eval_string = get_sym<void*(*)(const char*)>(pHandleJL, "jl_eval_string");
	auto* jl_string_ptr = get_sym<const char*(*)(void*)>(pHandleJL, "jl_string_ptr");
	auto* jl_pchar_to_string = get_sym<void*(*)(const char*, std::size_t)>(pHandleJL, "jl_pchar_to_string");

	auto* jl_cpu_threads = get_sym<int(*)()>(pHandleJL, "jl_cpu_threads");


	(*jl_init)();


	/*void* jl_main_module = get_sym<void*>(pHandleJL, "jl_main_module");
	void* jl_core_module = get_sym<void*>(pHandleJL, "jl_core_module");
	void* jl_base_module = get_sym<void*>(pHandleJL, "jl_base_module");

	std::cout
		<< std::hex << "main module: " << jl_main_module << "\n"
		<< std::hex << "core module: " << jl_core_module << "\n"
		<< std::hex << "base module: " << jl_base_module << std::endl;*/


	std::cout << "Number of CPU threads: " << jl_cpu_threads() << std::endl;

	jl_eval_string("for x in range(0, stop=2*pi, length=32) print(sin(x), \", \"); end; print(\"\n\")");


	//void* pPrint = jl_get_global(jl_base_module, jl_symbol("print"));
	//void* pPrintLn = jl_get_global(jl_base_module, jl_symbol("println"));
	auto *pPrintLn = jl_eval_string("Base.println");
	jl_call1(pPrintLn, jl_pchar_to_string("Test", 4));


	dlclose(pHandleJL);
}



int main(int argc, char **argv)
{
	jl_tst();
	return 0;
}
