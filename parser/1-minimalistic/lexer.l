/**
 * lexer test
 * @author Tobias Weber
 * @date 26-may-18
 * @license see 'LICENSE.GPL' file
 *
 * Reference: https://github.com/westes/flex/tree/master/examples/manual
 */

option c
%option noyywrap


%{
	#include <string>
	#include "parser_defs.h"
	//extern "C" int yywrap();
%}


%%

"print"		{ return PRINT; }
"\n"		{ return NEWLINE; }
[ \t]		/* whitespace */
[A-Za-z0-9]*	{ yylval.data = new std::string(yytext); return STRING; }

%%
