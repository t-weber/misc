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
	std::cerr << "Lexer error: " << err << std::endl;
}


/**
 * Lexer message
 */
void yy::Lexer::LexerOutput(const char* str, int /*len*/)
{
	std::cerr << "Lexer output: " << str << std::endl;
}


/**
 * Parser error output
 */
void yy::Parser::error(const std::string& err)
{
	std::cerr << "Parser error: " << err << std::endl;
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
	if(argc <= 1)
	{
		std::cerr << "Please specify a program." << std::endl;
		return -1;
	}

	std::ifstream ifstr{argv[1]};


	// parsing
	yy::ParserContext ctx{ifstr};
	yy::Parser parser(ctx);
	int res = parser.parse();

	if(res != 0)
		return res;


	// code generation

	LLAsm llasm;
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
declare i8 @putchar(i8)
; -----------------------------------------------------------------------------

; main entry point for llvm
define i32 @main()
{
	; call entry function
	%val = call double @start()

	; convert to string
	%strval = alloca [64 x i8]
	%strvalptr = bitcast [64 x i8]* %strval to i8*
	call i8* @gcvt(double %val, i32 6, i8* %strvalptr)

	; output string
	call i8 @puts(i8* %strvalptr)
	;call i8 @putchar(i8 10)

	ret i32 0
}
)START";

		std::cout << strStartup << std::endl;
	}

	return 0;
}
