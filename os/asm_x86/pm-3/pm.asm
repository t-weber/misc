;
; @author Tobias Weber
; @date feb-2021
; @license see 'LICENSE.GPL' file
;
; references:
;	- https://wiki.osdev.org/Babystep7
;	- https://wiki.osdev.org/Babystep1
;	- https://wiki.osdev.org/Global_Descriptor_Table
;	- "Writing a simple operating system -- from scratch" by N. Blundell, 2010
;


%define WORD_SIZE         4
%define WORD_SIZE_16      2

%define CR0_PROTECT_BIT   0
%define CR0_PAGING_BIT    31
%define CR4_PAGESIZE_BIT  4
%define CR4_ADDREXT_BIT   5
%define USE_PAGING        1

; see https://jbwyatt.com/253/emu/memory.html
%define CHAROUT           000b_8000h ; base address for character output
%define SCREEN_ROW_SIZE   25
%define SCREEN_COL_SIZE   80
%define SCREEN_SIZE       SCREEN_ROW_SIZE * SCREEN_COL_SIZE
%define ATTR_BOLD         0000_1111b

; see https://wiki.osdev.org/Memory_Map_(x86) and https://en.wikipedia.org/wiki/Master_boot_record
%define BOOTSECT_START    7c00h
%define STACK_START_16    7fffh      ; some (arbitrary) stack base address
%define STACK_START       07f_ffffh  ; some (arbitrary) stack base address below 8MB

%define NUM_GDT_ENTRIES   4
%define NUM_IDT_ENTRIES   256

%define SECTOR_SIZE       200h
%define MBR_BOOTSIG_ADDR  ($$ + SECTOR_SIZE - 2) ; 510 bytes after section start
%define NUM_ASM_SECTORS   8          ; total number of sectors  ceil("pm.x86" file size / SECTOR_SIZE)
%define NUM_C_SECTORS     11         ; number of sectors for c code
%define END_ADDR          ($$ + SECTOR_SIZE * NUM_ASM_SECTORS)
%define FILL_BYTE         0xf4       ; fill with hlt

; see https://en.wikipedia.org/wiki/INT_13H
%define BIOS_FLOPPY0      0x00
%define BIOS_HDD0         0x80

; see https://wiki.osdev.org/PIC
%define PIC0_INSTR_PORT   0x20
%define PIC1_INSTR_PORT   0xa0
%define PIC0_DATA_PORT    0x21
%define PIC1_DATA_PORT    0xa1
%define PIC0_INT_OFFS     0x20
%define PIC1_INT_OFFS     0x28

; see https://wiki.osdev.org/Programmable_Interval_Timer
%define TIMER_INSTR_PORT  0x43
%define TIMER_DATA_PORT   0x40
%define TIMER_VAL         0x00ff

; https://wiki.osdev.org/%228042%22_PS/2_Controller
%define KEYB_DATA_PORT    0x60


; -----------------------------------------------------------------------------
; code in 16-bit real mode
; in boot sector
; -----------------------------------------------------------------------------
[bits 16]
;[org BOOTSECT_START]	; not needed, set by linker
	mov sp, STACK_START_16
	mov bp, STACK_START_16
	push dx

	; clear screen
	call clear_16

	; write start message
	push word ATTR_BOLD	; attrib
	push word str_start_16	; string
	call write_str_16
	pop ax	; remove args from stack
	pop ax	; remove args from stack
	;stop_16: jmp stop_16

	; read all needed sectors
	; see https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=02h:_Read_Sectors_From_Drive
	pop dx
	;xor ax, ax
	mov ax, cs
	mov es, ax
	mov ah, 02h       ; func
	mov al, byte (NUM_ASM_SECTORS + NUM_C_SECTORS - 1) ; sector count (boot sector already loaded)
	mov bx, word start_32 ; destination address es:bx
	mov cx, 00_02h    ; C = 0, S = 2
	mov dh, 00h       ; H = 0
	;mov dl, BIOS_HDD0 ; drive
	clc
	int 13h	; sets carry flag on failure
	jnc load_sectors_succeeded_16
		; write error message on failure
		push word ATTR_BOLD	; attrib
		push word str_load_error_16	; string
		call write_str_16
		pop ax	; remove args from stack
		pop ax	; remove args from stack
		call exit_16
	load_sectors_succeeded_16:

	; disable interrupts
	cli

	; configure pics
	call config_pics_16

	; load gdt register
	lgdt [gdtr]

	; set protection enable bit
	mov eax, cr0
	or eax, 1b << CR0_PROTECT_BIT
	mov cr0, eax

	; go to 32 bit code
	jmp code_descr-gdt_base_addr : start_32	; automatically sets cs



