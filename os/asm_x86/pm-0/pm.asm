;
; calculates factorials in a protected mode program
; @author Tobias Weber
; @date feb-2021
; @license see 'LICENSE.GPL' file
;
; nasm -w+all -o pm.x86 -f bin pm.asm
; qemu-system-i386 -drive format=raw,file=pm.x86,index=0
; bochs "floppya: 1_44=pm.x86, status=inserted" "boot:floppy"
; debugging: qemu-system-x86_64 -d cpu -drive format=raw,file=pm.x86,index=0 2> /tmp/cpu.txt
;
; references:
;	- https://wiki.osdev.org/Babystep7
;	- https://wiki.osdev.org/Babystep1
;	- https://wiki.osdev.org/Global_Descriptor_Table
;	- "Writing a simple operating system -- from scratch" by N. Blundell, 2010
;	- https://en.wikipedia.org/wiki/X86_calling_conventions
;


%define WORD_SIZE        4
%define WORD_SIZE_16     2

%define CHAROUT          000b_8000h ; base address for character output
%define SCREEN_ROW_SIZE  25
%define SCREEN_COL_SIZE  80
%define SCREEN_SIZE      SCREEN_ROW_SIZE * SCREEN_COL_SIZE

; see: https://wiki.osdev.org/Memory_Map_(x86) and https://en.wikipedia.org/wiki/Master_boot_record
%define BOOTSECT_START   7c00h
%define STACK_START_16   7fffh      ; some (arbitrary) stack base address
%define STACK_START      003f_ffffh ; some (arbitrary) stack base address
;%define STACK_START     000b_8200h ; test: write into screen memory
%define NUM_GDT_ENTRIES  4

%define SECTOR_SIZE      200h
%define MBR_BOOTSIG_ADDR $$ + SECTOR_SIZE-2 ; 510 bytes after section start
%define FILL_BYTE        0xf4               ; fill with hlt

%define USE_STRING_OPS      ; use optimised string functions
%define NUM_DEC          9  ; number of decimals to print
%define MAX_FACT         12 ; maximum input number for calculation

%define ATTR_BOLD        0000_1111b
%define ATTR_INV         1111_0000b


; -----------------------------------------------------------------------------
; code in 16-bit real mode
; in boot sector
; -----------------------------------------------------------------------------
[bits 16]
[org BOOTSECT_START]
	mov sp, STACK_START_16
	mov bp, STACK_START_16

	; clear screen
	call clear_16

	; write start message
	push word ATTR_BOLD     ; attrib
	push word str_start_16	; string
	call write_str_16
	add sp, WORD_SIZE_16*2  ; remove args from stack
	;stop_16: jmp stop_16

	; load gdt register
	cli
	lgdt [gdtr]

	; set protection enable bit
	mov eax, cr0
	or eax, 1b
	mov cr0, eax

	; go to 32 bit code
	jmp code_descr-descr_base_addr : start32	; automatically sets cs


;
; clear screen
;
clear_16:
	mov ax, word CHAROUT / 16    ; base address segment
	mov es, ax
	mov di, word 00h             ; base address

	mov cx, SCREEN_ROW_SIZE*SCREEN_COL_SIZE*2

	clear_write_loop_16:
		mov [es:di], word 00_00h ; store char
		add di, 2                ; next output char index
		dec cx                   ; decrement length counter
		cmp cx, 0
		jnz clear_write_loop_16

	ret


;
; write a string to charout
; @param [sp + WORD_SIZE_16] pointer to a string
;
write_str_16:
	push bp
	mov bp, sp

	xor ax, ax
	mov ds, ax                   ; data segment for char*
	mov si, [bp + WORD_SIZE_16*2] ; char*
	mov dh, [bp + WORD_SIZE_16*3] ; attrib

	mov ax, CHAROUT / 16         ; base address segment
	mov es, ax
	mov di, word 00h             ; base address

	write_loop_16:
		mov dl, byte [ds:si]     ; dereference char*
		cmp dl, 0h               ; string end 0?
		jz write_loop_end_16
		mov [es:di], dl          ; store char
		inc di                   ; to char attribute index
		mov [es:di], dh          ; store char attributes
		inc di                   ; next output char index
		inc si                   ; increment char pointer
		jmp write_loop_16
	write_loop_end_16:

	mov sp, bp
	pop bp
	ret

; -----------------------------------------------------------------------------




