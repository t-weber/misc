;
; calculates factorials in a 512 B bootsector protected mode program
; @author Tobias Weber
; @date feb-2021
; @license see 'LICENSE.GPL' file
;
; nasm -w+all -o pm.x86 -f bin pm.asm
; qemu-system-i386 -drive format=raw,file=pm.x86,index=0
; debugging: qemu-system-x86_64 -d cpu -drive format=raw,file=pm.x86,index=0 2> /tmp/cpu.txt
;
; references:
;	- https://wiki.osdev.org/Babystep7
;	- https://wiki.osdev.org/Babystep1
;	- https://wiki.osdev.org/Global_Descriptor_Table
;


%define WORD_SIZE        4
%define CHAROUT          000b_8000h	; base address for character output
%define SCREEN_ROW_SIZE  25
%define SCREEN_COL_SIZE  80
%define SCREEN_SIZE      SCREEN_ROW_SIZE * SCREEN_COL_SIZE
%define STACK_START      00ff_ffffh	; some (arbitrary) stack base address
;%define STACK_START     000b_8200h		; test: write into screen memory
%define NUM_GDT_ENTRIES  4
%define MBR_BOOTSIG_ADDR $$ + 200h-2	; 510 bytes after section start
%define USE_STRING_OPS	; use optimised string functions

%define NUM_DEC          9	; number of decimals to print
%define MAX_FACT         12	; maximum input number for calculation



; -----------------------------------------------------------------------------
; code in 16-bit real mode
; -----------------------------------------------------------------------------
[bits 16]
[org 7c00h]	; boot code origin, see: https://en.wikipedia.org/wiki/Master_boot_record
	cli
	lgdt [gdtr]

	mov eax, cr0
	or eax, 1b		; protection enable bit
	mov cr0, eax

	jmp code_descr-descr_base_addr : start	; automatically sets cs
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; code in 32-bit protected mode
; -----------------------------------------------------------------------------
[bits 32]
start:
	;sti

	; data segment
	mov ax, 0000000000001_0_00b	; segment index 1, gdt, ring=0
	mov ds, ax
	mov es, ax

	; stack segment
	mov ax, 0000000000010_0_00b	; segment index 2, gdt, ring=0
	mov ss, ax
	mov esp, dword STACK_START

	push dword CHAROUT		; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT		; address to write to
	push dword 1111_0000b	; attrib
	push str_title			; string
	call write_str
	add esp, WORD_SIZE*3	; remove args from stack

	; loop input numbers
	mov dword [esp + 1*WORD_SIZE], 1	; start counter
	input_loop_begin:
		mov eax, dword [esp + 1*WORD_SIZE]	; current counter
		mov edx, SCREEN_COL_SIZE*2
		mul edx
		mov [esp + 2*WORD_SIZE], dword eax	; line offset

		mov edx, dword CHAROUT + SCREEN_COL_SIZE*4		; address to write to
		;mov eax, dword [esp + 2*WORD_SIZE]	; line offset
		add eax, edx
		mov ecx, dword [esp + 1*WORD_SIZE]	; current counter
		push dword eax			; address to write to
		push dword 0000_1111b	; attrib
		push dword ecx			; number to write
		call write_num
		add esp, WORD_SIZE*3	; remove args from stack

		mov edx, dword CHAROUT + SCREEN_COL_SIZE*4 + (NUM_DEC+1)*2		; address to write to
		mov eax, dword [esp + 2*WORD_SIZE]	; line offset
		add eax, edx
		push dword eax			; address to write to
		push dword 0000_1111b	; attrib
		push str_fac			; string to write to
		call write_str
		add esp, WORD_SIZE*3	; remove args from stack

		mov eax, dword [esp + 1*WORD_SIZE]	; current counter
		push dword eax
		call fact
		add esp, WORD_SIZE*1	; remove arg from stack
		mov ecx, eax			; get result

		mov edx, dword CHAROUT + SCREEN_COL_SIZE*4 + (NUM_DEC+4)*2		; address to write to
		mov eax, dword [esp + 2*WORD_SIZE]	; line offset
		add eax, edx
		push dword eax			; address to write to
		push dword 0000_1111b	; attrib
		push ecx				; result from fact
		call write_num
		add esp, WORD_SIZE*3	; remove args from stack

		mov eax, dword [esp + 1*WORD_SIZE]	; current counter
		cmp eax, MAX_FACT
		jge input_loop_end
		inc eax
		mov dword [esp + 1*WORD_SIZE], eax	; current counter
		jmp input_loop_begin
	input_loop_end:

	call exit


;
; calculate factorials
; dword argument on stack
; returns result in eax
;
fact:
	mov eax, [esp + WORD_SIZE*1]	; number
	cmp eax, 2		; recursion end condition
	jle fact_end

	mov edx, eax
	dec eax

	push edx

	push eax
	call fact
	add esp, WORD_SIZE*1	; remove arg from stack

	pop edx
	mul edx

	fact_end:
	ret


;
; clear screen
;
clear:
	mov ecx, [esp + WORD_SIZE]		; size
	mov edi, [esp + WORD_SIZE*2]	; base address

%ifdef USE_STRING_OPS
	mov ax, word 00_00h
	rep stosw

%else
	clear_write_loop:
		mov [edi], word 00_00h	; store char
		add edi, 2	; next output char index
		dec ecx		; decrement length counter
		cmp ecx, 0
		jz clear_write_loop_end
		jmp clear_write_loop
	clear_write_loop_end:
