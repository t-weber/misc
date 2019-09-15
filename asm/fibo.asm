#
# recursive function call test
# @author Tobias Weber
# @date 15-sep-19
# @license: see 'LICENSE.EUPL' file
#


.text
	jal input
	lw	$t1,	-8($sp)


	# push string argument
	la	$t0,	fibo1
	sub	$sp,	$sp,	4
	sw	$t0, 	0($sp)

	# push number argument
	sub	$sp,	$sp,	4
	sw	$t1, 	0($sp)

	jal	print

	# unwind stack frame
	add	$sp,	$sp,	8



	# push number argument
	sub	$sp,	$sp,	4
	sw	$t1, 	0($sp)

	jal	fibo
	lw	$t2,	-8($sp)

	# unwind stack frame
	add	$sp,	$sp,	4



	# push string argument
	la	$t0,	fibo2
	sub	$sp,	$sp,	4
	sw	$t0, 	0($sp)

	# push number argument
	sub	$sp,	$sp,	4
	sw	$t2, 	0($sp)

	jal	print

	# unwind stack frame
	add	$sp,	$sp,	8


	j 	terminate



#
# Fibonacci number
#
fibo:
	# create stack frame
	sub	$sp,	$sp,	8

	# push return address
	sw	$ra,	4($sp)

	# get argument
	lw	$t5,	8($sp)
	ble	$t5,	2,	basecase
	j	generalcase


	basecase:
		xor	$t5,	$t5,	$t5
		add	$t5,	$t5,	1
		j saveresult

	generalcase:
		# fibo(arg-1)
		lw	$t5,	8($sp)
		sub	$t5,	$t5,	1

		# push argument
		sub	$sp,	$sp,	4
		sw	$t5, 	0($sp)

		jal	fibo

		# get return value
		lw	$t5,	-8($sp)

		# pop argument
		add	$sp,	$sp,	4

		# save return value to local stack var
		sw	$t5,	0($sp)



		# fibo(arg-2)
		lw	$t5,	8($sp)
		sub	$t5,	$t5,	2

		# push argument
		sub	$sp,	$sp,	4
		sw	$t5, 	0($sp)

		jal	fibo

		# get return value and add it to saved stack variable
		lw	$t5,	-8($sp)

		# pop argument
		add	$sp,	$sp,	4

		# add return value to saved local stack variable
		lw	$t6,	0($sp)
		add	$t5,	$t5,	$t6


	saveresult:
		# return result
		sw	$t5,	0($sp)


	# pop return address
	lw	$ra,	4($sp)

	# unwind stack frame
	add	$sp,	$sp,	8

	jr $ra



#
# input an int
#
input:
	# push return address
	sw	$ra,	-4($sp)

	la	$a0,	msg
	li	$v0,	51
	syscall

	# return int
	sw	$a0,	-8($sp)

	# pop return address
	lw	$ra,	-4($sp)
	jr $ra



#
# output a string and a number
#
print:
	# push return address
	sw	$ra,	-4($sp)

	# output string
	lw	$a0,	4($sp)
	li	$v0,	4
	syscall

	# output number
	lw	$a0,	0($sp)
	li	$v0,	1
	syscall

	# pop return address
	lw	$ra,	-4($sp)
	jr	$ra



#
# end program
#
terminate:
	li 	$v0, 	10
	syscall



.data
	msg:	.asciiz "Please enter a number: "
	fibo1:	.asciiz	"Fibonacci number "
	fibo2:	.asciiz	" is "
