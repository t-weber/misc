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
#include <array>

#include <FlexLexer.h>

#include "ast.h"
#include "sym.h"
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

		SymTab m_symbols;

		// information about currently parsed symbol
		std::vector<std::string> m_curscope;
		SymbolType m_symtype = SymbolType::SCALAR;
		std::array<unsigned int, 2> m_symdims = {0, 0};

	public:
		ParserContext(std::istream& istr = std::cin) :
			m_lex{istr}, m_statements{}
		{}

		yy::Lexer& GetLexer() { return m_lex; }

		// --------------------------------------------------------------------
		void SetStatements(std::shared_ptr<ASTStmts> stmts) { m_statements = stmts; }
		const std::shared_ptr<ASTStmts> GetStatements() const { return m_statements; }
		// --------------------------------------------------------------------

		// --------------------------------------------------------------------
		// current function scope
		const std::vector<std::string>& GetScope() const { return m_curscope; }
		std::string GetScopeName() const
		{
			std::string name;
			for(const std::string& scope : m_curscope)
				name += scope + "::";	// scope name separator
			return name;
		}

		void EnterScope(const std::string& name) { m_curscope.push_back(name); }
		void LeaveScope(const std::string& name)
		{
			const std::string& curscope = *m_curscope.rbegin();

			if(curscope != name)
			{
				std::cerr << "Error in line " << GetCurLine()
					<< ": Trying to leave scope " << name
					<< ", but the top scope is " <<curscope << ".";
			}

			m_curscope.pop_back();
		}
		// --------------------------------------------------------------------

		// --------------------------------------------------------------------
		std::string AddSymbol(const std::string& name, bool bUseScope=true)
		{
			std::string symbol_with_scope = bUseScope ? GetScopeName() + name : name;
			m_symbols.AddSymbol(symbol_with_scope, name, m_symtype, m_symdims);
			return symbol_with_scope;
		}

		const SymTab& GetSymbols() const { return m_symbols; }
		SymTab& GetSymbols() { return m_symbols; }

		// type of current symbol
		void SetSymType(SymbolType ty) { m_symtype = ty; }

		// dimensions of vector and matrix symbols
		void SetSymDims(unsigned int dim1, unsigned int dim2=0)
		{
			m_symdims[0] = dim1;
			m_symdims[1] = dim2;
		}
		// --------------------------------------------------------------------

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
