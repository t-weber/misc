;
; long mode test
; @author Tobias Weber
; @date feb-2021
; @license see 'LICENSE.GPL' file
;
; references:
;	- https://wiki.osdev.org/Setting_Up_Long_Mode
;	- https://wiki.osdev.org/Babystep7
;	- https://wiki.osdev.org/Babystep1
;	- https://wiki.osdev.org/Global_Descriptor_Table
;	- "Writing a simple operating system -- from scratch" by N. Blundell, 2010
;

%define WORD_SIZE         8
%define WORD_SIZE_32      4
%define WORD_SIZE_16      2

; -----------------------------------------------------------------------------
; paging
; -----------------------------------------------------------------------------
; see https://wiki.osdev.org/CPU_Registers_x86-64#IA32_EFER
%define REG_EFER          0xc000_0080
%define EFER_LM           (1b << 8)
%define CR0_PM            (1b << 0)
%define CR0_PAGING        (1b << 31)
%define CR4_PAGESIZE      (1b << 4)
%define CR4_ADDREXT       (1b << 5)

%define PAGING_PML4_START      (1b << 23)                       ; PML4 paging table starts at 8 MB
%define PAGING_PDPT_START      (PAGING_PML4_START + (1b << 12)) ; page dir. pointer table starts at +4kB
%define PAGING_PTD_START       (PAGING_PDPT_START + (1b << 12)) ; page table dir. starts at +4kB
%define PAGING_FRAME_START     0b                               ; start address of page frames
%define PAGING_PTD_ENTRY_SIZE  8
%define PAGING_PTD_ENTRIES     4                                ; number of present page table directory entries
%define PAGING_PTD_MAX_ENTRIES 512                              ; number of max. page table directory entries
%define PAGING_FRAME_SIZE      (1b << 21)                       ; 2 MB page frames

%define PTD_SIZE          (1b << 7)
%define PTD_AVAIL         (1b << 6)
%define PTD_SUPERVISOR    (1b << 2)
%define PTD_WRITE         (1b << 1)
%define PTD_PRESENT       (1b << 0)

%define PDPT_AVAIL        (1b << 6)
%define PDPT_SUPERVISOR   (1b << 2)
%define PDPT_WRITE        (1b << 1)
%define PDPT_PRESENT      (1b << 0)

%define PML4_AVAIL        (1b << 6)
%define PML4_SUPERVISOR   (1b << 2)
%define PML4_WRITE        (1b << 1)
%define PML4_PRESENT      (1b << 0)
; -----------------------------------------------------------------------------

; -----------------------------------------------------------------------------
; addresses
; -----------------------------------------------------------------------------
; see https://jbwyatt.com/253/emu/memory.html
%define CHAROUT           000b_8000h ; base address for character output
%define SCREEN_ROW_SIZE   25
%define SCREEN_COL_SIZE   80
%define SCREEN_SIZE       SCREEN_ROW_SIZE * SCREEN_COL_SIZE
%define ATTR_BOLD         0000_1111b

; see https://wiki.osdev.org/Memory_Map_(x86) and https://en.wikipedia.org/wiki/Master_boot_record
%define BOOTSECT_START    7c00h
%define STACK_START_16    7fffh      ; some (arbitrary) stack base address
%define STACK_START       0040_0000h ; stack base address at 4MB
;%define STACK_START       00f0_0000h ; stack base address at 16MB
; -----------------------------------------------------------------------------

; -----------------------------------------------------------------------------
; segmentation
; -----------------------------------------------------------------------------
%define NUM_GDT_ENTRIES   4
%define NUM_IDT_ENTRIES   256

%define GDT_PRESENT       (1b << 7)
%define GDT_CODEDATA      (1b << 4)
%define GDT_EXEC          (1b << 3)
%define GDT_EXPAND_DOWN   (1b << 2)
%define GDT_WRITABLE      (1b << 1)
%define GDT_READABLE      (1b << 1)
%define GDT_GRANULARITY   (1b << 7)
%define GDT_32BIT         (1b << 6)
%define GDT_64BIT         (1b << 5)

%define SEG_DATA_START    0
%define SEG_DATA_LEN      0xffffff
%define SEG_STACK_START   0
%define SEG_STACK_LEN     0
%define SEG_CODE_START    0
%define SEG_CODE_LEN      0xffffff
; -----------------------------------------------------------------------------

