/**
 * parser test
 * @author Tobias Weber
 * @date 27-may-18
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#undef yyFlexLexer

#include <iostream>
#include <string>
#include <FlexLexer.h>

#include "parser_defs.h"


namespace yy
{
	/**
	 * lexer
	 */
	class Lexer : public yyFlexLexer
	{
	public:
		Lexer() : yyFlexLexer(std::cin, std::cerr) {}
		Lexer(std::istream& istr) : yyFlexLexer(istr, std::cerr) {}
		virtual ~Lexer() = default;

		virtual Parser::symbol_type yylex(ParserContext &context) /*override*/;
	};
}


/**
 * holds parser state
 */
class ParserContext
{
private:
	yy::Lexer m_lex;

public:
	yy::Lexer& GetLexer() { return m_lex; }
};


// yylex definition for lexer
#undef YY_DECL
#define YY_DECL yy::Parser::symbol_type yy::Lexer::yylex(ParserContext &context)

// yylex function which the parser calls
extern yy::Parser::symbol_type yylex(ParserContext &context);


// unused
#define yyterminate() /**/


#endif
