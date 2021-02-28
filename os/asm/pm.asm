;
; @author Tobias Weber
; @date feb-2021
; @license see 'LICENSE.GPL' file
;
; nasm -w+all -o pm.x86 -f bin pm.asm
; qemu-system-i386 -drive format=raw,file=pm.x86,index=0
;
; references:
;	- https://wiki.osdev.org/Babystep7
;	- https://wiki.osdev.org/Babystep1
;	- https://wiki.osdev.org/Global_Descriptor_Table
;


%define WORD_SIZE	4
%define CHAROUT		000b_8000h	; base address for character output
%define STACK_START	0000_ffffh	; some (arbitrary) stack base address
%define MBR_BOOTSIG_ADDR	$$ + 200h-2	; 510 bytes after section start



; -----------------------------------------------------------------------------
; code in 16-bit real mode
; -----------------------------------------------------------------------------
[bits 16]
[org 7c00h]	; boot code origin, see: https://en.wikipedia.org/wiki/Master_boot_record
	cli
	lgdt [gdtr]

	mov eax, cr0
	or al, 1b		; protection enable bit
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
	mov ax, 0000000000000_0_00b	; segment index 0, gdt, ring=0
	mov ds, ax

	; stack segment
	mov ax, 0000000000001_0_00b	; segment index 1, gdt, ring=0
	mov ss, ax
	mov esp, dword STACK_START

	push str1
	call write
	pop eax

	call exit


;
; write to charout
; @param [esp + WORD_SIZE] pointer to a string
;
write:
	mov esi, [esp + WORD_SIZE]	; argument: char*
	push esi
	call strlen
	pop esi

	push ebx
	push ecx
	push edx

	mov eax, dword CHAROUT
	mov ebx, [esp + WORD_SIZE*(1+3)]	; char*

	write_loop:
		mov dl, byte [ebx]	; dereference char*
		mov [eax], dl	; store char
		add eax, 2	; next output char index
		dec ecx		; decrement length counter
		inc ebx		; increment char pointer
		cmp ecx, 0
		jz write_loop_end
		jmp write_loop
	write_loop_end:

	pop edx
	pop ecx
	pop ebx
	ret


;
; strlen
; @param [esp + WORD_SIZE] pointer to a string
; @returns length in ecx
;
strlen:
	push edx
	xor ecx, ecx		; counter = 0

	count_chars:
		mov esi, [esp + WORD_SIZE*(1+1)]	; argument: string_ptr
		add esi, ecx	; string_ptr += counter
		mov dl, [esi]	; dl = *string_ptr
		cmp dl, 0
		jz count_chars_end
		inc ecx			; ++counter
		jmp count_chars
	count_chars_end:

	pop edx
	ret


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
	at size, dw descr_struc_size*3 - 1	; size of 3 entries - 1
	at offs, dd descr_base_addr
iend

descr_base_addr:
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