; -----------------------------------------------------------------------------
%define NUM_ASM_SECTORS   12                     ; total number of sectors  ceil("pm.x86" file size / SECTOR_SIZE)
%define NUM_C_SECTORS     15                     ; number of sectors for c code
;%define NUM_C_SECTORS     23                    ; number of sectors for c code
%define SECTOR_SIZE       200h
%define MBR_BOOTSIG_ADDR  ($$ + SECTOR_SIZE - 2) ; 510 bytes after section start
%define END_ADDR          ($$ + SECTOR_SIZE * NUM_ASM_SECTORS)
%define FILL_BYTE         0xf4                   ; fill with hlt

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


; -----------------------------------------------------------------------------
; code in 16-bit real mode
; in boot sector
; -----------------------------------------------------------------------------
[bits 16]
;align 2
;[org BOOTSECT_START]       ; not needed, set by linker
	mov sp, STACK_START_16
	mov bp, STACK_START_16
	push dx

	; clear screen
	call clear_16

	; write start message
	push word ATTR_BOLD     ; attrib
	push word str_start_16  ; string
	call write_str_16
	add sp, WORD_SIZE_16*2  ; remove args from stack

	; read all needed sectors
	; see https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=02h:_Read_Sectors_From_Drive
	pop dx
	;xor ax, ax
	mov ax, cs
	mov es, ax
	mov ah, 02h             ; func
	mov al, byte (NUM_ASM_SECTORS + NUM_C_SECTORS - 1) ; sector count (boot sector already loaded)
	mov bx, word start_32   ; destination address es:bx
	mov cx, 00_02h          ; C = 0, S = 2
	mov dh, 00h             ; H = 0
	;mov dl, BIOS_HDD0      ; drive
	clc
	int 13h	; sets carry flag on failure
	jnc load_sectors_succeeded_16
		; write error message on failure
		push word ATTR_BOLD	        ; attrib
		push word str_load_error_16 ; string
		call write_str_16
		add sp, WORD_SIZE_16*2      ; remove args from stack
		jmp exit_16
	load_sectors_succeeded_16:

	cli                             ; disable interrupts
	call config_pics_16             ; configure pics
	lgdt [gdtr]                     ; load gdt register

	mov eax, cr0
	or eax, CR0_PM                  ; set protection enable bit
	mov cr0, eax

	; go to 32 bit code
	jmp code_descr-gdt_base_addr : start_32 ; automatically sets cs



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
	mov ax, word CHAROUT / 16  ; base address segment
	mov es, ax
	mov di, word 00h           ; base address

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

	push word [bp + WORD_SIZE_16*2] ; argument: char*
	call strlen_16
	add sp, WORD_SIZE_16            ; remove arg from stack

	xor ax, ax
	mov ds, ax                    ; data segment for char*
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
	mov ds, ax                         ; data segment for char*
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
	.offs resq 1
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

align 4, db 0
; see https://wiki.osdev.org/Babystep7
gdtr: istruc gdtr_struc
	at gdtr_struc.size, dw gdt_struc_size*NUM_GDT_ENTRIES - 1	; size - 1
	at gdtr_struc.offs, dq gdt_base_addr
iend

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

align 4, db 0
gdt_base_addr: times gdt_struc_size db 0	; null descriptor

data_descr: gdt_descr SEG_DATA_LEN, SEG_DATA_START, \
	GDT_PRESENT|GDT_CODEDATA|GDT_WRITABLE, GDT_GRANULARITY|GDT_32BIT, 0
stack_descr: gdt_descr SEG_STACK_LEN, SEG_STACK_START, \
	GDT_PRESENT|GDT_CODEDATA|GDT_WRITABLE|GDT_EXPAND_DOWN, GDT_GRANULARITY|GDT_32BIT, 0
code_descr: gdt_descr SEG_CODE_LEN, SEG_CODE_START, \
	GDT_PRESENT|GDT_CODEDATA|GDT_READABLE|GDT_EXEC, GDT_GRANULARITY|GDT_32BIT, 0

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
align 4
start_32:
	; data segment
	mov ax, data_descr-gdt_base_addr
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; stack segment
	mov ax, stack_descr-gdt_base_addr
	mov ss, ax
	mov esp, dword STACK_START
	mov ebp, dword STACK_START

	push dword CHAROUT + SCREEN_COL_SIZE*2 ; address to write to
	push dword ATTR_BOLD                   ; attrib
	push str_start_32                      ; string
	call write_str_32
	add esp, WORD_SIZE_32*3                ; remove args from stack

	; paging
	; see https://wiki.osdev.org/Paging and http://www.lowlevel.eu/wiki/Paging
	mov eax, PAGING_FRAME_START            ; start address of page frames
	mov ebx, PAGING_PTD_START              ; page table dir. start address

	mov ecx, PAGING_PTD_ENTRIES            ; number of page table directory entries
	ptd_loop_avail_pages:
		; lower dword
		mov dword [ebx + 0], dword eax
		or byte [ebx + 0], byte PTD_SIZE|PTD_AVAIL|PTD_SUPERVISOR|PTD_WRITE|PTD_PRESENT
		; upper dword
		mov dword [ebx + 4], dword 00_00_00_00h

		add ebx, PAGING_PTD_ENTRY_SIZE ; next page entry
		add eax, PAGING_FRAME_SIZE     ; next page frame (2 MB)

		dec ecx                        ; next counter
		cmp ecx, 0
		jnz ptd_loop_avail_pages       ; loop if counter > 0

