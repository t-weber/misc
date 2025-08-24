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


.globl _entrypoint, _startup, main, isr_main


.text
	_entrypoint:
		#li gp, 0x80010000
		#li sp, 0x8000fff0
		la gp, _gp_addr
		la sp, _sp_addr

		call _startup
		call main

		# exit via an envcall if it exists
		#li a7, 123
		#ecall

		ebreak
		j .

	.balign 32
	_isr_entrypoint:
		# TODO
		#call isr_main
		mret
