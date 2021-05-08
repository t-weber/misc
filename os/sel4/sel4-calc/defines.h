/**
 * sel4 test
 * @author Tobias Weber
 * @date apr-2021
 * @license GPLv3, see 'LICENSE.GPL' file
 *
 * References:
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/mapping/mapping.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/untyped/untyped.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/threads/threads.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/tutorials/interrupts/interrupts.md
 *   - https://github.com/seL4/sel4-tutorials/blob/master/libsel4tutorials/src/alloc.c
 *   - https://docs.sel4.systems/projects/sel4/api-doc.html
 */

#ifndef __SEL4_TST_DEFS_H__
#define __SEL4_TST_DEFS_H__


#define SERIAL_DEBUG 1

#include <sel4/sel4.h>

#if SERIAL_DEBUG !=0
	#include <stdio.h>
#else
	#define printf(...) {}
#endif


/*
typedef unsigned char u8;
typedef char i8;
typedef unsigned short u16;
typedef short i16;
typedef unsigned int u32;
typedef int i32;
typedef unsigned long u64;
typedef long i64;
typedef float f32;
typedef double f64;
*/

typedef seL4_Uint8       u8;
typedef seL4_Int8        i8;
typedef seL4_Uint16      u16;
typedef seL4_Int16       i16;
typedef seL4_Uint32      u32;
typedef seL4_Int32       i32;
typedef seL4_Uint64      u64;
typedef seL4_Int64       i64;
typedef seL4_Word        word_t;
typedef float            f32;
typedef double           f64;


#define SCREEN_ROW_SIZE  25
#define SCREEN_COL_SIZE  80
#define SCREEN_SIZE      (SCREEN_ROW_SIZE * SCREEN_COL_SIZE)

//#define PAGE_TYPE      seL4_X86_LargePageObject
#define PAGE_TYPE        seL4_X86_4K
#define PAGE_SIZE        4096
#define CHAROUT_PHYS     0x000b8000  // see https://jbwyatt.com/253/emu/memory.html

#define ATTR_BOLD        0b00001111
#define ATTR_INV         0b01110000
#define ATTR_NORM        0b00000111

#define KEYB_DATA_PORT   0x60        // see https://wiki.osdev.org/%228042%22_PS/2_Controller
#define KEYB_PIC         0           // on which PIC is the keyboard?
#define KEYB_IRQ         1           // on which IRQ pin of the PIC is the keyboard?
#define KEYB_INT         33          // cpu interrupt to map to


#endif
