/**
 * lexer test
 * @author Tobias Weber
 * @date 23-nov-19
 * @license see 'LICENSE.GPL' file
 *
 * Reference: https://github.com/westes/flex/tree/master/examples/manual
 */

option c
%option noyywrap


%{
	#include <string>
	#include <iostream>

	double yylval = 0.;

	#define TOK_REAL 1000
	#define TOK_END 1001
%}


white	[ \t]
num	[0-9]
/* real	[+-]?{white}*{num}+(\.{num}*)? */
real	{num}+(\.{num}*)?


%%


{white}*	/* whitespace */
"\n"	{ return TOK_END; }

"+"	{ return '+'; }
"-"	{ return '-'; }
"*"	{ return '*'; }
"/"	{ return '/'; }
"%"	{ return '%'; }
"("	{ return '('; }
")"	{ return ')'; }


{real} {
	yylval = std::stod(yytext);
	return TOK_REAL;
}


. {
	std::cerr << "Unkown token encountered in lexer: " << yylex << "." << std::endl;
}


%%