;
; configure pics
; see https://wiki.osdev.org/8259_PIC
; see https://www.eeeguide.com/8259-programmable-interrupt-controller
;
config_pics_16:
	; primary pic
	mov al, 000_1_0_0_0_1b   ; command 1
	out PIC0_INSTR_PORT, al
	mov al, PIC0_INT_OFFS    ; command 2, offset for interrupts
	out PIC0_DATA_PORT, al
	mov al, 0000_0100b       ; command 3, secondary pic at irq 2
	out PIC0_DATA_PORT, al
	mov al, 000_0_00_1_1b    ; command 4
	out PIC0_DATA_PORT, al
	mov al, 1111_1000b       ; enable timer (irq 0), keyboard (irq 1) and secondary pic (irq 2)
	out PIC0_DATA_PORT, al

	; secondary pic
	mov al, 000_1_0_0_0_1b   ; command 1
	out PIC1_INSTR_PORT, al
	mov al, PIC1_INT_OFFS    ; command 2, offset for interrupts
	out PIC1_DATA_PORT, al
	mov al, 00000_000b       ; command 3, secondary pic with id=0
	out PIC1_DATA_PORT, al
	mov al, 000_0_00_1_1b    ; command 4
	out PIC1_DATA_PORT, al
	mov al, 1111_1111b       ; disable all hardware interrupts
	;mov al, 1111_1110b      ; disable all hardware interrupts except rtc (irq 8)
	out PIC1_DATA_PORT, al

	; start timer, see https://wiki.osdev.org/Programmable_Interval_Timer
	; set frequency divider lower and upper byte
	mov al, byte TIMER_VAL
	out TIMER_DATA_PORT, al
	mov al, byte TIMER_VAL >> 8
	out TIMER_DATA_PORT, al
	; start timer
	mov al, 00_00_000_0b
	out TIMER_INSTR_PORT, al

	ret



;
; halt
;
exit_16:
	;cli
	hlt_loop_16: hlt	; loop, because hlt can be interrupted
	jmp hlt_loop_16



;
; clear screen
;
clear_16:
	mov ax, word CHAROUT / 16	; base address segment
	mov es, ax
	mov di, word 00h	; base address

	mov cx, SCREEN_ROW_SIZE*SCREEN_COL_SIZE*2

	clear_write_loop_16:
		mov [es:di], word 00_00h	; store char
		add di, 2	; next output char index
		dec cx		; decrement length counter
		cmp cx, 0
		jz clear_write_loop_end_16
		jmp clear_write_loop_16
	clear_write_loop_end_16:

	ret


;
; write a string to charout
; @param [sp + WORD_SIZE_16] pointer to a string
;
write_str_16:
	mov bp, sp

	mov ax, [bp + WORD_SIZE_16]	; argument: char*
	push ax
	call strlen_16
	mov cx, ax	; string length
	pop ax		; remove arg from stack

	mov bp, sp

	xor ax, ax
	mov ds, ax	; data segment for char*
	mov si, [bp + WORD_SIZE_16*1]	; char*
	mov dh, [bp + WORD_SIZE_16*2]	; attrib

	mov ax, CHAROUT / 16	; base address segment
	mov es, ax
	mov di, word 00h	; base address

	write_loop_16:
		mov dl, byte [ds:si]	; dereference char*
		mov [es:di], dl	; store char
		inc di		; to char attribute index
		mov [es:di], dh	; store char attributes
		inc di		; next output char index
		dec cx		; decrement length counter
		inc si		; increment char pointer
		cmp cx, 0
		jz write_loop_end_16
		jmp write_loop_16
	write_loop_end_16:

	ret


;
; strlen_16
; @param [sp + WORD_SIZE_16] pointer to a string
; @returns length in eax
;
strlen_16:
	mov bp, sp

	xor ax, ax
	mov ds, ax	; data segment for char*
	mov si, word [bp + WORD_SIZE_16*1]	; argument: char*

	xor cx, cx
	count_chars_16:
		mov dl, byte [ds:si]	; dl = *str_ptr
		cmp dl, 0
		jz count_chars_end_16
		inc cx		; ++counter
		inc si		; ++str_ptr
		jmp count_chars_16
	count_chars_end_16:

	mov ax, cx
	ret
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; data
; -----------------------------------------------------------------------------
; see https://wiki.osdev.org/Global_Descriptor_Table
; see https://en.wikipedia.org/wiki/Global_Descriptor_Table

