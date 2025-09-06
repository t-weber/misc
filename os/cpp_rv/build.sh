#!/bin/bash

#
# bare c++ program test
# @author Tobias Weber
# @date 24-aug-2025
# @license see 'LICENSE.GPL' file
#

# tools
CC=riscv64-elf-gcc
CPP=riscv64-elf-cpp
CXX=riscv64-elf-g++
OBJCPY=riscv64-elf-objcopy
OBJDMP=riscv64-elf-objdump

USE_INTERRUPTS=0
TESTBENCH_DEFS="-DDEBUG"
#TESTBENCH_DEFS+=" -DRAM_DISABLE_PORT2"
CFLAGS="-std=c++20 -O2 -Wall -Wextra -Weffc++"

if [ "$USE_INTERRUPTS" != 0 ]; then
	TESTBENCH_DEFS+=" -DUSE_INTERRUPTS"
fi

#
# base integer (i) and mul/div (m)
# see: https://gcc.gnu.org/onlinedocs/gcc/RISC-V-Options.html
#
if [ "$1" = "64" ]; then
	echo -e "Building 64 bit binary..."
	ISA=rv64im
	ABI=lp64
	USE_64BIT=1
else
	echo -e "Building 32 bit binary..."
	ISA=rv32im
	ABI=ilp32
	USE_64BIT=0
fi

# files
CFILES="startup.cpp main.cpp"
LFILE="linker.ld"
PROGFILE=bare.prog
BINFILE=bare.bin


rm -fv "${PROGFILE}"
rm -fv "${BINFILE}"
rm -fv "${LFILE}"
rm -fv rom.sv
rm -fv entrypoint.s
rm -fv rv_tb



# preprocess entrypoint file
echo -e "\nentrypoint.s.in -> entrypoint.s..."
${CPP} -P -DUSE_64BIT=${USE_64BIT} -DUSE_INTERRUPTS=${USE_INTERRUPTS} \
	entrypoint.s.in -o entrypoint.s


# preprocess linker file
echo -e "\n${LFILE}.in -> ${LFILE}..."
${CPP} -P -DUSE_64BIT=${USE_64BIT} ${LFILE}.in -o ${LFILE}


# compile & link
echo -e "\n${CFILES} -> ${PROGFILE}..."
if ! ${CXX} ${CFLAGS} -time \
	-march=${ISA} -mabi=${ABI} -mcmodel=medany \
	-mno-save-restore -mno-riscv-attribute -mno-fdiv -mdiv \
	-nostartfiles -nolibc -nodefaultlibs -nostdlib++ -nostdlib \
	-fno-builtin -ffreestanding -static \
	-DUSE_INTERRUPTS=${USE_INTERRUPTS} \
	-T ${LFILE} -o ${PROGFILE} entrypoint.s ${CFILES}; then
	exit -1
fi

${OBJDMP} -tDS "${PROGFILE}"


# get binary image
echo -e "\n${PROGFILE} -> ${BINFILE}..."
if ! ${OBJCPY} -v -O binary "${PROGFILE}" "${BINFILE}"; then
	exit -1
fi

hexdump -C "${BINFILE}"


# create sv rom
echo -e "\n${BINFILE} -> rom.sv..."
if [ -e genrom ]; then
	if ! ./genrom -t sv -c 0 -p 1 -d 1 -w 32 -o rom.sv "${BINFILE}"; then
		exit -1
	fi
else
	echo -e "Error: Could not find genrom tool, get it here:"
	echo -e "\thttps://github.com/t-weber/electro/tree/main/tools/genrom"
	exit -1
fi


# build sv testbench
echo -e "\nBuilding testbench..."
if [ -d externals ]; then
	if ! iverilog -g2012 ${TESTBENCH_DEFS} \
		\-o rv_tb \
		externals/ram_2port.sv externals/memcpy.sv \
		externals/memsel.sv externals/picorv32.v \
		rom.sv rv_tb.sv; then
		exit -1
	fi
else
	echo -e "Error: Could not find externals, use the ./get_externals.sh script."
	exit -1
fi
