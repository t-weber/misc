;
; curdir test
; @author Tobias Weber
; @date 12-sep-20
; @license see 'LICENSE.EUPL' file
;
; @see sycalls, e.g. http://shell-storm.org/shellcode/files/syscalls.html
;
; yasm -f elf32 -o curdir.o curdir32.asm  &&  ld -melf_i386 -o curdir curdir.o
;

%define WORD_SIZE 4
%define dir1_len 256


global _start


; =============================================================================
;
; code
;
section .text


; -----------------------------------------------------------------------------
_start:
	mov eax, dir1_len
	push eax
	push dir1
	call getcurdir
	pop eax
	pop eax

	mov dword [esp - WORD_SIZE], 10	; using local stack var as counter
	mov ecx, [esp - WORD_SIZE]

	loop1:
		push ecx		; save counter

		push str1
		call write
		pop eax

		push dir1
		call write
		pop eax

		push str2
		call write
		pop eax

		pop ecx			; restore counter
		dec ecx			; --counter
		cmp ecx, 0
		jnz loop1

	call exit
; -----------------------------------------------------------------------------




; -----------------------------------------------------------------------------
;
; write to stdout
; @param [esp + WORD_SIZE] Pointer to a string
; @see man 2 write
;
write:
	mov esi, [esp + WORD_SIZE]	; argument: string_ptr
	push esi
	call strlen
	pop esi

	push ebx
	push ecx
	push edx

	mov eax, dword [sys_write]
	mov edx, ecx		; sys_write arg: string length
	mov ecx, [esp + WORD_SIZE*(1+3)]	; sys_write arg: const char*
	mov ebx, dword [stdout]	; sys_write arg: fd
	int 0x80

	pop edx
	pop ecx
	pop ebx
	ret
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
;
; strlen
; @param [esp + WORD_SIZE] Pointer to a string
; @return length in ecx
;
strlen:
	push edx
	xor ecx, ecx			; counter = 0

	count_chars:
		mov esi, [esp + WORD_SIZE*(1+1)]	; argument: string_ptr
		add esi, ecx		; string_ptr += counter
		mov dl, [esi]		; dl = *string_ptr
		cmp dl, 0x00
		jz count_chars_end
		inc ecx			; ++counter
		jmp count_chars
	count_chars_end:

	pop edx
	ret
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
;
; getcurdir
; @param [esp + WORD_SIZE] Pointer to a buffer
; @param [esp + 2*WORD_SIZE] Length of the buffer
; @see man 2 getcwd
;
getcurdir:
	push ebx
	push ecx

	mov eax, dword [sys_getcwd]
	mov ebx, [esp + WORD_SIZE*(1+2)]	; sys_cwd arg: char *
	mov ecx, [esp + WORD_SIZE*(2+2)]	; sys_cwd arg: buffer length
	int 0x80

	pop ecx
	pop ebx
	ret
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
;
; exit
; @see man 2 exit
;
exit:
	mov eax, dword [sys_exit]
	mov ebx, 0x00	; sys_exit arg
	int 0x80

	ret
; -----------------------------------------------------------------------------

; =============================================================================





; =============================================================================
;
; constants
;
section .data

; syscall numbers
sys_write:	dd 0x04
sys_exit:	dd 0x01
sys_getcwd:	dd 0xb7

stdin:	dd 0	; see <unistd.h>
stdout:	dd 1	; see <unistd.h>
stderr:	dd 2	; see <unistd.h>

str1:	db "Current directory: ", 0x22, 0x00
str2:	db 0x22, ".", 0x0a, 0x00

; =============================================================================





; =============================================================================
;
; variables
;
section .bss

dir1:	resb	dir1_len

; =============================================================================
