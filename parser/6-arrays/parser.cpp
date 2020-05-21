/**
 * parser test
 * @author Tobias Weber
 * @date 20-dec-19
 * @license: see 'LICENSE.GPL' file
 */

#include <fstream>

#include "ast.h"
#include "parser.h"
#include "llasm.h"


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
		if(argc <= 1)
		{
			std::cerr << "Please specify a program." << std::endl;
			return -1;
		}

		std::ifstream ifstr{argv[1]};


		// parsing
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

		//ctx.AddFunc("ext_determinant", SymbolType::SCALAR, {SymbolType::MATRIX, SymbolType::INT, SymbolType::INT});

		yy::Parser parser(ctx);
		int res = parser.parse();

		if(res != 0)
			return res;

		//std::cout << ctx.GetSymbols() << std::endl;


		// code generation
		LLAsm llasm{&ctx.GetSymbols()};
		auto stmts = ctx.GetStatements()->GetStatementList();
		for(auto iter=stmts.rbegin(); iter!=stmts.rend(); ++iter)
		{
			(*iter)->accept(&llasm);
			std::cout << std::endl;
		}

		// additional code to make it run
		{
			std::string strStartup= R"START(
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
; declare double @ext_determinant(double*, i64, i64);
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

			std::cout << "\n" << strStartup << std::endl;
		}
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return -1;
	}

	return 0;
}
