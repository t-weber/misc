/**
 * long mode test
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __PM_STRING_H__
#define __PM_STRING_H__


#include "defines.h"


void reverse_str(char* buf, u64 len);
void uint_to_str(u64 num, u64 base, char* buf);
void int_to_str(i64 num, u64 base, char* buf);

u64 strlen(char *str);
void memset(i8* mem, i8 val, u64 size);

void write_str(char *str, u8 attrib, i8 *addr);


#endif