; -----------------------------------------------------------------------------
; code in 32-bit protected mode
; starting in second 512B sector
; -----------------------------------------------------------------------------
[bits 32]
start32:
	; data segment
	mov ax, 0000000000001_0_00b            ; segment index 1, gdt, ring=0
	mov ds, ax
	mov es, ax

	; stack segment
	mov ax, 0000000000010_0_00b            ; segment index 2, gdt, ring=0
	mov ss, ax
	mov esp, dword STACK_START

	push dword CHAROUT                     ; address to write to
	push dword SCREEN_SIZE                 ; number of characters to write
	call clear
	add esp, WORD_SIZE*2                   ; remove args from stack

	push dword CHAROUT+35*2                ; address to write to
	push dword ATTR_INV                    ; attrib
	push str_title                         ; string
	call write_str
	add esp, WORD_SIZE*3                   ; remove args from stack

	; loop input numbers
	mov dword [esp + 1*WORD_SIZE], 1       ; start counter
	input_loop_begin:
		mov eax, dword [esp + 1*WORD_SIZE] ; current counter
		mov edx, SCREEN_COL_SIZE*2
		mul edx
		mov [esp + 2*WORD_SIZE], dword eax ; line offset

		mov edx, dword CHAROUT + SCREEN_COL_SIZE*4  ; address to write to
		;mov eax, dword [esp + 2*WORD_SIZE]         ; line offset
		add eax, edx
		mov ecx, dword [esp + 1*WORD_SIZE] ; current counter
		push dword eax                     ; address to write to
		push dword ATTR_BOLD              ; attrib
		push dword ecx                     ; number to write
		call write_num
		add esp, WORD_SIZE*3               ; remove args from stack

		mov edx, dword CHAROUT + SCREEN_COL_SIZE*4 + (NUM_DEC+1)*2  ; address to write to
		mov eax, dword [esp + 2*WORD_SIZE] ; line offset
		add eax, edx
		push dword eax                     ; address to write to
		push dword ATTR_BOLD              ; attrib
		push str_fac                       ; string to write to
		call write_str
		add esp, WORD_SIZE*3               ; remove args from stack

		mov eax, dword [esp + 1*WORD_SIZE] ; current counter
		push dword eax
		call fact
		add esp, WORD_SIZE*1               ; remove arg from stack
		mov ecx, eax                       ; get result

		mov edx, dword CHAROUT + SCREEN_COL_SIZE*4 + (NUM_DEC+4)*2  ; address to write to
		mov eax, dword [esp + 2*WORD_SIZE] ; line offset
		add eax, edx
		push dword eax                     ; address to write to
		push dword ATTR_BOLD              ; attrib
		push ecx                           ; result from fact
		call write_num
		add esp, WORD_SIZE*3               ; remove args from stack

		mov eax, dword [esp + 1*WORD_SIZE] ; current counter
		cmp eax, MAX_FACT
		jge input_loop_end
		inc eax
		mov dword [esp + 1*WORD_SIZE], eax ; current counter
		jmp input_loop_begin
	input_loop_end:

	call exit



;
; calculate factorials
; dword argument on stack
; returns result in eax
;
fact:
	push ebp
	mov ebp, esp
	sub esp, WORD_SIZE*1

	mov eax, [ebp + WORD_SIZE*2]  ; get arg
	cmp eax, 2                    ; recursion end condition
	jle fact_end

	mov [ebp - WORD_SIZE*1], eax  ; save number as local var
	dec eax

	push eax
	call fact
	add esp, WORD_SIZE*1          ; remove arg from stack

	mov edx, [ebp - WORD_SIZE*1]
	mul edx

	fact_end:
	mov esp, ebp
	pop ebp
	ret



;
; clear screen
;
clear:
	;push ebp
	;mov ebp, esp

	;mov ecx, [ebp + WORD_SIZE*2]  ; size
	;mov edi, [ebp + WORD_SIZE*3]  ; base address
	mov ecx, [esp + WORD_SIZE*1]  ; size
	mov edi, [esp + WORD_SIZE*2]  ; base address

%ifdef USE_STRING_OPS
	mov ax, word 00_00h
	rep stosw

%else
	clear_write_loop:
		mov [edi], word 00_00h    ; store char
		add edi, 2                ; next output char index
		dec ecx                   ; decrement length counter
		cmp ecx, 0
		jz clear_write_loop_end
		jmp clear_write_loop
	clear_write_loop_end:
