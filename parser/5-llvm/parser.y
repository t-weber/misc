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
	#include "sym.h"
}

// after inclusion of definitions header
%code
{
	#include "parser.h"
	#include <cmath>
	#include <cstdint>
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
%token<std::int64_t> INT
%token<std::string> STRING
%token FUNC RET
%token SCALARDECL VECTORDECL MATRIXDECL STRINGDECL INTDECL
%token IF THEN ELSE
%token LOOP DO
%token EQU NEQ GT LT GEQ LEQ
%token AND OR NOT


// nonterminals
%type<std::shared_ptr<AST>> expr
%type<std::shared_ptr<AST>> statement
%type<std::shared_ptr<ASTStmts>> statements
%type<std::shared_ptr<ASTVarDecl>> variables
%type<std::shared_ptr<ASTArgNames>> all_argumentnames
%type<std::shared_ptr<ASTArgNames>> argumentnames
%type<std::shared_ptr<ASTArgs>> arguments
%type<std::shared_ptr<ASTStmts>> block
%type<std::shared_ptr<ASTFunc>> function
%type<std::shared_ptr<ASTTypeDecl>> typedecl


// precedences and left/right-associativity
// see: https://en.wikipedia.org/wiki/Order_of_operations
%nonassoc RET
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
	| /* epsilon */			{ $res = std::make_shared<ASTStmts>(); }
	;


variables[res]
	: IDENT[name] ',' variables[lst] {
			std::string symName = context.AddSymbol($name);
			$lst->AddVariable(symName);
			$res = $lst;
		}
	| IDENT[name] {
			std::string symName = context.AddSymbol($name);
			$res = std::make_shared<ASTVarDecl>();
			$res->AddVariable(symName);
			$res = $res;
		}
	;


statement[res]
	: expr[term] ';'	{ $res = $term; }
	| block[blk]		{ $res = $blk; }

	| function[func]	{ $res = $func;  }
	| RET expr[term] ';'	{ $res = std::make_shared<ASTReturn>($term); }
	| RET ';'		{ $res = std::make_shared<ASTReturn>(); }

	// variable declarations
	| SCALARDECL {
			context.SetSymType(SymbolType::SCALAR);
		}
		variables[vars] ';'	{ $res = $vars; }
	| VECTORDECL REAL[dim] {
			context.SetSymType(SymbolType::VECTOR);
			context.SetSymDims(unsigned($dim));
		}
		variables[vars] ';' {
			$res = $vars;
		}
	| MATRIXDECL REAL[dim1] REAL[dim2] {
			context.SetSymType(SymbolType::MATRIX);
			context.SetSymDims(unsigned($dim1), unsigned($dim2));
		}
		variables[vars] ';' {
			$res = $vars;
		}
	| STRINGDECL {
			context.SetSymType(SymbolType::STRING);
		}
		variables[vars] ';'	{ $res = $vars; }
	| INTDECL {
			context.SetSymType(SymbolType::INT);
		}
		variables[vars] ';'	{ $res = $vars; }

	| IF expr[cond] THEN statement[if_stmt] {
		$res = std::make_shared<ASTCond>($cond, $if_stmt); }
	| IF expr[cond] THEN statement[if_stmt] ELSE statement[else_stmt] {
		$res = std::make_shared<ASTCond>($cond, $if_stmt, $else_stmt); }

	| LOOP expr[cond] DO statement[stmt] {
		$res = std::make_shared<ASTLoop>($cond, $stmt); }
	;


function[res]
	: FUNC typedecl[rettype] IDENT[ident] {
			context.EnterScope($ident);
		}
		'(' all_argumentnames[args] ')' block[blk] {
			$res = std::make_shared<ASTFunc>($ident, $rettype, $args, $blk);

			context.LeaveScope($ident);
			context.AddFunc($ident, $rettype->GetType());
		}
	| FUNC IDENT[ident] {
			context.EnterScope($ident);
		}
		'(' all_argumentnames[args] ')' block[blk] {
			auto rettype = std::make_shared<ASTTypeDecl>(SymbolType::VOID);
			$res = std::make_shared<ASTFunc>($ident, rettype, $args, $blk);

			context.LeaveScope($ident);
			context.AddFunc($ident, SymbolType::VOID);
		}
	;


typedecl[res]
	: SCALARDECL	{ $res = std::make_shared<ASTTypeDecl>(SymbolType::SCALAR); }
	| VECTORDECL	{ $res = std::make_shared<ASTTypeDecl>(SymbolType::VECTOR); }
	| MATRIXDECL	{ $res = std::make_shared<ASTTypeDecl>(SymbolType::MATRIX); }
	| STRINGDECL	{ $res = std::make_shared<ASTTypeDecl>(SymbolType::STRING); }
	| INTDECL		{ $res = std::make_shared<ASTTypeDecl>(SymbolType::INT); }
	;


all_argumentnames[res]
	: argumentnames[args]	{ $res = $args; }
	| /*epsilon*/		{ $res = std::make_shared<ASTArgNames>(); }
	;


argumentnames[res]
	: typedecl[ty] IDENT[argname] ',' argumentnames[lst] {
			$lst->AddArg($argname, $ty->GetType());
			$res = $lst;
		}
	| typedecl[ty] IDENT[argname] {
			$res = std::make_shared<ASTArgNames>();
			$res->AddArg($argname, $ty->GetType()); 
		}
	;


arguments[res]
	: expr[arg] ',' arguments[lst]		{ $lst->AddArgument($arg); $res = $lst; }
	| expr[arg]	{
			$res = std::make_shared<ASTArgs>();
			$res->AddArgument($arg);
		}
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
	| expr[term1] '+' expr[term2]		{ $res = std::make_shared<ASTPlus>($term1, $term2, 0); }
	| expr[term1] '-' expr[term2]		{ $res = std::make_shared<ASTPlus>($term1, $term2, 1); }
	| expr[term1] '*' expr[term2]		{ $res = std::make_shared<ASTMult>($term1, $term2, 0); }
	| expr[term1] '/' expr[term2]		{ $res = std::make_shared<ASTMult>($term1, $term2, 1); }
	| expr[term1] '%' expr[term2]		{ $res = std::make_shared<ASTMod>($term1, $term2); }
	| expr[term1] '^' expr[term2]		{ $res = std::make_shared<ASTPow>($term1, $term2); }

	// comparison expressions
	| expr[term1] EQU expr[term2]		{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::EQU); }
	| expr[term1] NEQ expr[term2]		{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::NEQ); }
	| expr[term1] GT expr[term2]		{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::GT); }
	| expr[term1] LT expr[term2]		{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::LT); }
	| expr[term1] GEQ expr[term2]		{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::GEQ); }
	| expr[term1] LEQ expr[term2]		{ $res = std::make_shared<ASTComp>($term1, $term2, ASTComp::LEQ); }

	// constants
	| REAL[num]			{ $res = std::make_shared<ASTRealConst>($num); }
	| INT[num]			{ $res = std::make_shared<ASTIntConst>($num); }

	// variable
	| IDENT[ident]		{ $res = std::make_shared<ASTVar>($ident); }

	// function calls
	| IDENT[ident] '(' ')'	{ $res = std::make_shared<ASTCall>($ident); }
	| IDENT[ident] '(' arguments[args] ')' {
		$res = std::make_shared<ASTCall>($ident, $args);
	}

	// assignment
	| IDENT[ident] '=' expr[term]		{ $res = std::make_shared<ASTAssign>($ident, $term); }
	;

%%
