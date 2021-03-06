#!/bin/bash
#
# protected mode test
# @author Tobias Weber
# @date apr-21
# @license: see 'LICENSE.GPL' file
#


ASM=nasm
#CC=clang
CC=gcc


declare -a ASM_FILES=("fact32.asm")
declare -a C_FILES=("main32.c")
declare -a O_FILES=()
OUT_FILE="tst32"


for ASM_FILE in ${ASM_FILES[@]}; do
	O_FILE=${ASM_FILE%.asm}.o
	echo -e "Assembling ${ASM_FILE} -> ${O_FILE}..."

	if ! ${ASM} -f elf -w+all -o ${O_FILE} ${ASM_FILE}; then
		echo -e "Error assembling."
		exit -1
	fi

	O_FILES+=(${O_FILE})
done



for C_FILE in ${C_FILES[@]}; do
	O_FILE=${C_FILE%.c}.o
	echo -e "Compiling ${C_FILE} -> ${O_FILE}..."

	if ! ${CC} -m32 -O2 -march=i686 -c -o ${O_FILE} ${C_FILE}; then
		echo -e "Error compiling."
		exit -1
	fi

	O_FILES+=(${O_FILE})
done


echo -e "Linking ${O_FILES[@]} -> ${OUT_FILE}..."
if ! ${CC} -m32 -o ${OUT_FILE} ${O_FILES[@]} -lc; then
	echo -e "Error linking."
	exit -1
fi
