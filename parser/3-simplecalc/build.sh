#!/bin/sh

echo -e "Running bison\n--------------------------------------------------------------------------------"
bison --defines=parser_defs.h -o parser_impl.cpp --report=all --report-file=parser.report --graph=parser.graph --xml=parser.xml parser.y

echo -e "\nRunning flex\n--------------------------------------------------------------------------------"
flex -v --header-file=lexer_impl.h -o lexer_impl.cpp lexer.l

echo -e "\nRunning g++\n--------------------------------------------------------------------------------"
g++ -O2 -march=native -o parser parser.cpp parser_impl.cpp lexer_impl.cpp

echo -e "\nGenerating graph\n--------------------------------------------------------------------------------"
dot -Tsvg parser.graph -o parser.svg
inkscape parser.svg -o parser.pdf
