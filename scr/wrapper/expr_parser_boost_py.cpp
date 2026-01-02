/**
 * boost.python interface test
 * @author Tobias Weber
 * @date 23-dec-25
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *  - https://www.boost.org/doc/libs/latest/libs/python/doc/html/tutorial/tutorial/exposing.html
 *
 * clang++ -std=c++20 -I/opt/homebrew/Cellar/boost/1.90.0/include -I/opt/homebrew/Cellar/python@3.14/3.14.2/Frameworks/Python.framework/Versions/3.14/include/python3.14/ -L/opt/homebrew/Cellar/boost-python3/1.90.0/lib -lboost_python314 -L/opt/homebrew/Cellar/python@3.14/3.14.2/Frameworks/Python.framework/Versions/3.14/lib -lpython3.14 -shared -o expr_parser.so expr_parser_boost_py.cpp
 */

#include "expr_parser.h"

using t_real = double;
template class ExprParser<t_real>;
using ExprParserD = ExprParser<t_real>;
using ExprParserDPtr = std::shared_ptr<ExprParserD>;


#include <boost/python.hpp>
#include <memory>
namespace py = boost::python;

BOOST_PYTHON_MODULE(expr_parser)
{
	py::class_<ExprParserD, ExprParserDPtr>("ExprParserD", py::init</* no ctor arg */>())
		.def("parse", &ExprParserD::parse);
  
	py::register_ptr_to_python<ExprParserDPtr>();
};
