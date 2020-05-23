#!/bin/sh
#
# parser test
# @author Tobias Weber
# @date 20-dec-19
# @license: see 'LICENSE.GPL' file
#

BISON=bison
FLEX=flex

if [ "${CXX}" == "" ]; then
	CXX=g++
fi

OUTNAME=parser


echo -e "\n--------------------------------------------------------------------------------"
echo -e "Running $($BISON --version | head -n 1)...\n"

if ! ${BISON} -v --defines=parser_defs.h -o parser_impl.cpp parser.y; then
	echo -e "Syntax analysis step failed."
	exit -1
fi
echo -e "--------------------------------------------------------------------------------\n"



echo -e "\n--------------------------------------------------------------------------------"
echo -e "Running $($FLEX --version | head -n 1)...\n"

if ! ${FLEX} -v --header-file=lexer_impl.h -o lexer_impl.cpp lexer.l; then
	echo -e "Lexical analysis step failed."
	exit -1
fi
echo -e "--------------------------------------------------------------------------------\n"



echo -e "\n--------------------------------------------------------------------------------"
echo -e "Running $($CXX --version | head -n 1)...\n"
echo -e "Building parser...\n"

if ! ${CXX} -O2 -march=native -Wall -Wextra -std=c++17 -o ${OUTNAME} parser.cpp parser_impl.cpp lexer_impl.cpp llasm.cpp -lboost_program_options; then
	echo -e "Compilation of parser failed."
	exit -1
fi

echo -e "\nStripping ${OUTNAME} binary..."
strip -v ${OUTNAME}
echo -e "--------------------------------------------------------------------------------\n"



echo -e "\n--------------------------------------------------------------------------------"
echo -e "\nBuilding runtime...\n"
if ! clang -O2 -S -emit-llvm -o runtime.asm runtime.cpp; then
	echo -e "Compilation of runtime failed."
	exit -1
fi
echo -e "--------------------------------------------------------------------------------\n"
