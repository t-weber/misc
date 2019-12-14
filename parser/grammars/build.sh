#!/bin/sh

flex -o ll1_lexer.cpp ll1_expr.l && g++ -std=c++17 -o lr1_opprec lr1_opprec.cpp ll1_lexer.cpp
flex -o ll1_lexer.cpp ll1_expr.l && g++ -std=c++17 -o ll1_expr ll1_expr.cpp ll1_lexer.cpp
