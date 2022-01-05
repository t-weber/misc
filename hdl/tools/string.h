/**
 * simple libc string replacement functions
 * @author Tobias Weber
 * @date mar-21
 * @license see 'LICENSE.GPL' file
 */

#ifndef __MY_STRING_H__
#define __MY_STRING_H__

#include <stdint.h>


extern void reverse_str(int8_t* buf, uint32_t len);

extern int8_t digit_to_char(uint8_t num, uint32_t base);
extern void uint_to_str(uint32_t num, uint32_t base, int8_t* buf);
extern void int_to_str(int32_t num, uint32_t base, int8_t* buf);
extern void real_to_str(float num, uint32_t base, int8_t* buf, uint8_t decimals);

extern int32_t my_atoi(const int8_t* str, int32_t base);
extern float my_atof(const int8_t* str, int32_t base);

extern void my_strncpy(int8_t* str_dst, const int8_t* str_src, uint32_t max_len);
extern void my_strncat(int8_t* str_dst, const int8_t* str_src, uint32_t max_len);
extern void strncat_char(int8_t* str, int8_t c, uint32_t max_len);

extern int8_t my_strncmp(const int8_t* str1, const int8_t* str2, uint32_t max_len);
extern int8_t my_strcmp(const int8_t* str1, const int8_t* str2);

extern uint32_t my_strlen(const int8_t* str);
extern void my_memset(int8_t* mem, int8_t val, uint32_t size);
extern void my_memset_interleaved(int8_t* mem, int8_t val, uint32_t size, uint8_t interleave);
extern void my_memcpy(int8_t* mem_dst, int8_t* mem_src, uint32_t size);
extern void my_memcpy_interleaved(int8_t* mem_dst, int8_t* mem_src, uint32_t size, uint8_t interleave);

extern int32_t my_max(int32_t a, int32_t b);

extern int8_t my_isupperalpha(int8_t c);
extern int8_t my_isloweralpha(int8_t c);
extern int8_t my_isalpha(int8_t c);
extern int8_t my_isdigit(int8_t c, int8_t hex);


#endif
