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
;	- https://en.wikipedia.org/wiki/X86_calling_conventions
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
%define USE_STRING_OPS               ; use optimised string functions

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
	add sp, WORD_SIZE_16*2	; remove arg from stack
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
		jnz clear_write_loop_16

	ret


;
; write a string to charout
; @param [sp + WORD_SIZE_16] pointer to a string
;
write_str_16:
	push bp
	mov bp, sp

	push word [bp + WORD_SIZE_16*2] ; argument: char*
	call strlen_16
	add sp, WORD_SIZE_16         ; remove arg from stack

	xor ax, ax
	mov ds, ax                   ; data segment for char*
	mov si, [bp + WORD_SIZE_16*2] ; char*
	mov dh, [bp + WORD_SIZE_16*3] ; attrib

	mov ax, CHAROUT / 16         ; base address segment
	mov es, ax
	mov di, word 00h             ; base address

	write_loop_16:
		mov dl, byte [ds:si]     ; dereference char*
		mov [es:di], dl          ; store char
		inc di                   ; to char attribute index
		mov [es:di], dh          ; store char attributes
		inc di                   ; next output char index
		dec cx                   ; decrement length counter
		inc si                   ; increment char pointer
		cmp cx, 0
		jnz write_loop_16

	mov sp, bp
	pop bp
	ret


;
; strlen_16
; @param [sp + WORD_SIZE_16] pointer to a string
; @returns length in cx
;
strlen_16:
	push bp
	mov bp, sp

	xor ax, ax
	mov ds, ax                   ; data segment for char*
	mov si, word [bp + WORD_SIZE_16*2] ; argument: char*

	xor cx, cx
	count_chars_16:
		mov dl, byte [ds:si]     ; dl = *str_ptr
		cmp dl, 0
		jz count_chars_end_16
		inc cx                   ; ++counter
		inc si                   ; ++str_ptr
		jmp count_chars_16
	count_chars_end_16:

	mov sp, bp
	pop bp
	ret
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; data
; -----------------------------------------------------------------------------
; see https://wiki.osdev.org/Global_Descriptor_Table
; see https://en.wikipedia.org/wiki/Global_Descriptor_Table

; gdt table description
struc gdtr_struc
	.size resw 1
	.offs resd 1
endstruc

; gdt table entries
struc gdt_struc
	.limit0_15 resw 1
	.base0_15 resw 1
	.base16_23 resb 1
	.access resb 1
	.sizes_limit16_19 resb 1
	.base24_31 resb 1
endstruc

; see https://wiki.osdev.org/Babystep7
gdtr: istruc gdtr_struc
	at gdtr_struc.size, dw gdt_struc_size*NUM_GDT_ENTRIES - 1	; size - 1
	at gdtr_struc.offs, dd gdt_base_addr
iend

%define GDT_PRESENT     (1b << 7)
%define GDT_CODEDATA    (1b << 4)
%define GDT_EXEC        (1b << 3)
%define GDT_EXPAND_DOWN (1b << 2)
%define GDT_WRITABLE    (1b << 1)
%define GDT_READABLE    (1b << 1)
%define GDT_GRANULARITY (1b << 7)
%define GDT_32BIT       (1b << 6)

%macro gdt_descr 5 ; args: limit, base, flags, flags 2, ring
	istruc gdt_struc
		at gdt_struc.limit0_15, dw (%1 & 0xffff)
		at gdt_struc.base0_15, dw (%2 & 0xffff)
		at gdt_struc.base16_23, db (%2>>16)
		at gdt_struc.access, db %3 | (%5 << 5)
		at gdt_struc.sizes_limit16_19, db %4 | (%1>>16 & 0xf)
		at gdt_struc.base24_31, db (%2>>24 & 0xff)
	iend
%endmacro

gdt_base_addr: times gdt_struc_size db 0	; null descriptor
data_descr: gdt_descr 0xffffff, 0x0, GDT_PRESENT|GDT_CODEDATA|GDT_WRITABLE, GDT_GRANULARITY|GDT_32BIT, 0
stack_descr: gdt_descr 0x0, 0x0, GDT_PRESENT|GDT_CODEDATA|GDT_WRITABLE|GDT_EXPAND_DOWN, GDT_GRANULARITY|GDT_32BIT, 0
code_descr: gdt_descr 0xffffff, 0x0, GDT_PRESENT|GDT_CODEDATA|GDT_READABLE|GDT_EXEC, GDT_GRANULARITY|GDT_32BIT, 0


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
	mov eax, (1b << 22) ; page directory start address at 4MB
	mov edi, eax
	;mov ecx, 1024      ; max. number of page directory entries
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

	; load idt register and enable interrupts
	lidt [idtr]
	sti

	call exit_32



;
; clear screen
;
clear_32:
	push ebp
	mov ebp, esp

	mov ecx, [ebp + WORD_SIZE*2]  ; size
	mov edi, [ebp + WORD_SIZE*3]  ; base address

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

	mov esp, ebp
	pop ebp
	ret



