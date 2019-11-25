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

	#define TOK_REAL	1000
	#define TOK_IDENT	1001
	#define TOK_END		1002
%}


white	[ \t]
num	[0-9]
/* real	[+-]?{white}*{num}+(\.{num}*)? */
real	{num}+(\.{num}*)?
ident	[A-Za-z]+[A-Za-z0-9]*


%%


{white}*	/* whitespace */
"\n"	{ return TOK_END; }

"+"	{ return '+'; }
"-"	{ return '-'; }
"*"	{ return '*'; }
"/"	{ return '/'; }
"%"	{ return '%'; }
"^"	{ return '^'; }
"("	{ return '('; }
")"	{ return ')'; }
","	{ return ','; }


{real} {
	yylval = std::stod(yytext);
	return TOK_REAL;
}


{ident} {
	return TOK_IDENT;
}


. {
	std::cerr << "Unkown token encountered in lexer: \""
		<< yytext << "\"." << std::endl;
}


%%