%endif

	ret



;
; write a string to charout
; @param [esp + WORD_SIZE] pointer to a string
;
write_str:
	mov eax, [esp + WORD_SIZE]	; argument: char*
	push eax
	call strlen
	mov ecx, eax	; string length
	add esp, WORD_SIZE	; remove arg from stack

	mov esi, [esp + WORD_SIZE*1]	; char*
	mov dh, [esp + WORD_SIZE*2]		; attrib
	mov edi, [esp + WORD_SIZE*3]	; base address

	write_loop:
		mov dl, byte [esi]	; dereference char*
		mov [edi], dl	; store char
		inc edi		; to char attribute index
		mov [edi], dh	; store char attributes
		inc edi		; next output char index
		dec ecx		; decrement length counter
		inc esi		; increment char pointer
		cmp ecx, 0
		jz write_loop_end
		jmp write_loop
	write_loop_end:

	ret


;
; write a decimal number to charout
; @param [esp + WORD_SIZE] pointer to a string
;
write_num:
	mov edi, [esp + WORD_SIZE*3]	; base address

	mov ecx, NUM_DEC	; number of decimals to print
	mov eax, ecx
	shl eax, 1
	add edi, eax	; write from the end to the beginning
	inc edi		; start with char attrib

	mov eax, [esp + WORD_SIZE*1]	; number

	num_write_loop:
		push ecx
		xor edx, edx
		mov ecx, dword 10
		div ecx		; div result -> eax, mod result -> edx
		pop ecx

		mov dh, [esp + WORD_SIZE*2]		; attrib
		mov [edi], dh	; store char attributes
		dec edi		; next output char index

		add dl, '0'		; convert to ascii char
		mov [edi], dl	; store char
		dec edi		; to char attribute index

		dec ecx		; decrement length counter
		cmp ecx, 0
		jz num_write_loop_end
		jmp num_write_loop
	num_write_loop_end:

	ret


;
; strlen
; @param [esp + WORD_SIZE] pointer to a string
; @returns length in eax
;
strlen:
	mov esi, [esp + WORD_SIZE*1]	; argument: string_ptr
	mov edi, esi	; argument: char*

%ifdef USE_STRING_OPS
	mov ecx, 0xffff	; max. string length
	xor eax, eax	; look for 0
	repnz scasb

	mov eax, edi	; eax = end_ptr
	sub eax, esi	; eax -= begin_ptr
	sub eax, 1

%else
	count_chars:
		mov dl, byte [edi]	; dl = *string_ptr
		cmp dl, 0
		jz count_chars_end
		inc ecx		; ++counter
		inc edi		; ++str_ptr
		jmp count_chars
	count_chars_end:

	mov eax, ecx
%endif

	ret


;
; halt
;
exit:
	hlt
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; data
; -----------------------------------------------------------------------------
; see https://wiki.osdev.org/Global_Descriptor_Table
; see https://en.wikipedia.org/wiki/Global_Descriptor_Table

; gdt table description
struc gdtr_struc
	size resw 1
	offs resd 1
endstruc

; gdt table entries
struc descr_struc
	limit0_15 resb 2
	base0_23 resb 3
	access resb 1
	sizes_limit16_19 resb 1
	base24_31 resb 1
endstruc


; see https://wiki.osdev.org/Babystep7
gdtr: istruc gdtr_struc
	at size, dw descr_struc_size*NUM_GDT_ENTRIES - 1	; size - 1
	at offs, dd descr_base_addr
iend

descr_base_addr:
	times descr_struc_size db 0	; null descriptor

data_descr: istruc descr_struc
	at limit0_15, db 0xff, 0xff
	at base0_23, db 00h, 00h, 00h
	at access, db 1_00_1_0_0_1_0b	; present=1, ring=0, code/data=1, exec=0, expand-down=0, writable=1, accessed=0
	at sizes_limit16_19, db 1_1_00_1111b	; granularity=1, 32bit=1
	at base24_31, db 00h
iend

stack_descr: istruc descr_struc
	at limit0_15, db 0x00, 0x00
	at base0_23, db 0x00, 0x00, 0x00
	at access, db 1_00_1_0_1_1_0b	; present=1, ring=0, code/data=1, exec=0, expand-down=1, writable=1, accessed=0
	at sizes_limit16_19, db 1_1_00_0000b	; granularity=1, 32bit=1
	at base24_31, db 0x00
iend

code_descr: istruc descr_struc
	at limit0_15, db 0xff, 0xff
	at base0_23, db 00h, 00h, 00h
	at access, db 1_00_1_1_0_1_0b	; present=1, ring=0, code/data=1, exec=1, conform=0, readable=1, accessed=0
	at sizes_limit16_19, db 1_1_00_1111b	; granularity=1, 32bit=1
	at base24_31, db 00h
iend


str_title: db \
	"                      Protected Mode Factorial Calculation                      ",\
	00h

str_fac: db "! = ", 00h
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; MBR, see https://en.wikipedia.org/wiki/Master_boot_record
; -----------------------------------------------------------------------------
mbr_fill: times MBR_BOOTSIG_ADDR - mbr_fill db 0xf4	; fill with hlt
mbr_bootsig: dw 0xaa55	; boot signature at end of boot sector
; -----------------------------------------------------------------------------
