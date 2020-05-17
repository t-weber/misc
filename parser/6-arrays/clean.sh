#!/bin/sh
#
# parser test
# @author Tobias Weber
# @date 20-dec-19
# @license: see 'LICENSE.GPL' file
#

rm -f parser
rm -f parser_impl.cpp
rm -f parser_impl.output
rm -f parser_defs.h
rm -f stack.hh
rm -f lexer_impl.cpp
rm -f lexer_impl.h

rm -rf test.asm
rm -rf test.asm.bc
rm -rf test.asm.s
rm -rf test.asm.o
rm -rf test-opt.asm
rm -rf test-opt.asm.bc
rm -rf test-opt.asm.s
rm -rf test-opt.asm.o
rm -rf test.o

rm -f 0
