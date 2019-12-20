#!/bin/sh

bison --defines=parser_defs.h -o parser_impl.cpp parser.y
flex --header-file=lexer_impl.h -o lexer_impl.cpp lexer.l
g++ -O2 -march=native -o parser parser.cpp parser_impl.cpp lexer_impl.cpp
