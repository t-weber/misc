#!/bin/sh
#
# parser test
# @author Tobias Weber
# @date 20-dec-19
# @license: see 'LICENSE.GPL' file
#

rm -fv parser
rm -fv parser_impl.cpp
rm -fv parser_impl.output
rm -fv parser_defs.h
rm -fv stack.hh
rm -fv lexer_impl.cpp
rm -fv lexer_impl.h

rm -rfv runtime.o

rm -rfv test.asm
rm -rfv test.asm.bc
rm -rfv test.asm.s
rm -rfv test.asm.o
rm -rfv test-opt.asm
rm -rfv test-opt.asm.bc
rm -rfv test-opt.asm.s
rm -rfv test-opt.asm.o
rm -rfv test.o

rm -fv 0
