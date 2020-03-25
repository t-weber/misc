/**
 * swig interface test
 * @author Tobias Weber
 * @date 25-mar-20
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *	* http://www.swig.org/tutorial.html
 * 	* http://www.swig.org/Doc4.0/SWIGPlus.html#SWIGPlus
 *
 * swig -v -c++ -python expr_parser.i
 * g++ -std=c++17 -I/usr/include/python3.7/ -o _expr_parser.so -shared -fpic expr_parser_wrap.cxx
 */

%module expr_parser
%{
	#include "expr_parser.h"
	//template class ExprParser<double>;
%}

%include <std_string.i>
%include "expr_parser.h"

//%rename(ExprParserD) ExprParser<double>;
%template(ExprParserD) ExprParser<double>;
using ExprParserD = ExprParser<double>;
