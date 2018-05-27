/**
 * parser test
 * @author Tobias Weber
 * @date 26-may-18
 * @license see 'LICENSE.GPL' file
 *
 * References: 	https://github.com/westes/flex/tree/master/examples/manual
 *		http://www.gnu.org/software/bison/manual/html_node/index.html
 *		http://git.savannah.gnu.org/cgit/bison.git/tree/examples
 */

// parser options
%skeleton "lalr1.cc"
%glr-parser

%define parser_class_name { Parser }
%define api.namespace { yy }
%define api.value.type variant	// instead of union
%define api.token.constructor	// symbol constructors


// code for parser_impl.cpp
%code
{
	#include "parser.h"
}

// (forward) declarations for parser_defs.h
%code requires
{
	class ParserContext;
}


// parameter to use for parser and yylex
%param { ParserContext &context }


// terminal symbols
%token PRINT
%token<std::string> STRING
%token NEWLINE


%%
// non-terminals / grammar

commands
: command commands
| /* epsilon */
;

command
: PRINT STRING
{
	std::cout << "Print: " << $2 << std::endl;
} NEWLINE
| NEWLINE
;

%%
