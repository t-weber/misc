//
// curdir test
// @author Tobias Weber
// @date 23-aug-19
// @license see 'LICENSE.EUPL' file
//
// @see syscalls, e.g. https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/unistd.h
//
// as -o curdir.o curdir.asm
// ld -o curdir curdir.o
// clang -o curdir curdir.o
//


// =============================================================================
//
// code
//
.text

//.globl _start
.globl main


//_start:
main:
	sub sp, sp, #256	// stack frame
	str x29, [sp, #8]	// frame ptr
	str x30, [sp, #0]	// return addr

	adr x0, strbeg
	bl write

	mov x0, sp
	add x0, x0, 16		// buffer: sp+16
	mov x1, #(256-16)
	bl getcurdir

	mov x0, sp
	add x0, x0, 16		// buffer: sp+16
	bl write

	adr x0, strend
	bl write

	ldr x29, [sp, #8]
	ldr x30, [sp, #0]
	add sp, sp, #256
	br x30			// ret


//
// strlen
// @param x0 string_ptr
// @return length in x7
//
strlen:
	sub sp, sp, #16		// setup stack frame
	str x29, [sp, #8]	// store frame ptr
	str x30, [sp, #0]	// store return addr

	eor x8, x8, x8		// counter = 0

	loop1:
		ldrb w9, [x0, x8]
		cbz w9, loop1_end
		add x8, x8, #1	// ++counter
		b loop1
	loop1_end:

	mov x7, x8		// ret = counter

	ldr x29, [sp, #8]	// restore frame ptr
	ldr x30, [sp, #0]	// restore return addr
	add sp, sp, #16		// remove stack frame
	br x30			// ret


//
// write
// @param x0 string_ptr
//
write:				// stack frame
	sub sp, sp, #32
	str x29, [sp, #8]	// frame ptr
	str x30, [sp, #0]	// return addr

	str x0, [sp, #16]	// locally store string_ptr

	ldr x0, [sp, #16]	// string_ptr
	bl strlen
	str x7, [sp, #24]	// locally store length


	mov x8, sys_write
	mov x0, stdout
	ldr x1, [sp, #16]	// string_ptr
	ldr x2, [sp, #24]	// length
	svc #0x00

	ldr x29, [sp, #8]
	ldr x30, [sp, #0]
	add sp, sp, #32
	br x30			// ret



//
// getcurdir
// @param x0 buffer
// @param x1 length
//
getcurdir:
	sub sp, sp, #16		// setup stack frame
	str x29, [sp, #8]	// store frame ptr
	str x30, [sp, #0]	// store return addr

	mov x8, sys_getcwd
	svc #0x00

	ldr x29, [sp, #8]	// restore frame ptr
	ldr x30, [sp, #0]	// restore return addr
	add sp, sp, #16		// remove stack frame
	br x30			// ret


// =============================================================================



// =============================================================================
//
// constants
//
.data

strbeg:	.asciz "Current directory: \""
strend:	.asciz "\".\n"

stdin 	= 0
stdout 	= 1
stderr 	= 2

sys_write = 0x40
sys_getcwd = 0x11
// =============================================================================
