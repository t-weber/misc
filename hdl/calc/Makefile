#
# @author Tobias Weber <tobias.weber@tum.de>
# @date dec-2020
# @license see 'LICENSE.EUPL' file
#

HDL = ghdl


.PHONY: all clean run

all: testbench


clean:
	rm -vf *.o
	rm -vf *.cf
	rm -vf *.vcd
	rm -vf cpu_tb-cpu_tb_arch


run:
	ghdl -r --std=08 cpu_tb cpu_tb_arch --vcd=cpu_tb.vcd --stop-time=6200ns


%.o: %.vhdl
	@echo "Building $@ from $<..."
	$(HDL) -a --std=08 $<


testbench: conv.o ram.o cpu.o testbench.o # clkdiv.o sevenseg.o dbgout.o calc.o
	$(HDL) -e --std=08 cpu_tb cpu_tb_arch
