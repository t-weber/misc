/**
 * lexer test
 * @author Tobias Weber
 * @date 26-may-18
 * @license see 'LICENSE.GPL' file
 *
 * References: 	https://github.com/westes/flex/tree/master/examples/manual
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


%%

"print"		{ return yy::Parser::make_PRINT(); }
"\n"		{ return yy::Parser::make_NEWLINE(); }
[ \t]		/* whitespace */
[A-Za-z0-9]*	{ return yy::Parser::make_STRING(yytext); }

%%
