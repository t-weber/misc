;
; @author Tobias Weber
; @date apr-2021
; @license see 'LICENSE.GPL' file
;

%define WORD_SIZE 4
%define STACK_SIZE 1*WORD_SIZE                    ; size of local stack frame


section .text
global fact_cdecl_asm
extern fact_cdecl_c


;
; calculate factorials
; dword argument on stack
; returns result in eax
;
fact_cdecl_asm:
	push ebp
	mov ebp, esp
	sub esp, STACK_SIZE                       ; create local var stack frame

	;pusha
	push ebx                                  ; save registers that are changed
	push edx


	mov eax, [ebp + WORD_SIZE + WORD_SIZE*1]  ; get number arg
	cmp eax, 1                                ; recursion end condition
	jle .end_le1                              ; num <= 1?

	mov [ebp - WORD_SIZE*1], eax              ; save number as local var
	dec eax                                   ; --num

	mov ebx, [ebp + WORD_SIZE + WORD_SIZE*2]  ; get function selector arg


	push ebx                                  ; push pure_call arg
	push eax                                  ; push num arg

	cmp ebx, 1                                ; pure_call != 1?
	jnz .c_call
		call fact_cdecl_asm               ; call asm version of this function
		jmp .c_call_end
	.c_call:
		call fact_cdecl_c                 ; call c version of this function
	.c_call_end:

	add esp, WORD_SIZE*2                      ; remove args from stack


	mov edx, [ebp - WORD_SIZE*1]              ; get local var
	mul edx                                   ; multiply with result
	jmp .end


	.end_le1:                                 ; num <= 1?
	mov eax, 1

	.end:
	pop edx
	pop ebx                                   ; restore registers that were changed
	;popa

	mov esp, ebp                              ; remove local var stack frame
	pop ebp
	ret


section .data


section .bss