; gdt table description
struc gdtr_struc
	gdt_size resw 1
	gdt_offs resd 1
endstruc

; gdt table entries
struc gdt_struc
	gdt_limit0_15 resb 2
	gdt_base0_23 resb 3
	gdt_access resb 1
	gdt_sizes_limit16_19 resb 1
	gdt_base24_31 resb 1
endstruc


; see https://wiki.osdev.org/Babystep7
gdtr: istruc gdtr_struc
	at gdt_size, dw gdt_struc_size*NUM_GDT_ENTRIES - 1	; size - 1
	at gdt_offs, dd gdt_base_addr
iend

gdt_base_addr:
	times gdt_struc_size db 0	; null descriptor

data_descr: istruc gdt_struc
	at gdt_limit0_15, db 0xff, 0xff
	at gdt_base0_23, db 00h, 00h, 00h
	at gdt_access, db 1_00_1_0_0_1_0b        ; present=1, ring=0, code/data=1, exec=0, expand-down=0, writable=1, accessed=0
	at gdt_sizes_limit16_19, db 1_1_00_1111b ; granularity=1, 32bit=1
	at gdt_base24_31, db 00h
iend

stack_descr: istruc gdt_struc
	at gdt_limit0_15, db 0x00, 0x00
	at gdt_base0_23, db 00h, 00h, 00h
	at gdt_access, db 1_00_1_0_1_1_0b        ; present=1, ring=0, code/data=1, exec=0, expand-down=1, writable=1, accessed=0
	at gdt_sizes_limit16_19, db 1_1_00_0000b ; granularity=1, 32bit=1
	at gdt_base24_31, db 00h
iend

code_descr: istruc gdt_struc
	at gdt_limit0_15, db 0xff, 0xff
	at gdt_base0_23, db 00h, 00h, 00h
	at gdt_access, db 1_00_1_1_0_1_0b        ; present=1, ring=0, code/data=1, exec=1, conform=0, readable=1, accessed=0
	at gdt_sizes_limit16_19, db 1_1_00_1111b ; granularity=1, 32bit=1
	at gdt_base24_31, db 00h
iend


str_start_16: db "Starting 16bit...", 00h
str_load_error_16: db "Error loading sectors.", 00h
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; MBR, see https://en.wikipedia.org/wiki/Master_boot_record
; -----------------------------------------------------------------------------
mbr_fill: times MBR_BOOTSIG_ADDR - mbr_fill db FILL_BYTE
mbr_bootsig: dw 0xaa55	; boot signature at end of boot sector
; -----------------------------------------------------------------------------
; end of 512B bootsector
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; code in 32-bit protected mode
; starting in second 512B sector
; -----------------------------------------------------------------------------
[bits 32]
start_32:
	; data segment
	mov ax, data_descr-gdt_base_addr
	mov ds, ax
	mov es, ax

	; stack segment
	mov ax, stack_descr-gdt_base_addr
	mov ss, ax
	mov esp, dword STACK_START
	mov ebp, dword STACK_START

%ifdef USE_PAGING
	mov eax, (1b << 12) ; page directory start address at 4MB
	mov edi, eax
	;mov ecx, 1024      ; number of page directory entries
	mov ecx, 2          ; number of page directory entries
	mov ebx, 0          ; current page frame address
	ptd_loop:
		; size=1, dirty=0, accessed=0, cache=0, write through=0, supervisor=1, write=1, present=1
		mov [edi + ptd4_flags], byte 1000_0111b
		mov [edi + ptd4_offs32_34_flags8_12], byte 000_0_000_0b  ; addr table=0, global=0
		mov edx, ebx
		;and edx, 11b
		shl edx, 6  ; lower two bits of base address
		mov [edi + ptd4_offs22_23_offs35_40], byte dl
		mov edx, ebx
		shr edx, 2  ; next bits of base address
		mov [edi + ptd4_offs24_31], byte dl

		add ebx, 1   ; page frame size: 2^12
		add edi, ptd4_struc_size
		dec ecx
		cmp ecx, 0
		jz ptd_loop_end
		jmp ptd_loop
	ptd_loop_end:
	mov cr3, eax        ; page table address

	mov eax, cr4
	or eax, 1b << CR4_PAGESIZE_BIT
	mov cr4, eax        ; set page size extension

	mov eax, cr0
	or eax, 1b << CR0_PAGING_BIT
	mov cr0, eax        ; enable paging