%if PAGING_PTD_MAX_ENTRIES-PAGING_PTD_ENTRIES > 0
	mov ecx, PAGING_PTD_MAX_ENTRIES-PAGING_PTD_ENTRIES ; number of non-present page table directory entries
	ptd_loop_nonavail_pages:
		; lower dword
		mov dword [ebx + 0], dword eax
		; size=1, avail=0, accessed=0, cache flags=00, supervisor=1, write=1, present=0
		or byte [ebx + 0], byte PTD_SIZE|PTD_SUPERVISOR|PTD_WRITE
		; upper dword
		mov dword [ebx + 4], dword 00_00_00_00h

		add ebx, PAGING_PTD_ENTRY_SIZE ; next page entry
		add eax, PAGING_FRAME_SIZE     ; next page frame (2 MB)

		dec ecx                        ; next counter
		cmp ecx, 0
		jnz ptd_loop_nonavail_pages    ; loop if counter > 0
%endif

	; page dir. pointer table
	mov dword [PAGING_PDPT_START + 0], dword PAGING_PTD_START  ; page table dir. start address
	or byte [PAGING_PDPT_START + 0], byte PDPT_AVAIL|PDPT_SUPERVISOR|PDPT_WRITE|PDPT_PRESENT
	mov dword [PAGING_PDPT_START + 4], dword 00_00_00_00h

	; pml4 table
	mov dword [PAGING_PML4_START + 0], dword PAGING_PDPT_START ; page dir. pointer table start address
	; avail=1, accessed=0, cache flags=00, supervisor=1, write=1, present=1
	or byte [PAGING_PML4_START + 0], byte PML4_AVAIL|PML4_SUPERVISOR|PML4_WRITE|PML4_PRESENT
	mov dword [PAGING_PML4_START + 4], dword 00_00_00_00h

	mov eax, PAGING_PML4_START
	mov cr3, eax

	; enable long mode
	; see https://wiki.osdev.org/CPU_Registers_x86-64#IA32_EFER
	mov ecx, REG_EFER
	rdmsr
	or eax, EFER_LM
	wrmsr

	; reset segmentation: modify gdt entries and reload gdt register
	mov byte [data_descr + gdt_struc.sizes_limit16_19], byte GDT_GRANULARITY|GDT_64BIT | (SEG_DATA_LEN & 0xf)
	mov byte [stack_descr + gdt_struc.sizes_limit16_19], byte GDT_GRANULARITY|GDT_64BIT | (SEG_STACK_LEN & 0xf)
	mov byte [code_descr + gdt_struc.sizes_limit16_19], byte GDT_GRANULARITY|GDT_64BIT | (SEG_CODE_LEN & 0xf)
	lgdt [gdtr]

	; set address extension
	mov eax, cr4
	or eax, CR4_ADDREXT ;| CR4_PAGESIZE
	mov cr4, eax

	; enable paging
	mov eax, cr0
	or eax, CR0_PAGING
	mov cr0, eax

	; go to 64 bit code
	jmp code_descr-gdt_base_addr : start_64	; automatically sets cs



;
; clear screen
;
clear_32:
	push ebp
	mov ebp, esp

	mov ecx, [ebp + WORD_SIZE_32*2]  ; size
	mov edi, [ebp + WORD_SIZE_32*3]  ; base address

	mov ax, word 00_00h
	rep stosw

	mov esp, ebp
	pop ebp
	ret



