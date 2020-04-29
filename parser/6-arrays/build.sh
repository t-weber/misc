#!/bin/sh

bison --defines=parser_defs.h -o parser_impl.cpp parser.y
flex --header-file=lexer_impl.h -o lexer_impl.cpp lexer.l
clang++ -O2 -march=native -Wall -Wextra -std=c++17 -o parser parser.cpp parser_impl.cpp lexer_impl.cpp llasm.cpp
