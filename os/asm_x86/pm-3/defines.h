/**
 * protected mode test
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __PM_DEFINES_H__
#define __PM_DEFINES_H__


typedef unsigned char u8;
typedef char i8;
typedef unsigned short u16;
typedef short i16;
typedef unsigned int u32;
typedef int i32;


#define SCREEN_ROW_SIZE 25
#define SCREEN_COL_SIZE 80
#define SCREEN_SIZE     (SCREEN_ROW_SIZE * SCREEN_COL_SIZE)

#define CHAROUT ((i8*)0x000b8000) /* see https://jbwyatt.com/253/emu/memory.html */


#endif
