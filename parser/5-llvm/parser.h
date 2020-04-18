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

#include "ast.h"
#include "parser_defs.h"


namespace yy
{
	/**
	 * lexer
	 */
	class Lexer : public yyFlexLexer
	{
	private:
		std::size_t m_curline = 1;

	public:
		Lexer() : yyFlexLexer(std::cin, std::cerr) {}
		Lexer(std::istream& istr) : yyFlexLexer(istr, std::cerr) {}
		virtual ~Lexer() = default;

		virtual yy::Parser::symbol_type yylex(yy::ParserContext& context) /*override*/;

		virtual void LexerOutput(const char* str, int len) override;
		virtual void LexerError(const char* err) override;

		void IncCurLine() { ++m_curline; }
		std::size_t GetCurLine() const { return m_curline; }
	};


	/**
 	* holds parser state
 	*/
	class ParserContext
	{
	private:
		yy::Lexer m_lex;
		std::shared_ptr<ASTStmts> m_statements;

	public:
		ParserContext(std::istream& istr = std::cin) :
			m_lex{istr}, m_statements{}
		{}

		yy::Lexer& GetLexer() { return m_lex; }

		void SetStatements(std::shared_ptr<ASTStmts> stmts) { m_statements = stmts; }
		const std::shared_ptr<ASTStmts> GetStatements() const { return m_statements; }

		std::size_t GetCurLine() const { return m_lex.GetCurLine(); }
	};
}


// yylex definition for lexer
#undef YY_DECL
#define YY_DECL yy::Parser::symbol_type yy::Lexer::yylex(yy::ParserContext&)

// yylex function which the parser calls
extern yy::Parser::symbol_type yylex(yy::ParserContext &context);


// stop parsing
#define yyterminate() { return yy::Parser::by_type::kind_type(0); }


#endif
