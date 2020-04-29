/**
 * parser test
 * @author Tobias Weber
 * @date 20-dec-19
 * @license: see 'LICENSE.GPL' file
 *
 * Test:
 *   ./parser test/fibo.prog > test.asm && llvm-as test.asm && lli test.asm.bc
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
		ctx.AddFunc("putstr", SymbolType::VOID, {SymbolType::STRING});
		ctx.AddFunc("putflt", SymbolType::VOID, {SymbolType::SCALAR});
		ctx.AddFunc("putint", SymbolType::VOID, {SymbolType::INT});

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

declare i8* @gcvt(double, i32, i8*)
declare i8 @puts(i8*)
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; runtime functions

; output an int
define void @putstr(i8* %val)
{
	call i8 @puts(i8* %val)
	ret void
}

; output a float
define void @putflt(double %val)
{
	; convert to string
	%strval = alloca [64 x i8]
	%strvalptr = bitcast [64 x i8]* %strval to i8*
	call i8* @gcvt(double %val, i32 6, i8* %strvalptr)

	; output string
	call void @putstr(i8* %strvalptr)
	ret void
}

; output an int
define void @putint(i64 %val)
{
	; convert to float
	%fval = sitofp i64 %val to double

	; output string
	call void @putflt(double %fval)
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
