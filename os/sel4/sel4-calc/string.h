/**
 * simple libc string replacement functions
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __MY_STRING_H__
#define __MY_STRING_H__

#include "defines.h"


extern void reverse_str(i8* buf, u64 len);

extern void uint_to_str(u64 num, u64 base, i8* buf);
extern void int_to_str(i64 num, u64 base, i8* buf);

extern i64 my_atoi(const i8* str, i64 base);
extern f64 my_atof(const i8* str, i64 base);

extern void my_strncpy(i8* str_dst, const i8* str_src, i64 max_len);
extern void strncat_char(i8* str, i8 c, i64 max_len);

extern i8 my_strncmp(const i8* str1, const i8* str2, i64 max_len);
extern i8 my_strcmp(const i8* str1, const i8* str2);

extern u64 my_strlen(const i8* str);
extern void my_memset(i8* mem, i8 val, u64 size);

extern i64 my_max(i64 a, i64 b);

extern i8 my_isupperalpha(i8 c);
extern i8 my_isloweralpha(i8 c);
extern i8 my_isalpha(i8 c);
extern i8 my_isdigit(i8 c, i8 hex);

extern void write_char(i8 ch, u8 attrib, i8 *addr);
extern void write_str(i8 *str, u8 attrib, i8 *addr);
extern void read_str(i8 *str, const i8 *addr, u32 len);

#endif
