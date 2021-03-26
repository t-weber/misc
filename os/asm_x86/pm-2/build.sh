#!/bin/bash
#
# protected mode test
# @author Tobias Weber
# @date mar-21
# @license: see 'LICENSE.GPL' file
#

ASM=nasm
CC=clang
LD=ld

echo -e "Assembling..."
if ! ${ASM} -f elf -w+all -o pm.o pm.asm; then
	echo -e "Error assembling."
	exit -1
fi

echo -e "Compiling..."
if ! ${CC} -m32 -O2 -march=i686 -ffreestanding -c -o main.o main.c; then
	echo -e "Error compiling."
	exit -1
fi

echo -e "Linking..."
if ! ${LD} -m elf_i386 --oformat binary -o pm.x86 -Ttext=0x7c00 pm.o main.o; then
	echo -e "Error linking."
	exit -1
fi
