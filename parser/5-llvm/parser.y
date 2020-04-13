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
%require "3.2"

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
%token FUNC
%token IF THEN ELSE
%token EQU NEQ GT LT GEQ LEQ
%token AND OR
%token NOT


// nonterminals
%type<std::shared_ptr<AST>> expr
%type<std::shared_ptr<AST>> statement
%type<std::shared_ptr<ASTStmts>> statements
%type<std::shared_ptr<ASTArgs>> arguments
%type<std::shared_ptr<ASTStmts>> block
%type<std::shared_ptr<AST>> function


// precedences and left/right-associativity
// see: https://en.wikipedia.org/wiki/Order_of_operations
%left ','
%right '='
%left OR
%left AND
%left GT LT GEQ LEQ
%left EQU NEQ
%left '+' '-'
%left '*' '/' '%'
%right UNARY_OP
%right '^'
%left '(' '[' '{'

// for the if/else r/s conflict shift "else"
// see: https://www.gnu.org/software/bison/manual/html_node/Non-Operators.html
%precedence IF THEN
%precedence ELSE


%%
// non-terminals / grammar
program
	: statements[stmts]		{ context.SetStatements($stmts); }
	;


statements[res]
	: statement[stmt] statements[lst]	{ $lst->AddStatement($stmt); $res = $lst; }
	| /* epsilon */						{ $res = std::make_shared<ASTStmts>(); }
	;


statement[res]
	: expr[term] ';'			{ $res = $term; }
	| block[blk]				{ $res = $blk; }

	| function[func]			{ $res = $func;  }

	| IF expr[cond] THEN statement[if_stmt] {
		$res = std::make_shared<ASTCond>($cond, $if_stmt); }
	| IF expr[cond] THEN statement[if_stmt] ELSE statement[else_stmt] {
		$res = std::make_shared<ASTCond>($cond, $if_stmt, $else_stmt); }
	;


function[res]
	: FUNC IDENT[ident] '(' arguments[args] ')' block[blk] {
		$res = std::make_shared<ASTFunc>($ident, $args, $blk); }
	;


arguments[res]
	: IDENT[argname] arguments[lst]	{ $lst->AddArg($argname); $res = $lst; }
	| /* epsilon */					{ $res = std::make_shared<ASTArgs>(); }
	;


block[res]
	: '{' statements[stmts] '}'		{ $res = $stmts; }
	;


expr[res]
	: '(' expr[term] ')'				{ $res = $term; }

	// unary expressions
	| '+' expr[term] %prec UNARY_OP		{ $res = $term; }
	| '-' expr[term] %prec UNARY_OP		{ $res = std::make_shared<ASTUMinus>($term); }

	// binary expressions
	| expr[term1] '+' expr[term2]			{ $res = std::make_shared<ASTPlus>($term1, $term2, 0); }
	| expr[term1] '-' expr[term2]			{ $res = std::make_shared<ASTPlus>($term1, $term2, 1); }
	| expr[term1] '*' expr[term2]			{ $res = std::make_shared<ASTMult>($term1, $term2, 0); }
	| expr[term1] '/' expr[term2]			{ $res = std::make_shared<ASTMult>($term1, $term2, 1); }
	| expr[term1] '%' expr[term2]			{ $res = std::make_shared<ASTMod>($term1, $term2); }
	| expr[term1] '^' expr[term2]			{ $res = std::make_shared<ASTPow>($term1, $term2); }

	// comparison expressions
	| expr[term1] EQU expr[term2]			{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::EQU); }
	| expr[term1] NEQ expr[term2]			{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::NEQ); }
	| expr[term1] GT expr[term2]			{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::GT); }
	| expr[term1] LT expr[term2]			{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::LT); }
	| expr[term1] GEQ expr[term2]			{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::GEQ); }
	| expr[term1] LEQ expr[term2]			{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::LEQ); }

	// constant
	| REAL[num]					{ $res = std::make_shared<ASTConst>($num); }

	// variable
	| IDENT[ident]				{ $res = std::make_shared<ASTVar>($ident); }

	// function call
	| IDENT[ident] '(' expr[arg] ')'		{ $res = std::make_shared<ASTCall>($ident, $arg); }
	| IDENT[ident] '(' expr[arg1] ',' expr[arg2] ')'{ $res = std::make_shared<ASTCall>($ident, $arg1, $arg2); }

	// assignment
	| IDENT[ident] '=' expr[term]			{ $res = std::make_shared<ASTAssign>($ident, $term); }
	;

%%
