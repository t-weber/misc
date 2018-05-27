/**
 * parser test
 * @author Tobias Weber
 * @date 26-may-18
 * @license: see 'LICENSE.GPL' file
 *
 * bison --defines=parser_defs.h -o parser_impl.cpp parser.y
 * flex --header-file=lexer_impl.h -o lexer_impl.cpp lexer.l
 * g++ -o parser parser.cpp parser_impl.cpp lexer_impl.cpp
 */

#include "parser.h"


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
extern yy::Parser::symbol_type yylex(ParserContext &context)
{
	return context.GetLexer().yylex(context);
}


int main()
{
	ParserContext ctx;
	yy::Parser parser(ctx);
	return parser.parse();
}
