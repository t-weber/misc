/**
 * parser test
 * @author Tobias Weber
 * @date 26-may-18
 * @license see 'LICENSE.GPL' file
 *
 * Reference: https://github.com/westes/flex/tree/master/examples/manual
 */

%{
	#include <iostream>
	#include <string>

	extern /*"C"*/ int yyerror(const char* msg);
	extern /*"C"*/ int yylex();
%}


%union
{
	void *data;
}


%token PRINT
%token<data> STRING
%token NEWLINE


%%

commands
: command commands
| /* epsilon */
;

command
: PRINT STRING
{
	std::cout << "Print: "
		<< *static_cast<std::string*>($2)
		<< std::endl;
} NEWLINE
| NEWLINE
;

%%