%endif

	; load idt register and enable interrupts
	lidt [idtr]
	sti

	;push dword CHAROUT	; address to write to
	;push dword SCREEN_SIZE	; number of characters to write
	;call clear_32
	;add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_start_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack
	;stop_32: jmp stop_32

	[extern entrypoint]
	call entrypoint

	call exit_32


;
; clear screen
;
clear_32:
	mov ecx, [esp + WORD_SIZE]		; size
	mov edi, [esp + WORD_SIZE*2]	; base address

	mov ax, word 00_00h
	rep stosw

	ret



;
; write a string to charout
; @param [esp + WORD_SIZE] pointer to a string
; @param [esp + WORD_SIZE*2] attributes
; @param [esp + WORD_SIZE*3] base address to write to
;
write_str_32:
	mov eax, [esp + WORD_SIZE]	; argument: char*
	push eax
	call strlen_32
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
; strlen
; @param [esp + WORD_SIZE] pointer to a string
; @returns length in eax
;
strlen_32:
	mov esi, [esp + WORD_SIZE*1]	; argument: char*
	mov edi, esi	; argument: char*

	mov ecx, 0xffff	; max. string length
	xor eax, eax	; look for 0
	repnz scasb

	mov eax, edi	; eax = end_ptr
	sub eax, esi	; eax -= begin_ptr
	sub eax, 1

	ret


;
; halt
;
exit_32:
	;cli
	hlt_loop: hlt	; loop, because hlt can be interrupted
	jmp hlt_loop
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; interrupt service routines
; see https://wiki.osdev.org/Interrupt_Service_Routines and https://wiki.osdev.org/Interrupts
; -----------------------------------------------------------------------------

;
; dummy isr
;
isr_null:
	iret


; see https://wiki.osdev.org/%228042%22_PS/2_Controller
isr_keyb:
	pushad

	xor eax, eax
	in al, KEYB_DATA_PORT
	push eax
	[extern keyb_event]
	call keyb_event
	add esp, WORD_SIZE

	popad
	iret


isr_timer:
	pushad

	[extern timer_event]
	call timer_event

	popad
	iret


isr_rtc:
	pushad

	[extern rtc_event]
	call rtc_event

	popad
	iret


isr_div0:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_div0_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_overflow:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_overflow_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_bounds:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_bounds_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_instr:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_instr_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_dev:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_dev_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_tssfault:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_tssfault_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_segfault_notavail:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_segfault_notavail_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_segfault_stack:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_segfault_stack_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_pagefault:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_pagefault_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_df:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_df_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_overrun:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_overrun_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_gpf:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_gpf_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32


isr_fpu:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_fpu_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32

isr_align:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_align_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32

isr_mce:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_mce_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32

isr_simd:
	push dword CHAROUT	; address to write to
	push dword SCREEN_SIZE	; number of characters to write
	call clear_32
	add esp, WORD_SIZE*2	; remove args from stack

	push dword CHAROUT + SCREEN_COL_SIZE*2	; address to write to
	push dword ATTR_BOLD	; attrib
	push str_simd_32	; string
	call write_str_32
	add esp, WORD_SIZE*3	; remove args from stack

	jmp exit_32
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; data
; -----------------------------------------------------------------------------

; page table dir entry for 4MB pages
; see https://wiki.osdev.org/Paging and http://www.lowlevel.eu/wiki/Paging
struc ptd4_struc
	ptd4_flags resb 1
	ptd4_offs32_34_flags8_12 resb 1
	ptd4_offs22_23_offs35_40 resb 1
	ptd4_offs24_31 resb 1
endstruc



; see https://wiki.osdev.org/Interrupt_Descriptor_Table
; idt table description
struc idtr_struc
	idt_size resw 1
	idt_offs resd 1
endstruc

; idt table entries
struc idt_struc
	idt_offs0_15 resw 1
	idt_codeseg resw 1
	idt_reserved resb 1
	idt_flags resb 1
	idt_offs16_31 resw 1
endstruc


idtr: istruc idtr_struc
	at idt_size, dw idt_struc_size*NUM_IDT_ENTRIES - 1	; size - 1
	at idt_offs, dd idt_base_addr
iend


; see https://wiki.osdev.org/Interrupt_Vector_Table
idt_base_addr:

idt_descr_0: istruc idt_struc
	at idt_offs0_15, dw isr_div0
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_div0) >> 10h
iend

idt_descr_1: istruc idt_struc
	at idt_offs0_15, dw isr_null
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_null) >> 10h
iend

idt_descr_2: istruc idt_struc
	at idt_offs0_15, dw isr_null
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_null) >> 10h
iend

