/**
 * check if bison agrees with the output of my own parser generator for the same grammar
 * (if not, only trust the first ;))
 * @author Tobias Weber
 * @date 8-jun-22
 * @license see 'LICENSE.GPL' file
 *
 * only need to create the viable prefix transition graph:
 *   bison --report=all -g left_recursion.y && dot -Tpdf left_recursion.dot > left_recursion.pdf
 */

%{
	extern /*"C"*/ int yyerror(const char* msg) { return -1; };
	extern /*"C"*/ int yylex() { return -1; }
%}

%%

S
	: As
;

As
	: As A
	| %empty
;

A
	: 'a' 'b'
;

%%
