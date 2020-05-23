/**
 * parser test
 * @author Tobias Weber
 * @date 20-dec-19
 * @license: see 'LICENSE.GPL' file
 */

#include "ast.h"
#include "parser.h"
#include "llasm.h"

#include <fstream>
#include <boost/program_options.hpp>
namespace args = boost::program_options;


/**
 * Lexer error output
 */
void yy::Lexer::LexerError(const char* err)
{
	std::cerr << "Lexer error in line " << GetCurLine()
		<< ": " << err << "." << std::endl;
}


/**
 * Lexer message
 */
void yy::Lexer::LexerOutput(const char* str, int /*len*/)
{
	std::cerr << "Lexer output (line " << GetCurLine()
		<< "): " << str << "." << std::endl;
}


/**
 * Parser error output
 */
void yy::Parser::error(const std::string& err)
{
	std::cerr << "Parser error in line " << context.GetCurLine()
		<< ": " << err << "." << std::endl;
}


/**
 * call lexer from parser
 */
extern yy::Parser::symbol_type yylex(yy::ParserContext &context)
{
	return context.GetLexer().yylex(context);
}


int main(int argc, char** argv)
{
	try
	{
		// llvm toolchain
		std::string tool_opt = "opt";
		std::string tool_bc = "llvm-as";
		std::string tool_bclink = "llvm-link";
		std::string tool_interp = "lli";
		std::string tool_s = "llc";
		std::string tool_o = "clang";
		std::string tool_exec = "clang";
		std::string tool_strip = "llvm-strip";


		// --------------------------------------------------------------------
		// get program arguments
		// --------------------------------------------------------------------
		std::vector<std::string> vecProgs;
		bool interpret = false;
		bool optimise = false;
		bool show_symbols = false;
		std::string outprog;

		args::options_description arg_descr("Compiler arguments");
		arg_descr.add_options()
			("out,o", args::value(&outprog), "compiled program output")
			("optimise,O", args::bool_switch(&optimise), "optimise program")
			("interpret,i", args::bool_switch(&interpret), "directly run program in interpreter")
			("symbols,s", args::bool_switch(&show_symbols), "print symbol table")
			("program", args::value<decltype(vecProgs)>(&vecProgs), "input program to compile");

		args::positional_options_description posarg_descr;
		posarg_descr.add("program", -1);

		args::options_description arg_descr_toolchain("Toolchain programs");
		arg_descr_toolchain.add_options()
			("tool_opt", args::value(&tool_opt), "llvm optimiser")
			("tool_bc", args::value(&tool_bc), "llvm bitcode assembler")
			("tool_bclink", args::value(&tool_bclink), "llvm bitcode linker")
			("tool_interp", args::value(&tool_interp), "llvm bitcode interpreter")
			("tool_bccomp", args::value(&tool_s), "llvm bitcode compiler")
			("tool_asm", args::value(&tool_o), "native assembler")
			("tool_link", args::value(&tool_exec), "native linker")
			("tool_strip", args::value(&tool_strip), "strip tool");
		arg_descr.add(arg_descr_toolchain);

		auto argparser = args::command_line_parser{argc, argv};
		argparser.style(args::command_line_style::default_style);
		argparser.options(arg_descr);
		argparser.positional(posarg_descr);

		args::variables_map mapArgs;
		auto parsedArgs = argparser.run();
		args::store(parsedArgs, mapArgs);
		args::notify(mapArgs);

		if(vecProgs.size() == 0)
		{
			std::cerr << "Please specify an input program.\n" << std::endl;
			std::cout << arg_descr << std::endl;
			return 0;
		}

		if(outprog == "")
		{
			outprog = "out";
			std::cerr << "No program output specified, using \"" << outprog << "\"." << std::endl;
		}

		std::string outprog_3ac = outprog + ".asm";
		std::string outprog_3ac_opt = outprog + "_opt.asm";
		std::string outprog_bc = outprog + ".bc";
		std::string outprog_linkedbc = outprog + "_linked.bc";
		std::string outprog_s = outprog + ".s";
		std::string outprog_o = outprog + ".o";

		std::string runtime_3ac = optimise ? "runtime_opt.asm" : "runtime.asm";
		std::string runtime_bc = "runtime.bc";
		// --------------------------------------------------------------------



		// --------------------------------------------------------------------
		// parse input
		// --------------------------------------------------------------------
		const std::string& inprog = vecProgs[0];
		std::cout << "Parsing \"" << inprog << "\"..." << std::endl;

		std::ifstream ifstr{inprog};
		yy::ParserContext ctx{ifstr};

		// register runtime functions
		ctx.AddFunc("pow", SymbolType::SCALAR, {SymbolType::SCALAR, SymbolType::SCALAR});
		ctx.AddFunc("sin", SymbolType::SCALAR, {SymbolType::SCALAR});
		ctx.AddFunc("cos", SymbolType::SCALAR, {SymbolType::SCALAR});
		ctx.AddFunc("sqrt", SymbolType::SCALAR, {SymbolType::SCALAR});
		ctx.AddFunc("exp", SymbolType::SCALAR, {SymbolType::SCALAR});
		ctx.AddFunc("fabs", SymbolType::SCALAR, {SymbolType::SCALAR});
		ctx.AddFunc("labs", SymbolType::INT, {SymbolType::INT});

		ctx.AddFunc("strlen", SymbolType::INT, {SymbolType::STRING});
		ctx.AddFunc("strncpy", SymbolType::STRING, {SymbolType::STRING, SymbolType::STRING, SymbolType::INT});
		ctx.AddFunc("strncat", SymbolType::STRING, {SymbolType::STRING, SymbolType::STRING, SymbolType::INT});
		ctx.AddFunc("memcpy", SymbolType::STRING, {SymbolType::STRING, SymbolType::STRING, SymbolType::INT});

		ctx.AddFunc("putstr", SymbolType::VOID, {SymbolType::STRING});
		ctx.AddFunc("putflt", SymbolType::VOID, {SymbolType::SCALAR});
		ctx.AddFunc("putint", SymbolType::VOID, {SymbolType::INT});

		ctx.AddFunc("flt_to_str", SymbolType::VOID, {SymbolType::SCALAR, SymbolType::STRING, SymbolType::INT});
		ctx.AddFunc("int_to_str", SymbolType::VOID, {SymbolType::INT, SymbolType::STRING, SymbolType::INT});

		//ctx.AddFunc("ext_determinant", SymbolType::SCALAR, {SymbolType::MATRIX, SymbolType::INT});

		yy::Parser parser(ctx);
		int res = parser.parse();
		if(res != 0)
		{
			std::cerr << "Parser reports failure." << std::endl;
			return res;
		}

		if(show_symbols)
		{
			std::cout << "\nSymbol table:\n";
			std::cout << ctx.GetSymbols() << std::endl;
		}
		// --------------------------------------------------------------------



		// --------------------------------------------------------------------
		// 3AC generation
		// --------------------------------------------------------------------
		std::cout << "Generating intermediate code: \""
			<< inprog << "\" -> \"" << outprog_3ac << "\"..." << std::endl;

		std::ofstream ofstr{outprog_3ac};
		std::ostream* ostr = &ofstr /*&std::cout*/;
		LLAsm llasm{&ctx.GetSymbols(), ostr};
		auto stmts = ctx.GetStatements()->GetStatementList();
		for(auto iter=stmts.rbegin(); iter!=stmts.rend(); ++iter)
		{
			(*iter)->accept(&llasm);
			(*ostr) << std::endl;
		}


		// additional runtime/startup code
		(*ostr) << "\n" << R"START(
; -----------------------------------------------------------------------------
; imported libc functions
declare double @pow(double, double)
declare double @sin(double)
declare double @cos(double)
declare double @sqrt(double)
declare double @exp(double)
declare double @fabs(double)
declare i64 @labs(i64)

declare i64 @strlen(i8*)
declare i8* @strncpy(i8*, i8*, i64)
declare i8* @strncat(i8*, i8*, i64)
declare i32 @puts(i8*)
declare i32 @snprintf(i8*, i64, i8*, ...)
declare i8* @memcpy(i8*, i8*, i64)
declare i8* @malloc(i64)
declare void @free(i8*)
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; external runtime functions from runtime.cpp
declare double @ext_determinant(double*, i64)
declare i64 @ext_power(double*, double*, i64, i64)
declare i64 @ext_transpose(double*, double*, i64, i64)
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; constants
@__strfmt_g = constant [3 x i8] c"%g\00"
@__strfmt_ld = constant [4 x i8] c"%ld\00"
@__str_vecbegin = constant [3 x i8] c"[ \00"
@__str_vecend = constant [3 x i8] c" ]\00"
@__str_vecsep = constant [3 x i8] c", \00"
@__str_matsep = constant [3 x i8] c"; \00"
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; runtime functions

; double -> string
define void @flt_to_str(double %flt, i8* %strptr, i64 %len)
{
	%fmtptr = bitcast [3 x i8]* @__strfmt_g to i8*
	call i32 (i8*, i64, i8*, ...) @snprintf(i8* %strptr, i64 %len, i8* %fmtptr, double %flt)
	ret void
}

; int -> string
define void @int_to_str(i64 %i, i8* %strptr, i64 %len)
{
	%fmtptr = bitcast [4 x i8]* @__strfmt_ld to i8*
	call i32 (i8*, i64, i8*, ...) @snprintf(i8* %strptr, i64 %len, i8* %fmtptr, i64 %i)
	ret void
}

; output a string
define void @putstr(i8* %val)
{
	call i32 (i8*) @puts(i8* %val)
	ret void
}

; output a float
define void @putflt(double %val)
{
	; convert to string
	%strval = alloca [64 x i8]
	%strvalptr = bitcast [64 x i8]* %strval to i8*
	call void @flt_to_str(double %val, i8* %strvalptr, i64 64)

	; output string
	call void (i8*) @putstr(i8* %strvalptr)
	ret void
}

; output an int
define void @putint(i64 %val)
{
	; convert to string
	%strval = alloca [64 x i8]
	%strvalptr = bitcast [64 x i8]* %strval to i8*
	call void @int_to_str(i64 %val, i8* %strvalptr, i64 64)

	; output string
	call void (i8*) @putstr(i8* %strvalptr)
	ret void
}

; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; main entry point for llvm
define i32 @main()
{
	; call entry function
	call void @start()

	ret i32 0
}
; -----------------------------------------------------------------------------
)START";

		(*ostr) << std::endl;
		// --------------------------------------------------------------------



		// --------------------------------------------------------------------
		// 3AC optimisation
		// --------------------------------------------------------------------
		if(optimise)
		{
			std::cout << "Optimising intermediate code: \""
				<< outprog_3ac << "\" -> \"" << outprog_3ac_opt << "\"..." << std::endl;

			std::string cmd_opt = tool_opt + " -stats -S --strip-debug -o "
				+ outprog_3ac_opt + " " + outprog_3ac;
			if(std::system(cmd_opt.c_str()) != 0)
			{
				std::cerr << "Failed." << std::endl;
				return -1;
			}

			outprog_3ac = outprog_3ac_opt;
		}
		// --------------------------------------------------------------------



		// --------------------------------------------------------------------
		// Bitcode generation
		// --------------------------------------------------------------------
		std::cout << "Assembling bitcode: \""
			<< outprog_3ac << "\" -> \"" << outprog_bc << "\"..." << std::endl;

		std::string cmd_bc = tool_bc + " -o " + outprog_bc + " " + outprog_3ac;
		if(std::system(cmd_bc.c_str()) != 0)
		{
			std::cerr << "Failed." << std::endl;
			return -1;
		}


		std::cout << "Assembling runtime bitcode: \""
			<< runtime_3ac << "\" -> \"" << runtime_bc << "\"..." << std::endl;

		cmd_bc = tool_bc + " -o " + runtime_bc + " " + runtime_3ac;
		if(std::system(cmd_bc.c_str()) != 0)
		{
			std::cerr << "Failed." << std::endl;
			return -1;
		}
		// --------------------------------------------------------------------



		// --------------------------------------------------------------------
		// Bitcode linking
		// --------------------------------------------------------------------
		std::cout << "Linking bitcode to runtime: \""
			<< outprog_bc << "\" + \"" << runtime_bc  << "\" -> \""
			<< outprog_linkedbc << "\"..." << std::endl;

		std::string cmd_bclink = tool_bclink + " -o " + outprog_linkedbc + " " + outprog_bc + " " + runtime_bc;
		if(std::system(cmd_bclink.c_str()) != 0)
		{
			std::cerr << "Failed." << std::endl;
			return -1;
		}
		// --------------------------------------------------------------------


		// interpret bitcode
		if(interpret)
		{
			std::cout << "Interpreting bitcode \"" << outprog_linkedbc << "\"..." << std::endl;

			std::string cmd_interp = tool_interp + " " + outprog_linkedbc;
			if(std::system(cmd_interp.c_str()) != 0)
			{
				std::cerr << "Failed." << std::endl;
				return -1;
			}
		}

		// compile bitcode
		else
		{
			std::cout << "Generating native assembly \""
				<< outprog_linkedbc << "\" -> \"" << outprog_s << "\"..." << std::endl;

			std::string opt_flag_s = optimise ? "-O2" : "";
			std::string cmd_s = tool_s + " " + opt_flag_s + " -o " + outprog_s + " " + outprog_linkedbc;
			if(std::system(cmd_s.c_str()) != 0)
			{
				std::cerr << "Failed." << std::endl;
				return -1;
			}


			std::cout << "Assembling native code \""
				<< outprog_s << "\" -> \"" << outprog_o << "\"..." << std::endl;

			std::string opt_flag_o = optimise ? "-O2" : "";
			std::string cmd_o = tool_o + " " + opt_flag_o + " -c -o " + outprog_o + " " + outprog_s;
			if(std::system(cmd_o.c_str()) != 0)
			{
				std::cerr << "Failed." << std::endl;
				return -1;
			}


			std::cout << "Generating native executable \""
				<< outprog_o << "\" -> \"" << outprog << "\"..." << std::endl;

			std::string opt_flag_exec = optimise ? "-O2" : "";
			std::string cmd_exec = tool_exec + " " + opt_flag_exec + " -o " + outprog + " " + outprog_o + " -lm -lc";
			if(std::system(cmd_exec.c_str()) != 0)
			{
				std::cerr << "Failed." << std::endl;
				return -1;
			}


			if(optimise)
			{
				std::cout << "Stripping debug symbols from \"" << outprog << "\"..." << std::endl;

				std::string cmd_strip = tool_strip + " " + outprog;
				if(std::system(cmd_strip.c_str()) != 0)
				{
					std::cerr << "Failed." << std::endl;
					return -1;
				}
			}
		}
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return -1;
	}

	return 0;
}