idt_descr_3: istruc idt_struc
	at idt_offs0_15, dw isr_null
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_null) >> 10h
iend

idt_descr_4: istruc idt_struc
	at idt_offs0_15, dw isr_overflow
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_overflow) >> 10h
iend

idt_descr_5: istruc idt_struc
	at idt_offs0_15, dw isr_bounds
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_bounds) >> 10h
iend

idt_descr_6: istruc idt_struc
	at idt_offs0_15, dw isr_instr
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_instr) >> 10h
iend

idt_descr_7: istruc idt_struc
	at idt_offs0_15, dw isr_dev
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_dev) >> 10h
iend

idt_descr_8: istruc idt_struc
	at idt_offs0_15, dw isr_df
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_df) >> 10h
iend

idt_descr_9: istruc idt_struc
	at idt_offs0_15, dw isr_overrun
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_overrun) >> 10h
iend

idt_descr_10: istruc idt_struc
	at idt_offs0_15, dw isr_tssfault
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_tssfault) >> 10h
iend

idt_descr_11: istruc idt_struc
	at idt_offs0_15, dw isr_segfault_notavail
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_segfault_notavail) >> 10h
iend

idt_descr_12: istruc idt_struc
	at idt_offs0_15, dw isr_segfault_stack
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_segfault_stack) >> 10h
iend

idt_descr_13: istruc idt_struc
	at idt_offs0_15, dw isr_gpf
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_gpf) >> 10h
iend

idt_descr_14: istruc idt_struc
	at idt_offs0_15, dw isr_pagefault
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_pagefault) >> 10h
iend

idt_descr_15: istruc idt_struc
	at idt_offs0_15, dw isr_null
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_null) >> 10h
iend

idt_descr_16: istruc idt_struc
	at idt_offs0_15, dw isr_fpu
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_fpu) >> 10h
iend

idt_descr_17: istruc idt_struc
	at idt_offs0_15, dw isr_align
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_align) >> 10h
iend

idt_descr_18: istruc idt_struc
	at idt_offs0_15, dw isr_mce
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_mce) >> 10h
iend

idt_descr_19: istruc idt_struc
	at idt_offs0_15, dw isr_simd
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_simd) >> 10h
iend

; null descriptors till the beginning of the primary pic interrupts
times (PIC0_INT_OFFS-(19+1))*idt_struc_size db 0

idt_descr_32: istruc idt_struc
	at idt_offs0_15, dw isr_timer
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_timer) >> 10h
iend

idt_descr_33: istruc idt_struc
	at idt_offs0_15, dw isr_keyb
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_keyb) >> 10h
iend

; null descriptors till the beginning of the secondary pic interrupts
times (PIC1_INT_OFFS-(33+1))*idt_struc_size db 0

idt_descr_40: istruc idt_struc
	at idt_offs0_15, dw isr_rtc
	at idt_codeseg, dw code_descr-gdt_base_addr
	at idt_reserved, db 00h
	at idt_flags, db 1_00_0_1110b	; used=1, ring=0, trap/int=0, int. gate
	at idt_offs16_31, dw (BOOTSECT_START-$$ + isr_rtc) >> 10h
iend

; null descriptors for the rest of the interrupts
times (NUM_IDT_ENTRIES-(40+1))*idt_struc_size db 0


str_start_32: db "Starting 32bit...", 00h

str_div0_32: db "*** Exception: Division by Zero ***", 00h
str_overflow_32: db "*** Exception: Overflow ***", 00h
str_bounds_32: db "*** Exception: Out of Bounds ***", 00h
str_instr_32: db "*** Fault: Unknown Instruction ***", 00h
str_overrun_32: db "*** Fault: Overrun ***", 00h
str_dev_32: db "*** Fault: No Device ***", 00h
str_tssfault_32: db "*** Fault: TSS ***", 00h
str_segfault_notavail_32: db "*** Segmentation Fault: Not Available ***", 00h
str_segfault_stack_32: db "*** Segmentation Fault: Stack ***", 00h
str_pagefault_32: db "*** Paging Fault ***", 00h
str_df_32: db "*** Double Fault ***", 00h
str_gpf_32: db "*** General Protection Fault ***", 00h
str_fpu_32: db "*** Fault: FPU ***", 00h
str_align_32: db "*** Exception: Alignment ***", 00h
str_mce_32: db "*** Exception: MCE ***", 00h
str_simd_32: db "*** Exception: SIMD ***", 00h


sector_fill: times END_ADDR - sector_fill db FILL_BYTE
; -----------------------------------------------------------------------------
