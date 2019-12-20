/**
 * parser test
 * @author Tobias Weber
 * @date 20-dec-19
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *		https://github.com/westes/flex/tree/master/examples/manual
 *		http://www.gnu.org/software/bison/manual/html_node/index.html
 *		http://git.savannah.gnu.org/cgit/bison.git/tree/examples
 *		https://de.wikipedia.org/wiki/LL(k)-Grammatik
 */

// parser options
%skeleton "lalr1.cc"
%glr-parser

//%define parser_class_name { Parser }
%define api.parser.class { Parser }
%define api.namespace { yy }
%define api.value.type variant	// instead of union
%define api.token.constructor	// symbol constructors


// code for parser_impl.cpp
// before inclusion of definitions header
%code requires
{
	#include "ast.h"
}

// after inclusion of definitions header
%code
{
	#include "parser.h"
	#include <cmath>
}


// (forward) declarations for parser_defs.h
%code requires
{
	namespace yy
	{
		class ParserContext;
	}
}


// parameter to use for parser and yylex
%param
{
	yy::ParserContext &context
}


// terminal symbols
%token<std::string> IDENT
%token<double> REAL
%token NEWLINE


// nonterminals
%type<std::shared_ptr<AST>> expr
%type<std::shared_ptr<AST>> statement


// precedences and left/right-associativity
// see: https://en.wikipedia.org/wiki/Order_of_operations
%left ','
%right '='
%left '+' '-'
%left '*' '/' '%'
%right UNARY_OP
%right '^'
%left '(' '[' '{'


%%
// non-terminals / grammar

statements
	: statement[stmt] statements	{ context.AddStatement($stmt); }
	| /* epsilon */			{ }
	;


statement[res]
	: expr[term] NEWLINE 					{ $res = $term; }
	;


expr[res]
	: '(' expr[term] ')'					{ $res = $term; }

	// unary expressions
	| '+' expr[term]	%prec UNARY_OP		{ $res = $term; }
	| '-' expr[term]	%prec UNARY_OP		{ $res = std::make_shared<ASTUMinus>($term); }

	// binary expressions
	| expr[term1] '+' expr[term2]			{ $res = std::make_shared<ASTPlus>($term1, $term2); }
	| expr[term1] '-' expr[term2]			{ $res = std::make_shared<ASTMinus>($term1, $term2); }
	| expr[term1] '*' expr[term2]			{ $res = std::make_shared<ASTMult>($term1, $term2); }
	| expr[term1] '/' expr[term2]			{ $res = std::make_shared<ASTDiv>($term1, $term2); }
	| expr[term1] '%' expr[term2]			{ $res = std::make_shared<ASTMod>($term1, $term2); }
	| expr[term1] '^' expr[term2]			{ $res = std::make_shared<ASTPow>($term1, $term2); }

	// constant
	| REAL[num]								{ $res = std::make_shared<ASTConst>($num); }

	// variable
	| IDENT[ident]							{ $res = std::make_shared<ASTVar>($ident); }

	// function call
	| IDENT[ident] '(' expr[arg] ')'		{ $res = std::make_shared<ASTCall>($ident, $arg); }
	| IDENT[ident] '(' expr[arg1] ',' expr[arg2] ')'{ $res = std::make_shared<ASTCall>($ident, $arg1, $arg2); }

	// assignment
	| IDENT[ident] '=' expr[term]			{ $res = std::make_shared<ASTAssign>($ident, $term); }
	;

%%
