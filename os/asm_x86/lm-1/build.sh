#!/bin/bash
#
# long mode test
# @author Tobias Weber
# @date mar-21
# @license: see 'LICENSE.GPL' file
#


ASM=nasm
CC=clang
#CC=gcc
LD=ld


declare -a ASM_FILES=("lm.asm")
declare -a C_FILES=("main.c" "string.c")
declare -a O_FILES=()
OUT_FILE="lm.x86"


for ASM_FILE in ${ASM_FILES[@]}; do
	O_FILE=${ASM_FILE%.asm}.o
	echo -e "Assembling ${ASM_FILE} -> ${O_FILE}..."

	if ! ${ASM} -f elf64 -w+all -o ${O_FILE} ${ASM_FILE}; then
		echo -e "Error assembling."
		exit -1
	fi

	O_FILES+=(${O_FILE})
done



for C_FILE in ${C_FILES[@]}; do
	O_FILE=${C_FILE%.c}.o
	echo -e "Compiling ${C_FILE} -> ${O_FILE}..."

	if ! ${CC} -m64 -Os -ffreestanding -falign-functions=8 -c -o ${O_FILE} ${C_FILE}; then
		echo -e "Error compiling."
		exit -1
	fi

	O_FILES+=(${O_FILE})
done


echo -e "Linking ${O_FILES[@]} -> ${OUT_FILE}..."
if ! ${LD} -s -m elf_x86_64 --oformat=binary --section-start=.text=0x7c00 -o ${OUT_FILE} ${O_FILES[@]}; then
	echo -e "Error linking."
	exit -1
fi
