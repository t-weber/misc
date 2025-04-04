#
# @author Tobias Weber <tobias.weber@tum.de>
# @date dec-2020
# @license see 'LICENSE.EUPL' file
#

HDL = ghdl

.PHONY: all
all: conv.o clkdiv.o sevenseg.o dbgout.o ram.o cpu.o calc.o

%.o: %.vhdl
	@echo "Building $@ from $<..."
	$(HDL) -a --std=08 $<
