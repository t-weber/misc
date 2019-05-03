/**
 * parser test
 * @author Tobias Weber
 * @date 3-may-19
 * @license see 'LICENSE.GPL' file
 */

%{
	#include <iostream>
	#include <string>

	int yyerror(const char* msg)
	{
		std::cout << "Error: " << msg << std::endl;
		return 0;
	}

	int yylex()
	{
		static int i = 0;
		int syms[] = { 'a', 'b' };

		if(i >= sizeof(syms)/sizeof(*syms))
			return EOF;

		return syms[i++];
	}

	extern int yyparse();

	int main()
	{
		return yyparse();
	}
%}


%token 'a' 'b'


%%


start
	: A { std::cout << "start -> A" << std::endl; }
	| B { std::cout << "start -> BB" << std::endl; }
	| /* eps */ { std::cout << "start -> eps" << std::endl; }
;


A
	: 'a' B { std::cout << "A -> aB" << std::endl; }
;


B
	: 'b' { std::cout << "B -> b" << std::endl; }
;


%%