;
; write a string to charout
; @param [esp + WORD_SIZE_32] pointer to a string
; @param [esp + WORD_SIZE_32*2] attributes
; @param [esp + WORD_SIZE_32*3] base address to write to
;
write_str_32:
	push ebp
	mov ebp, esp

	mov eax, [ebp + WORD_SIZE_32*2]   ; argument: char*
	push eax
	call strlen_32
	mov ecx, eax                      ; string length
	add esp, WORD_SIZE_32             ; remove arg from stack

	mov esi, [ebp + WORD_SIZE_32*2]   ; char*
	mov dh, [ebp + WORD_SIZE_32*3]    ; attrib
	mov edi, [ebp + WORD_SIZE_32*4]   ; base address

	write_loop_32:
		mov dl, byte [esi]            ; dereference char*
		mov [edi], dl                 ; store char
		inc edi                       ; to char attribute index
		mov [edi], dh                 ; store char attributes
		inc edi                       ; next output char index
		dec ecx                       ; decrement length counter
		inc esi                       ; increment char pointer
		cmp ecx, 0
		jnz write_loop_32

	mov esp, ebp
	pop ebp
	ret



;
; strlen
; @param [esp + WORD_SIZE_32] pointer to a string
; @returns length in eax
;
strlen_32:
	push ebp
	mov ebp, esp

	mov esi, [ebp + WORD_SIZE_32*2]   ; argument: char*
	mov edi, esi                      ; argument: char*

	mov ecx, 0xffff                   ; max. string length
	xor eax, eax                      ; look for 0
	repnz scasb

	mov eax, edi                      ; eax = end_ptr
	sub eax, esi                      ; eax -= begin_ptr
	sub eax, 1

	mov esp, ebp
	pop ebp
	ret



;
; halt
;
exit_32:
	;cli
	hlt_loop_32: hlt     ; loop, because hlt can be interrupted
	jmp hlt_loop_32
; -----------------------------------------------------------------------------




; -----------------------------------------------------------------------------
; code in 64-bit long mode
; -----------------------------------------------------------------------------
[bits 64]
align 8
start_64:
	; data segment
	mov ax, data_descr-gdt_base_addr
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; stack segment
	mov ax, stack_descr-gdt_base_addr
	mov ss, ax
	mov rsp, qword STACK_START
	mov rbp, qword STACK_START

	; clear screen
	;mov rdi, CHAROUT      ; address to write to
	;mov rcx, SCREEN_SIZE  ; number of characters to write
	;call clear_64

	mov rsi, str_start_64                ; string
	mov rdi, CHAROUT + SCREEN_COL_SIZE*4 ; address to write to
	mov dl, ATTR_BOLD                    ; attrib
	call write_str_64

	[extern entrypoint]
	call entrypoint

	lidt [idtr]    ; load idt register
	sti            ; enable interrupts

	jmp exit_64



;
; clear screen
;
align 8
clear_64:
	mov ax, word 00_00h
	rep stosw

	retq



;
; write a string to charout
; @param rsi pointer to a string
; @param rdi base address to write to
; @param dl attributes
;
align 8
write_str_64:
	push rsi
	call strlen_64
	pop rsi

	write_loop:
		mov dh, byte [rsi]  ; dereference char*
		mov byte [rdi], dh  ; store char
		inc rdi             ; to char attribute index
		mov byte [rdi], dl  ; store char attributes
		inc rdi             ; next output char index
		dec rcx             ; decrement length counter
		inc rsi             ; increment char pointer
		cmp rcx, 0
		jnz write_loop

	retq



;
; strlen
; @param rsi pointer to a string
; @returns length in rcx
;
align 8
strlen_64:
	xor rcx, rcx
	count_chars_64:
		mov al, byte [rsi]  ; dl = *str_ptr
		cmp al, 0
		jz count_chars_end_64
		inc rcx             ; ++counter
		inc rsi             ; ++str_ptr
		jmp count_chars_64
	count_chars_end_64:

	retq



;
; halt
;
align 8
exit_64:
	;cli
	hlt_loop_64: hlt        ; loop, because hlt can be interrupted
	jmp hlt_loop_64



align 8
print_exit_64:
	push rax
	mov rdi, CHAROUT        ; address to write to
	mov rcx, SCREEN_SIZE    ; number of characters to write
	call clear_64
	pop rax

	mov rsi, rax            ; string
	mov rdi, qword CHAROUT + SCREEN_COL_SIZE*2  ; address to write to
	mov dl, ATTR_BOLD       ; attrib
	call write_str_64

	jmp exit_64



; -----------------------------------------------------------------------------
; interrupt service routines
; see https://wiki.osdev.org/Interrupt_Service_Routines and https://wiki.osdev.org/Interrupts
; -----------------------------------------------------------------------------

;
; dummy isr
;
align 8
isr_null:
	iretq


; see https://wiki.osdev.org/%228042%22_PS/2_Controller
align 8
isr_keyb:
	xor rax, rax
	in al, KEYB_DATA_PORT

	mov rdi, rax         ; calling convention passes arguments in registers, not on stack
	;push rax
	[extern keyb_event]
	call keyb_event
	;add rsp, WORD_SIZE

	iretq


