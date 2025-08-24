#!/bin/bash
#
# bare c++ program test
# @author Tobias Weber
# @date 24-aug-2025
# @license see 'LICENSE.GPL' file
#


if [ "$1" = "64" ]; then
    echo -e "Running 64 bit binary..."
    QEMU=qemu-system-riscv64
else
    echo -e "Running 32 bit binary..."
    QEMU=qemu-system-riscv32
fi

# https://www.qemu.org/docs/master/system/target-riscv.html#
echo -e "Exit with control+a x, enter monitor with control-a c.\n"

#${QEMU} -nographic -no-reboot -machine virt -bios bare.prog
${QEMU} -nographic -no-reboot -machine virt -bios bare.bin
