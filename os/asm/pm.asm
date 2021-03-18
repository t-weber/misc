;
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
%define SCREEN_SIZE      80*20
%define STACK_START      0000_ffffh	; some (arbitrary) stack base address
%define NUM_GDT_ENTRIES  4
%define MBR_BOOTSIG_ADDR $$ + 200h-2	; 510 bytes after section start



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

	jmp code_descr-descr_base_addr : start
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

	call clear

	push dword 0000_1111b	; attrib
	push str1
	call write
	pop eax

	call exit


;
; clear screen
;
clear:
	mov edi, dword CHAROUT
	mov ecx, dword SCREEN_SIZE

	clear_write_loop:
		mov [edi], byte 20h	; store char
		add edi, 2	; next output char index
		dec ecx		; decrement length counter
		cmp ecx, 0
		jz clear_write_loop_end
		jmp clear_write_loop
	clear_write_loop_end:

	ret


;
; write to charout
; @param [esp + WORD_SIZE] pointer to a string
;
write:
	mov esi, [esp + WORD_SIZE]	; argument: char*
	push esi
	call strlen
	mov ecx, eax	; string length
	pop esi

	mov edi, dword CHAROUT
	mov esi, [esp + WORD_SIZE*1]	; char*
	mov dh, [esp + WORD_SIZE*2]	; attrib

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
; strlen
; @param [esp + WORD_SIZE] pointer to a string
; @returns length in eax
;
strlen:
	mov esi, [esp + WORD_SIZE*1]	; argument: string_ptr
	mov edi, esi	; argument: char*

;	count_chars:
;		mov dl, byte [edi]	; dl = *string_ptr
;		cmp dl, 0
;		jz count_chars_end
;		inc ecx		; ++counter
;		inc edi		; ++str_ptr
;		jmp count_chars
;	count_chars_end:
;
;	mov eax, ecx
;	ret

	mov ecx, 0xffff	; max. string length
	xor eax, eax	; look for 0
	repnz scasb

	mov eax, edi	; eax = end_ptr
	sub eax, esi	; eax -= begin_ptr
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
	at sizes_limit16_19, db 1_1_00_1111b	; granularity=1, size=1
	at base24_31, db 00h
iend

stack_descr: istruc descr_struc
	at limit0_15, db 0xff, 0xff
	at base0_23, db 00h, 00h, 00h
	at access, db 1_00_1_0_0_1_0b	; present=1, ring=0, code/data=1, exec=0, expand-down=1, writable=1, accessed=0
	at sizes_limit16_19, db 1_1_00_1111b	; granularity=1, size=1
	at base24_31, db 00h
iend

code_descr: istruc descr_struc
	at limit0_15, db 0xff, 0xff
	at base0_23, db 00h, 00h, 00h
	at access, db 1_00_1_1_0_1_0b	; present=1, ring=0, code/data=1, exec=1, conform=0, readable=1, accessed=0
	at sizes_limit16_19, db 1_1_00_1111b	; granularity=1, size=1
	at base24_31, db 00h
iend


str1: db \
	"--------------------------------------------------------------------------------",\
	"                            In 32-bit protected mode.                           ",\
	"--------------------------------------------------------------------------------",\
	00h
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; MBR, see https://en.wikipedia.org/wiki/Master_boot_record
; -----------------------------------------------------------------------------
mbr_fill: times MBR_BOOTSIG_ADDR - mbr_fill db 0xf4	; fill with hlt
mbr_bootsig: dw 0xaa55	; boot signature
; -----------------------------------------------------------------------------
