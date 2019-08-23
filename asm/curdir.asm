;
; curdir test
; @author Tobias Weber
; @date 23-aug-19
; @license see 'LICENSE.EUPL' file
;
; @see sycalls, e.g. https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/
;
; yasm -f elf64 -o curdir.o curdir.asm
; ld -o curdir curdir.o
;


global _start


; =============================================================================
;
; code
;
section .text


; -----------------------------------------------------------------------------
_start:
	mov rax, 256
	push rax
	push dir1
	call getcurdir
	pop rax
	pop rax


	;mov qword [i], 10	; using .bss var as counter
	;mov rcx, [i]

	mov qword [rsp-8], 10	; using local stack var as counter
	mov rcx, [rsp-8]


	loop1:
		push rcx		; save counter

		push str1
		call write
		pop rax

		push dir1
		call write
		pop rax

		push str2
		call write
		pop rax

		pop rcx			; restore counter
		dec rcx			; --counter
		cmp rcx, 0
		jnz loop1

	call exit
; -----------------------------------------------------------------------------




; -----------------------------------------------------------------------------
;
; write to stdout
; @param [rsp+8] Pointer to a string
; @see man 2 write
;
write:
	mov rsi, [rsp+8]	; argument: string_ptr
	push rsi
	call strlen
	pop rsi

	mov rax, qword [sys_write]
	mov rsi, [rsp+8]	; sys_write arg: const char*
	mov rdx, rcx		; sys_write arg: string length
	mov rdi, qword [stdout]	; sys_write arg: fd
	syscall

	ret
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
;
; strlen
; @param [rsp+8] Pointer to a string
; @return length in rcx
;
strlen:
	xor rcx, rcx			; counter = 0

	count_chars:
		mov rsi, [rsp+8]	; argument: string_ptr
		add rsi, rcx		; string_ptr += counter
		mov r8b, [rsi]		; r8b = *string_ptr
		cmp r8b, 0x00
		jz count_chars_end
		inc rcx			; ++counter
		jmp count_chars
	count_chars_end:

	ret
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
;
; getcurdir
; @param [rsp+8] Pointer to a buffer
; @param [rsp+16] Length of the buffer
; @see man 2 getcwd
;
getcurdir:
	mov rax, qword [sys_getcwd]
	mov rdi, [rsp+8]	; sys_cwd arg: char *
	mov rsi, [rsp+16]	; sys_cwd arg: buffer length
	syscall

	ret
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
;
; exit
; @see man 2 exit
;
exit:
	mov rax, qword [sys_exit]
	mov rdi, 0x00	; sys_exit arg
	syscall

	ret
; -----------------------------------------------------------------------------

; =============================================================================





; =============================================================================
;
; constants
;
section .data

; syscall numbers
sys_write:	dq 0x01
sys_exit:	dq 0x3c
sys_getcwd:	dq 0x4f

stdin:	dq 0	; see <unistd.h>
stdout:	dq 1	; see <unistd.h>
stderr:	dq 2	; see <unistd.h>

str1:	db "Current directory: ", 0x22, 0x00
str2:	db 0x22, ".", 0x0a, 0x00

; =============================================================================





; =============================================================================
;
; variables
;
section .bss

;i:	resq	1
dir1:	resb	256

; =============================================================================
