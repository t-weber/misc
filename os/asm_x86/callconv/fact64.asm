;
; @author Tobias Weber
; @date apr-2021
; @license see 'LICENSE.GPL' file
;
; @see https://en.wikipedia.org/wiki/X86_calling_conventions
;

%define WORD_SIZE 8
%define FLOAT_SIZE 16

section .text

global fact_sysv_asm
global fact_sysv_float_asm
global fact_ms_asm

extern fact_sysv_c
extern fact_sysv_float_c
extern fact_ms_c



;
; calculate factorials
; dword arguments in rdi (number) and rsi (pure_call)
; returns result in rax
;
fact_sysv_asm:
	push rbp
	mov rbp, rsp
	sub rsp, WORD_SIZE*2                      ; create local var stack frame

	;pusha
	push rcx                                  ; save registers that are changed

	mov [rbp - WORD_SIZE*1], rdi              ; save arg 1 (number) as local var
	mov [rbp - WORD_SIZE*2], rsi              ; save arg 2 (pure_call) as local var


	mov rax, [rbp - WORD_SIZE*1]              ; get number arg
	cmp rax, 1                                ; recursion end condition
	jle .end_le1                              ; num <= 1?

	dec rax                                   ; --num
	mov rdi, rax                              ; arg 1 (number)
	mov rsi, [rbp - WORD_SIZE*2]              ; arg 2 (pure call)

	cmp rsi, 1                                ; pure_call != 1?
	jnz .c_call
		call fact_sysv_asm                    ; call asm version of this function
		jmp .c_call_end
	.c_call:
		call fact_sysv_c                      ; call c version of this function
	.c_call_end:


	mov rcx, [rbp - WORD_SIZE*1]              ; get local var
	mul rcx                                   ; multiply with result
	jmp .end

	.end_le1:                                 ; num <= 1?
	mov rax, 1

	.end:
	pop rcx                                   ; restore registers that were changed
	;popa

	mov rsp, rbp                              ; remove local var stack frame
	pop rbp

	;add rsp, WORD_SIZE*0                     ; remove args from stack
	ret WORD_SIZE*0



;
; calculate factorials (float version)
; dword arguments in xmm0 (number) and rdi (pure_call)
; returns result in xmm0
;
fact_sysv_float_asm:
	push rbp
	mov rbp, rsp
	sub rsp, FLOAT_SIZE*2                     ; create local var stack frame

	movsd [rbp - FLOAT_SIZE*1], xmm0          ; save arg 1 (number) as local var
	mov [rbp - FLOAT_SIZE*2], rdi             ; save arg 2 (pure_call) as local var


	movsd xmm1, [rbp - FLOAT_SIZE*1]          ; get number arg
	comisd xmm1, [const_1]                    ; recursion end condition
	jbe .end_le1                              ; num <= 1?

	subsd xmm1, [const_1]                     ; --num
	movsd xmm0, xmm1                          ; arg 1 (number)
	mov rdi, [rbp - FLOAT_SIZE*2]             ; arg 2 (pure call)

	cmp rdi, 1                                ; pure_call != 1?
	jnz .c_call
		call fact_sysv_float_asm              ; call asm version of this function
		jmp .c_call_end
	.c_call:
		call fact_sysv_float_c                ; call c version of this function
	.c_call_end:


	movsd xmm1, [rbp - FLOAT_SIZE*1]          ; get local var
	mulsd xmm0, xmm1                          ; multiply with result
	jmp .end


	.end_le1:                                 ; num <= 1?
	movsd xmm0, [const_1]

	.end:
	mov rsp, rbp                              ; remove local var stack frame
	pop rbp

	;add rsp, FLOAT_SIZE*0                    ; remove args from stack
	ret FLOAT_SIZE*0



;
; calculate factorials
; dword arguments in rcx (number) and rdx (pure_call)
; returns result in rax
;
fact_ms_asm:
	push rbp
	mov rbp, rsp
	sub rsp, WORD_SIZE*2                      ; create local var stack frame

	;pusha
	push rcx                                  ; save registers that are changed
	push rdx

	mov [rbp - WORD_SIZE*1], rcx              ; save arg 1 (number) as local var
	mov [rbp - WORD_SIZE*2], rdx              ; save arg 2 (pure_call) as local var


	mov rax, [rbp - WORD_SIZE*1]              ; get number arg
	cmp rax, 1                                ; recursion end condition
	jle .end_le1                              ; num <= 1?

	dec rax                                   ; --num
	mov rcx, rax                              ; arg 1
	mov rdx, [rbp - WORD_SIZE*2]              ; arg 2

	cmp rdx, 1                                ; pure_call != 1?
	jnz .c_call
		call fact_ms_asm                      ; call asm version of this function
		jmp .c_call_end
	.c_call:
		call fact_ms_c                        ; call c version of this function
	.c_call_end:


	mov rcx, [rbp - WORD_SIZE*1]              ; get local var
	mul rcx                                   ; multiply with result
	jmp .end

	.end_le1:                                 ; num <= 1?
	mov rax, 1

	.end:
	pop rdx
	pop rcx                                   ; restore registers that were changed
	;popa

	mov rsp, rbp                              ; remove local var stack frame
	pop rbp

	;add rsp, WORD_SIZE*0                     ; remove args from stack
	ret WORD_SIZE*0



section .data
const_1: dq 1.0


section .bss
