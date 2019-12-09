/**
 * parser test
 * @author Tobias Weber
 * @date 07-dec-19
 * @license see 'LICENSE.GPL' file
 *
 * References:	https://github.com/westes/flex/tree/master/examples/manual
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
%type<double> term_add
%type<double> term_add_rest
%type<double> term_mul
%type<double> term_mul_rest
%type<double> term_pow
%type<double> term_pow_rest
%type<double> factor


%%
// non-terminals / grammar

commands
	: command commands	{ $<double>$ = $<double>1; }	// directly define type
	| /* epsilon */		{ $<double>$ = 0.; }
	;

command
	: term_add { /*std::cout << $1 << std::endl;*/ } NEWLINE
		{ $<double>$ = $1; std::cout << $1 << std::endl; }
	;


// lowest priority operators: +,-
term_add[sum]
	: term_mul[term1] term_add_rest[term2]		{ $sum = +$term1 + $term2; }
	| '+' term_mul[term1] term_add_rest[term2]	{ $sum = +$term1 + $term2; }
	| '-' term_mul[term1] term_add_rest[term2]	{ $sum = -$term1 + $term2; }
	;

term_add_rest[sum]
	: '+' term_mul[term1] term_add_rest[term2]	{ $sum = +$term1 + $term2; }
	| '-' term_mul[term1] term_add_rest[term2]	{ $sum = -$term1 + $term2; }
	| /*%empty*/	{ $sum = 0.; }
	;


// middle priority operators: *,/,%
term_mul[prod]
	: term_pow[term1] term_mul_rest[term2]	{ $prod = $term1 * $term2; }
	;

term_mul_rest[prod]
	: '*' term_pow[term1] term_mul_rest[term2]	{ $prod = $term1 * $term2; }
	| '/' term_pow[term1] term_mul_rest[term2]	{ $prod = 1./$term1 * $term2; }
	| '%' term_pow[term1] term_mul_rest[term2]	{ $prod = std::fmod($<double>0, $term1) * $term2; }	// !! $0 dangerous !!
	| /*eps*/		{ $prod = 1.; }
	;


// highest priority operators: ^
term_pow[pot]
	: factor[term1] term_pow_rest[term2]	{ $pot = std::pow($term1, $term2); }
	;

term_pow_rest[pot]
	: '^' factor[term1] term_pow_rest[term2]	{ $pot = std::pow($term1, $term2); }
	| /*eps*/		{ $pot = 1.; }
	;


// even higher priority
factor[val]
	: REAL[num]		{ $val = $num; }
	| IDENT[ident]		{ if($ident == "pi") $val = M_PI; }
	| IDENT[ident] '(' term_add[arg] ')'{ if($ident == "sin") $val = std::sin($arg); }
	| '(' term_add[term] ')'	{ $val = $term; }
	;

%%
