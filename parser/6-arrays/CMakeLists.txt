#
# @author Tobias Weber
# @date 11-apr-2020
# @license: see 'LICENSE.GPL' file
#

project(parser)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)


find_package(FLEX 2 REQUIRED)
find_package(BISON 3 REQUIRED)
find_package(Boost REQUIRED)


add_definitions(${Boost_CXX_FLAGS})

include_directories(
	"${PROJECT_SOURCE_DIR}"
	"${Boost_INCLUDE_DIRS}/.."
	"${CMAKE_CURRENT_BINARY_DIR}"
)



BISON_TARGET(parser_impl
	parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser_impl.cpp
	DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser_defs.h
	VERBOSE REPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser_report.txt
)


FLEX_TARGET(lexer_impl
	lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer_impl.cpp
	DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lexer_impl.h
)

ADD_FLEX_BISON_DEPENDENCY(lexer_impl parser_impl)



message("Flex outputs: ${FLEX_lexer_impl_OUTPUTS}")
message("Bison output source: ${BISON_parser_impl_OUTPUT_SOURCE}")
message("Bison output header: ${BISON_parser_impl_OUTPUT_HEADER}")



add_executable(parser
	parser.cpp parser.h ast.h sym.h llasm.cpp llasm.h
	${FLEX_lexer_impl_OUTPUTS}
	${BISON_parser_impl_OUTPUT_SOURCE} ${BISON_parser_impl_OUTPUT_HEADER}
)

#add_dependencies(parser parser_impl lexer_impl)
target_link_libraries(parser)
