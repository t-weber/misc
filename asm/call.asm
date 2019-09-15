#
# Function call test
# @author Tobias Weber
# @date 24-mar-19
# @license: see 'LICENSE.EUPL' file
#


.text
	# push string
	la	$t0,	str
	sw	$t0, 	-8($sp)

	# push number
	lw	$t0,	num
	sw	$t0, 	-4($sp)
	jal	print

	j 	terminate



#
# output a string and a number
#
print:
	# create stack frame
	sub	$sp,	$sp,	8

	# output string
	lw	$a0,	0($sp)
	li	$v0,	4
	syscall

	# output number
	lw	$a0,	4($sp)
	li	$v0,	1
	syscall

	# unwind stack frame
	add	$sp,	$sp,	8
	jr	$ra



#
# end program
#
terminate:
	li 	$v0, 	10
	syscall



.data
	str:	.asciiz	"Number: "
	num:	.word	1234
