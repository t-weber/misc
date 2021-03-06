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
#include <list>

#include <FlexLexer.h>

#include "ast.h"
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

		virtual yy::Parser::symbol_type yylex(yy::ParserContext &context) /*override*/;

		virtual void LexerOutput(const char* str, int len) override;
		virtual void LexerError(const char* err) override;
	};


	/**
 	* holds parser state
 	*/
	class ParserContext
	{
	private:
		yy::Lexer m_lex;
		std::list<std::shared_ptr<AST>> m_statements;

	public:
		yy::Lexer& GetLexer()
		{
			return m_lex;
		}

		void AddStatement(std::shared_ptr<AST> stmt)
		{
			m_statements.push_back(stmt);
		}

		const std::list<std::shared_ptr<AST>>& GetStatements() const
		{
			return m_statements;
		}
	};
}


// yylex definition for lexer
#undef YY_DECL
#define YY_DECL yy::Parser::symbol_type yy::Lexer::yylex(yy::ParserContext &context)

// yylex function which the parser calls
extern yy::Parser::symbol_type yylex(yy::ParserContext &context);


// stop parsing
#define yyterminate() { return yy::Parser::by_type::kind_type(0); }


#endif
