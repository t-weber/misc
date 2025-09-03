#
# calls the main c++ program
# @author Tobias Weber
# @date 24-aug-2025
# @license see 'LICENSE.GPL' file
#
# References:
#   - https://github.com/YosysHQ/picorv32/tree/main/picosoc
#   - https://github.com/grughuhler/picorv32_tang_nano_unified/tree/main
#


.globl _entrypoint, _isr_entrypoint, _startup, main, isr_main


.text
	_entrypoint:
		#li gp, 0x80010000
		#li sp, 0x8000fff0
		la gp, _gp_addr
		la sp, _sp_addr

		# enable interrupts via instr_maskirq
		#.word(0x600600b)

		call _startup
		call main

		# exit via an envcall if it exists
		#li a7, 123
		#ecall

		ebreak
		j .


	.balign 32
	_isr_entrypoint:
		# create stack frame and store registers
		.set WORD_SIZE, 4
		addi sp, sp, -30*WORD_SIZE
		.set reg_idx, 0
		.irp reg, ra,gp,tp,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31
			sw \reg, (reg_idx*WORD_SIZE)(sp)
			.set reg_idx, reg_idx + 1
		.endr

		# TODO: pass irq number
		call isr_main

		# restore registers and remove stack frame
		.set reg_idx, 0
		.irp reg, ra,gp,tp,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31
			lw \reg, (reg_idx*WORD_SIZE)(sp)
			.set reg_idx, reg_idx + 1
		.endr
		addi sp, sp, +30*WORD_SIZE
		ret
