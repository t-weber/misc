#
# @author Tobias Weber (orcid: 0000-0002-7230-1932)
# @date aug-2022
# @license see 'LICENSE.EUPL' file
#

cmake_minimum_required(VERSION 3.12)


project(expr_parsers)
enable_language(CXX)

set(CMAKE_CXX_STANDARD 20)
add_definitions(-Wall -Wextra -Weffc++)
include_directories(${PROJECT_SOURCE_DIR})


# recursive descent expression parser
add_executable(expr_parser
	expr_parser.cpp expr_parser.h)


# recursive ascent expression parser
add_executable(expr_parser_recasc
	expr_parser_recasc.cpp expr_parser_recasc.h)
