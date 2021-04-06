;
; @author Tobias Weber
; @date apr-2021
; @license see 'LICENSE.GPL' file
;
; @see https://en.wikipedia.org/wiki/X86_calling_conventions
;

%define WORD_SIZE 4
%define STACK_SIZE 1*WORD_SIZE                    ; size of local stack frame


section .text
global fact_cdecl_asm
global fact_stdcall_asm
global fact_fastcall_asm

extern fact_cdecl_c
extern fact_stdcall_c
extern fact_fastcall_c


;
; calculate factorials
; dword arguments on stack
; returns result in eax
;
fact_cdecl_asm:
	push ebp
	mov ebp, esp
	sub esp, STACK_SIZE                       ; create local var stack frame

	;pusha
	push ecx                                  ; save registers that are changed
	push edx


	mov eax, [ebp + WORD_SIZE + WORD_SIZE*1]  ; get number arg
	cmp eax, 1                                ; recursion end condition
	jle .end_le1                              ; num <= 1?

	mov [ebp - WORD_SIZE*1], eax              ; save number as local var
	dec eax                                   ; --num

	mov ecx, [ebp + WORD_SIZE + WORD_SIZE*2]  ; get function selector arg


	push ecx                                  ; push pure_call arg
	push eax                                  ; push num arg

	cmp ecx, 1                                ; pure_call != 1?
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
	pop ecx                                   ; restore registers that were changed
	;popa

	mov esp, ebp                              ; remove local var stack frame
	pop ebp
	ret



;
; calculate factorials
; dword arguments on stack
; returns result in eax
;
fact_stdcall_asm:
	push ebp
	mov ebp, esp
	sub esp, STACK_SIZE                       ; create local var stack frame

	;pusha
	push ecx                                  ; save registers that are changed
	push edx


	mov eax, [ebp + WORD_SIZE + WORD_SIZE*1]  ; get number arg
	cmp eax, 1                                ; recursion end condition
	jle .end_le1                              ; num <= 1?

	mov [ebp - WORD_SIZE*1], eax              ; save number as local var
	dec eax                                   ; --num

	mov ecx, [ebp + WORD_SIZE + WORD_SIZE*2]  ; get function selector arg


	push ecx                                  ; push pure_call arg
	push eax                                  ; push num arg

	cmp ecx, 1                                ; pure_call != 1?
	jnz .c_call
		call fact_stdcall_asm             ; call asm version of this function
		jmp .c_call_end
	.c_call:
		call fact_stdcall_c               ; call c version of this function
	.c_call_end:


	mov edx, [ebp - WORD_SIZE*1]              ; get local var
	mul edx                                   ; multiply with result
	jmp .end

	.end_le1:                                 ; num <= 1?
	mov eax, 1

	.end:
	pop edx
	pop ecx                                   ; restore registers that were changed
	;popa

	mov esp, ebp                              ; remove local var stack frame
	pop ebp

	;add esp, WORD_SIZE*2                     ; remove args from stack
	ret WORD_SIZE*2



;
; calculate factorials
; dword arguments in ecx (number) and edx (pure_call)
; returns result in eax
;
fact_fastcall_asm:
	push ebp
	mov ebp, esp
	sub esp, WORD_SIZE*3                      ; create local var stack frame

	;pusha
	push ecx                                  ; save registers that are changed
	push edx

	mov [ebp - WORD_SIZE*2], ecx              ; save arg 1 (number) as local var
	mov [ebp - WORD_SIZE*3], edx              ; save arg 2 (pure_call) as local var

	mov eax, [ebp - WORD_SIZE*2]              ; get number arg
	cmp eax, 1                                ; recursion end condition
	jle .end_le1                              ; num <= 1?

	mov [ebp - WORD_SIZE*1], eax              ; save number as local var
	dec eax                                   ; --num

	mov ecx, eax                              ; arg 1
	mov edx, [ebp - WORD_SIZE*3]              ; arg 2

	cmp edx, 1                                ; pure_call != 1?
	jnz .c_call
		call fact_fastcall_asm            ; call asm version of this function
		jmp .c_call_end
	.c_call:
		call fact_fastcall_c              ; call c version of this function
	.c_call_end:


	mov ecx, [ebp - WORD_SIZE*1]              ; get local var
	mul ecx                                   ; multiply with result
	jmp .end

	.end_le1:                                 ; num <= 1?
	mov eax, 1

	.end:
	pop edx
	pop ecx                                   ; restore registers that were changed
	;popa

	mov esp, ebp                              ; remove local var stack frame
	pop ebp

	;add esp, WORD_SIZE*0                     ; remove args from stack
	ret WORD_SIZE*0


section .data


section .bss
