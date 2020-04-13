/**
 * lexer test
 * @author Tobias Weber
 * @date 09-dec-19
 * @license see 'LICENSE.GPL' file
 *
 * References:	https://github.com/westes/flex/tree/master/examples/manual
 *		http://www.gnu.org/software/bison/manual/html_node/index.html
 *		http://git.savannah.gnu.org/cgit/bison.git/tree/examples
 */

%option c++
%option noyywrap
%option yyclass = "yy::Lexer"


%{
	#include <string>
	#include "parser.h"
%}


white	[ \t\n]
num	[0-9]
real	{num}+(\.{num}*)?
ident	[A-Za-z_]+[A-Za-z0-9_]*


%%

{white}+	/* whitespace */

";"			{ return yytext[0]; }
"+"|"-"		{ return yytext[0]; }
"*"|"/"|"%"	{ return yytext[0]; }
"^"			{ return yytext[0]; }
"("|")"		{ return yytext[0]; }
"{"|"}"		{ return yytext[0]; }
"["|"]"		{ return yytext[0]; }
","			{ return yytext[0]; }

"=="		{ return yy::Parser::make_EQU(); }
"<>"		{ return yy::Parser::make_NEQ(); }
">="		{ return yy::Parser::make_GEQ(); }
"<="		{ return yy::Parser::make_LEQ(); }
">"			{ return yy::Parser::make_GT(); }
"<"			{ return yy::Parser::make_LT(); }
"and"		{ return yy::Parser::make_AND(); }
"or"		{ return yy::Parser::make_OR(); }
"not"		{ return yy::Parser::make_NOT(); }

"var"		{ return yy::Parser::make_VAR(); }

"="			{ return yytext[0]; }

"func"		{ return yy::Parser::make_FUNC(); }
"if"		{ return yy::Parser::make_IF(); }
"then"		{ return yy::Parser::make_THEN(); }
"else"		{ return yy::Parser::make_ELSE(); }

{real}		{ return yy::Parser::make_REAL(std::stod(yytext)); }
{ident}		{ return yy::Parser::make_IDENT(yytext); }

%%
