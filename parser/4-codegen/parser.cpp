/**
 * parser test
 * @author Tobias Weber
 * @date 20-dec-19
 * @license: see 'LICENSE.GPL' file
 *
 * Test:
 * echo -e "(2+3)*(4-5)\n1+2" | ./parser
 */

#include "ast.h"
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
	int res = parser.parse();

	if(res != 0)
		return res;

	std::cout << "# Zero-address code:\n";
	for(auto iter=ctx.GetStatements().rbegin(); iter!=ctx.GetStatements().rend(); ++iter)
	{
		(*iter)->Generate0AC(std::cout);
		std::cout << std::endl;
	}

	return 0;
}