%endif

	;mov esp, ebp
	;pop ebp
	ret



;
; write a string to charout
; @param [esp + WORD_SIZE] pointer to a string
;
write_str:
	;push ebp
	;mov ebp, esp

	;mov esi, [ebp + WORD_SIZE*2]  ; char*
	;mov dh, byte [ebp + WORD_SIZE*3] ; attrib
	;mov edi, [ebp + WORD_SIZE*4]  ; base address
	mov esi, [esp + WORD_SIZE*1]  ; char*
	mov dh, byte [esp + WORD_SIZE*2] ; attrib
	mov edi, [esp + WORD_SIZE*3]  ; base address

	write_loop:
		mov dl, byte [esi]        ; dereference char*
		cmp dl, 0h                ; string end 0?
		jz write_loop_end
		mov [edi], dl             ; store char
		inc edi                   ; to char attribute index
		mov [edi], dh             ; store char attributes
		inc edi                   ; next output char index
		inc esi                   ; increment char pointer
		jmp write_loop
	write_loop_end:

	;mov esp, ebp
	;pop ebp
	ret



;
; write a decimal number to charout
; @param [esp + WORD_SIZE] pointer to a string
;
write_num:
	;push ebp
	;mov ebp, esp

	;mov edi, [ebp + WORD_SIZE*4]  ; base address
	mov edi, [esp + WORD_SIZE*3]  ; base address

	mov ecx, NUM_DEC              ; number of decimals to print
	mov eax, ecx
	shl eax, 1
	add edi, eax                  ; write from the end to the beginning
	inc edi                       ; start with char attrib

	;mov eax, [ebp + WORD_SIZE*2]  ; number
	mov eax, [esp + WORD_SIZE*1]  ; number

	num_write_loop:
		push ecx
		xor edx, edx
		mov ecx, dword 10
		div ecx                   ; div result -> eax, mod result -> edx
		pop ecx

		;mov dh, byte [ebp + WORD_SIZE*3] ; attrib
		mov dh, byte [esp + WORD_SIZE*2] ; attrib
		mov [edi], dh             ; store char attributes
		dec edi                   ; next output char index

		add dl, '0'               ; convert to ascii char
		mov [edi], dl             ; store char
		dec edi                   ; to char attribute index

		dec ecx                   ; decrement length counter
		cmp ecx, 0
		jnz num_write_loop

	;mov esp, ebp
	;pop ebp
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
	at size, dw descr_struc_size*NUM_GDT_ENTRIES - 1  ; size - 1
	at offs, dd descr_base_addr
iend

descr_base_addr:
	times descr_struc_size db 0	; null descriptor

data_descr: istruc descr_struc
	at limit0_15, db 0xff, 0xff
	at base0_23, db 00h, 00h, 00h
	at access, db 1_00_1_0_0_1_0b          ; present=1, ring=0, code/data=1, exec=0, expand-down=0, writable=1, accessed=0
	at sizes_limit16_19, db 1_1_00_1111b   ; granularity=1, 32bit=1
	at base24_31, db 00h
iend

stack_descr: istruc descr_struc
	at limit0_15, db 0x00, 0x00
	at base0_23, db 00h, 00h, 00h
	at access, db 1_00_1_0_1_1_0b          ; present=1, ring=0, code/data=1, exec=0, expand-down=1, writable=1, accessed=0
	at sizes_limit16_19, db 1_1_00_0000b   ; granularity=1, 32bit=1
	at base24_31, db 00h
iend

code_descr: istruc descr_struc
	at limit0_15, db 0xff, 0xff
	at base0_23, db 00h, 00h, 00h
	at access, db 1_00_1_1_0_1_0b          ; present=1, ring=0, code/data=1, exec=1, conform=0, readable=1, accessed=0
	at sizes_limit16_19, db 1_1_00_1111b   ; granularity=1, 32bit=1
	at base24_31, db 00h
iend


str_start_16: db "Starting", 00h
str_title: db "Factorials", 00h
str_fac: db "! = ", 00h
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; MBR, see https://en.wikipedia.org/wiki/Master_boot_record
; -----------------------------------------------------------------------------
mbr_fill: times MBR_BOOTSIG_ADDR - mbr_fill db FILL_BYTE
mbr_bootsig: dw 0xaa55	; boot signature at end of boot sector
; -----------------------------------------------------------------------------
; end of 512B bootsector
; -----------------------------------------------------------------------------
