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

rm -fv runtime.o
rm -fv runtime.asm
rm -fv runtime.s
rm -fv runtime.bc
rm -fv runtime.asm.bc

rm -fv test.asm
rm -fv test.asm.bc
rm -fv test.asm.s
rm -fv test.asm.o
rm -fv test-opt.asm
rm -fv test-opt.asm.bc
rm -fv test-opt.asm.s
rm -fv test-opt.asm.o
rm -fv test.o

rm -fv out
rm -fv out.asm
rm -fv out.bc
rm -fv out.s
rm -fv out_linked.bc
rm -fv out_linked.s
rm -fv out.o

rm -fv test_linked.bc

rm -fv 0