align 8
isr_timer:
	[extern timer_event]
	call timer_event

	iretq


align 8
isr_rtc:
	[extern rtc_event]
	call rtc_event

	iretq


align 8
isr_div0:
	mov rax, str_div0_64
	jmp print_exit_64


align 8
isr_overflow:
	mov rax, str_overflow_64
	jmp print_exit_64


align 8
isr_bounds:
	mov rax, str_bounds_64
	jmp print_exit_64


align 8
isr_instr:
	mov rax, str_instr_64
	jmp print_exit_64


align 8
isr_dev:
	mov rax, str_dev_64
	jmp print_exit_64


align 8
isr_tssfault:
	mov rax, str_tssfault_64
	jmp print_exit_64


align 8
isr_segfault_notavail:
	mov rax, str_segfault_notavail_64
	jmp print_exit_64


align 8
isr_segfault_stack:
	mov rax, str_segfault_stack_64
	jmp print_exit_64


align 8
isr_pagefault:
	mov rax, str_pagefault_64
	jmp print_exit_64


align 8
isr_df:
	mov rax, str_df_64
	jmp print_exit_64


align 8
isr_overrun:
	mov rax, str_overrun_64
	jmp print_exit_64


align 8
isr_gpf:
	mov rax, str_gpf_64
	jmp print_exit_64


align 8
isr_fpu:
	mov rax, str_fpu_64
	jmp print_exit_64


align 8
isr_align:
	mov rax, str_align_64
	jmp print_exit_64


align 8
isr_mce:
	mov rax, str_mce_64
	jmp print_exit_64


align 8
isr_simd:
	mov rax, str_simd_64
	jmp print_exit_64
; -----------------------------------------------------------------------------



; -----------------------------------------------------------------------------
; data
; -----------------------------------------------------------------------------

; idt table description
; see https://wiki.osdev.org/Interrupt_Descriptor_Table
struc idtr_struc
	.size resw 1
	.offs resq 1
endstruc

; idt table entries
struc idt_struc
	.offs0_15 resw 1
	.codeseg resw 1
	.stackoffs resb 1
	.flags resb 1
	.offs16_31 resw 1
	.offs32_63 resd 1
	.reserved resd 1
endstruc


align 8, db 0
idtr: istruc idtr_struc
	at idtr_struc.size, dw idt_struc_size*NUM_IDT_ENTRIES - 1	; size - 1
	at idtr_struc.offs, dq idt_base_addr
iend


; see https://wiki.osdev.org/Interrupt_Vector_Table
; interrupt descriptor
%macro idt_descr 1
	istruc idt_struc
		at idt_struc.offs0_15, dw %1
		at idt_struc.codeseg, dw code_descr-gdt_base_addr
		at idt_struc.stackoffs, db 00h
		at idt_struc.flags, db 1_00_0_1110b  ; used=1, ring=0, trap/int=0, int. gate
		at idt_struc.offs16_31, dw (BOOTSECT_START-$$ + %1) >> 10h
		at idt_struc.offs32_63, dd (BOOTSECT_START-$$ + %1) >> 20h
		at idt_struc.reserved, dd 00h
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


align 8, db 0
str_start_32: db "Starting 32bit...", 00h
str_start_64: db "Starting 64bit...", 00h

str_div0_64: db "*** Exception: Division by Zero ***", 00h
str_overflow_64: db "*** Exception: Overflow ***", 00h
str_bounds_64: db "*** Exception: Out of Bounds ***", 00h
str_instr_64: db "*** Fault: Unknown Instruction ***", 00h
str_overrun_64: db "*** Fault: Overrun ***", 00h
str_dev_64: db "*** Fault: No Device ***", 00h
str_tssfault_64: db "*** Fault: TSS ***", 00h
str_segfault_notavail_64: db "*** Segmentation Fault: Not Available ***", 00h
str_segfault_stack_64: db "*** Segmentation Fault: Stack ***", 00h
str_pagefault_64: db "*** Paging Fault ***", 00h
str_df_64: db "*** Double Fault ***", 00h
str_gpf_64: db "*** General Protection Fault ***", 00h
str_fpu_64: db "*** Fault: FPU ***", 00h
str_align_64: db "*** Exception: Alignment ***", 00h
str_mce_64: db "*** Exception: MCE ***", 00h
str_simd_64: db "*** Exception: SIMD ***", 00h


sector_fill: times END_ADDR - sector_fill db FILL_BYTE
; -----------------------------------------------------------------------------