;
; write a string to charout
; @param [esp + WORD_SIZE] pointer to a string
; @param [esp + WORD_SIZE*2] attributes
; @param [esp + WORD_SIZE*3] base address to write to
;
write_str_32:
	push ebp
	mov ebp, esp

	mov eax, [ebp + WORD_SIZE*2]	; argument: char*
	push eax
	call strlen_32
	mov ecx, eax	; string length
	add esp, WORD_SIZE	; remove arg from stack

	mov esi, [ebp + WORD_SIZE*2]	; char*
	mov dh, [ebp + WORD_SIZE*3]		; attrib
	mov edi, [ebp + WORD_SIZE*4]	; base address

	write_loop:
		mov dl, byte [esi]	; dereference char*
		mov [edi], dl	; store char
		inc edi		; to char attribute index
		mov [edi], dh	; store char attributes
		inc edi		; next output char index
		dec ecx		; decrement length counter
		inc esi		; increment char pointer
		cmp ecx, 0
		jnz write_loop

	mov esp, ebp
	pop ebp
	ret



;
; strlen
; @param [esp + WORD_SIZE] pointer to a string
; @returns length in eax
;
strlen_32:
	push ebp
	mov ebp, esp

	mov esi, [ebp + WORD_SIZE*2]  ; argument: char*

%ifdef USE_STRING_OPS
	mov edi, esi                  ; argument: char*

	mov ecx, 0xffff               ; max. string length
	xor eax, eax                  ; look for 0
	repnz scasb

	mov eax, edi                  ; eax = end_ptr
	sub eax, esi                  ; eax -= begin_ptr
	sub eax, 1

%else
	xor ecx, ecx
	count_chars:
		mov dl, byte [esi]        ; dl = *string_ptr
		cmp dl, 0
		jz count_chars_end
		inc ecx                   ; ++counter
		inc esi                   ; ++str_ptr
		jmp count_chars
	count_chars_end:

	mov eax, ecx
%endif

	mov esp, ebp
	pop ebp
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
	;pushad

	xor eax, eax
	in al, KEYB_DATA_PORT
	push eax
	[extern keyb_event]
	call keyb_event
	add esp, WORD_SIZE

	;popad
	iret


isr_timer:
	;pushad

	[extern timer_event]
	call timer_event

	;popad
	iret


isr_rtc:
	;pushad

	[extern rtc_event]
	call rtc_event

	;popad
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
	.size resw 1
	.offs resd 1
endstruc

; idt table entries
struc idt_struc
	.offs0_15 resw 1
	.codeseg resw 1
	.reserved resb 1
	.flags resb 1
	.offs16_31 resw 1
endstruc


idtr: istruc idtr_struc
	at idtr_struc.size, dw idt_struc_size*NUM_IDT_ENTRIES - 1	; size - 1
	at idtr_struc.offs, dd idt_base_addr
iend


; see https://wiki.osdev.org/Interrupt_Vector_Table
; interrupt descriptor
%macro idt_descr 1
	istruc idt_struc
		at idt_struc.offs0_15, dw %1
		at idt_struc.codeseg, dw code_descr-gdt_base_addr
		at idt_struc.reserved, db 00h
		at idt_struc.flags, db 1_00_0_1110b  ; used=1, ring=0, trap/int=0, int. gate
		at idt_struc.offs16_31, dw (BOOTSECT_START-$$ + %1) >> 10h
	iend
%endmacro

align 8, db 0
idt_base_addr:

idt_descr_0: idt_descr isr_div0
idt_descr_1: idt_descr isr_null
idt_descr_2: idt_descr isr_null
idt_descr_3: idt_descr isr_null
idt_descr_4: idt_descr isr_overflow
idt_descr_5: idt_descr isr_bounds
idt_descr_6: idt_descr isr_instr
idt_descr_7: idt_descr isr_dev
idt_descr_8: idt_descr isr_df
idt_descr_9: idt_descr isr_overrun
idt_descr_10: idt_descr isr_tssfault
idt_descr_11: idt_descr isr_segfault_notavail
idt_descr_12: idt_descr isr_segfault_stack
idt_descr_13: idt_descr isr_gpf
idt_descr_14: idt_descr isr_pagefault
idt_descr_15: idt_descr isr_null
idt_descr_16: idt_descr isr_fpu
idt_descr_17: idt_descr isr_align
idt_descr_18: idt_descr isr_mce
idt_descr_19: idt_descr isr_simd

; null descriptors till the beginning of the primary pic interrupts
times (PIC0_INT_OFFS-(19+1))*idt_struc_size db 0

idt_descr_32: idt_descr isr_timer
idt_descr_33: idt_descr isr_keyb

; null descriptors till the beginning of the secondary pic interrupts
times (PIC1_INT_OFFS-(33+1))*idt_struc_size db 0

idt_descr_40: idt_descr isr_rtc

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
