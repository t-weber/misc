#!/bin/sh

bison --defines=parser_defs.h -o parser_impl.cpp parser.y
flex --header-file=lexer_impl.h -o lexer_impl.cpp lexer.l
g++ -O2 -march=native -std=c++17 -o parser parser.cpp parser_impl.cpp lexer_impl.cpp
g++ -O2 -march=native -std=c++17 -o vm_0ac vm_0ac.cpp
