/**
 * check if bison agrees with the output of my own parser generator for the same grammar
 * (if not, only trust the first ;))
 * @author Tobias Weber
 * @date 13-jul-20
 * @license see 'LICENSE.GPL' file
 *
 * only need to create the viable prefix transition graph:
 *   bison -g tst1.y && dot -Tpdf tst1.dot > tst1.pdf
 */

%{
	extern /*"C"*/ int yyerror(const char* msg) { return -1; };
	extern /*"C"*/ int yylex() { return -1; }
%}

%%

S
	: A
	| B
;

A
	: 'a'
;

B
	: 'b' B
	| /*eps*/
;

%%
