#
# swig (or also boost.python) interface test
# @author Tobias Weber
# @date 25-mar-20
# @license see 'LICENSE.GPL' file
#
# Reference for swig: http://www.swig.org/tutorial.html
#
# swig -c++ -python expr_parser.i
# g++ -std=c++17 -I/usr/include/python3.7/ -o _expr_parser.so -shared -fpic expr_parser_wrap.cxx
#
# (does not work if the java interface "expr_parser.so" is also present!)
#

import expr_parser as exp

while True:
	str = input("Enter an expression: ")
	parser = exp.ExprParserD()
	print(parser.parse(str));
