/**
 * protected mode test
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __PM_STRING_H__
#define __PM_STRING_H__


#include "defines.h"


void reverse_str(char* buf, u32 len);
void uint_to_str(u32 num, char* buf);
void int_to_str(i32 num, char* buf);

u32 strlen(char *str);
void memset(i8* mem, i8 val, u32 size);

void write_str(char *str, u8 attrib, i8 *addr);


#endif
