#!/bin/sh


BISON=bison
FLEX=flex


if [ "${CXX}" == "" ]; then
	CXX=g++
fi



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

if ! ${CXX} -O2 -march=native -Wall -Wextra -std=c++17 -o parser parser.cpp parser_impl.cpp lexer_impl.cpp llasm.cpp; then
	echo -e "Compilation failed."
	exit -1
fi
echo -e "--------------------------------------------------------------------------------\n"
