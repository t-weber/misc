/**
 * parser test
 * @author Tobias Weber
 * @date 26-may-18
 * @license: see 'LICENSE.GPL' file
 *
 * bison --defines=parser_defs.h -o parser_impl.cpp parser.y
 * flex -o lexer_impl.cpp lexer.l
 * g++ -o parser parser.cpp parser_impl.cpp lexer_impl.cpp
 */

#include <iostream>


extern /*"C"*/ int yyparse();
//extern "C" int yywrap() {  return 1; }

int yyerror(const char* msg)
{
	std::cerr << "Parser error: " << msg << std::endl;
	return 0;
}


int main()
{
	return yyparse();
}
