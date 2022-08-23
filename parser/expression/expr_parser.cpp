/**
 * simple LL(1) expression parser via recursive descent
 *
 * @author Tobias Weber
 * @date 14-mar-20
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- https://www.cs.uaf.edu/~cs331/notes/FirstFollow.pdf
 *	- https://de.wikipedia.org/wiki/LL(k)-Grammatik
 */

#include "expr_parser.h"


int main()
{
	ExprParser<double> parser;
	std::cout << parser.parse("a = 2 + 3*4") << std::endl;
	std::cout << parser.parse("(2 + (b=3))*4 + b*2") << std::endl;
	std::cout << parser.parse("pow((2 + 3)*4, 2)") << std::endl;
	std::cout << parser.parse("sqrt(400)") << std::endl;

	std::cout << "\nSymbols:" << std::endl;
	for(const auto& sym : parser.get_symbols())
		std::cout << sym.first << " = " << sym.second << std::endl;
	std::cout << "\n" << std::endl;


	ExprParser<int> parser2;
	std::cout << parser2.parse("2 + 3*4") << std::endl;
	std::cout << parser2.parse("(2 + 3)*4") << std::endl;
	std::cout << parser2.parse("pow((2 + 3)*4, 2)") << std::endl;
	std::cout << parser2.parse("sqrt(400)") << std::endl;

	return 0;
}
