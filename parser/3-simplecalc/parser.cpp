/**
 * parser test
 * @author Tobias Weber
 * @date 07-dec-19
 * @license: see 'LICENSE.GPL' file
 *
 * bison --defines=parser_defs.h -o parser_impl.cpp parser.y
 * flex --header-file=lexer_impl.h -o lexer_impl.cpp lexer.l
 * g++ -o parser parser.cpp parser_impl.cpp lexer_impl.cpp
 *
 * Test:
 * echo -e "(2+3) * (6-3+1) - 1" | ./parser
 * echo -e "sin(pi/2)" | ./parser
 */

#include "parser.h"


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
void yy::Lexer::LexerOutput(const char* str, int len)
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


int main()
{
	yy::ParserContext ctx;
	yy::Parser parser(ctx);
	return parser.parse();
}
