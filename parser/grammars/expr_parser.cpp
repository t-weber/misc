/**
 * test simple LL(1) expression parser
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
	std::cout << parser.parse("2 + 3*4") << std::endl;
	std::cout << parser.parse("(2 + 3)*4") << std::endl;

	return 0;
}
