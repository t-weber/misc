/**
 * simple libc string replacement functions
 * @author Tobias Weber
 * @date mar-21
 * @license: see 'LICENSE.EUPL' file
 */

#ifndef __MY_STRING_H__
#define __MY_STRING_H__


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


void reverse_str(i8* buf, u64 len);

void uint_to_str(u64 num, u64 base, i8* buf);
void int_to_str(i64 num, u64 base, i8* buf);

i64 my_atoi(const i8* str, i64 base);
f64 my_atof(const i8* str, i64 base);

void my_strncpy(i8* str_dst, const i8* str_src, i64 max_len);
void strncat_char(i8* str, i8 c, i64 max_len);

i8 my_strncmp(const i8* str1, const i8* str2, i64 max_len);
i8 my_strcmp(const i8* str1, const i8* str2);

u64 my_strlen(const i8* str);
void my_memset(i8* mem, i8 val, u64 size);

void write_str(i8 *str, u8 attrib, i8 *addr);

i64 my_max(i64 a, i64 b);

i8 my_isupperalpha(i8 c);
i8 my_isloweralpha(i8 c);
i8 my_isalpha(i8 c);
i8 my_isdigit(i8 c, i8 hex);


#endif
