#
# strlen test
# @author Tobias Weber
# @date 24-mar-19
# @license: see 'LICENSE.EUPL' file
#


.text
	la	$t0,	str
	sw	$t0, 	-4($sp)
	jal strlen
	# get return value
	lw	$t0,	-4($sp)

	# push length int
	sw	$t0, 	-4($sp)
	jal	print

	j 	terminate



#
# strlen
#
strlen:
	# create stack frame
	subi	$sp,	$sp,	4

	# char *
	lw	$t0,	0($sp)

	# set counter to 0
	xor	$t2,	$t2,	$t2

next:
	# get char
	lb	$t1,	($t0)
	beqz	$t1,	fin

	# inc char* and counter
	addi	$t0, 	$t0, 	1
	addi	$t2,	$t2,	1
	j next

fin:
	# unwind stack frame
	addi	$sp,	$sp,	4

	# return counter
	sw	$t2, 	-4($sp)
	jr	$ra



#
# output a number
#
print:
	# create stack frame
	subi	$sp,	$sp,	4

	# output number
	lw	$a0,	0($sp)
	li	$v0,	1
	syscall

	# unwind stack frame
	addi	$sp,	$sp,	4
	jr	$ra



#
# end program
#
terminate:
	li 	$v0, 	10
	syscall



.data
	str:	.asciiz	"Teststring 123 ABC"
