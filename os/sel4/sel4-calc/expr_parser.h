/**
 * simple LL(1) expression parser -- C version
 *
 * @author Tobias Weber
 * @date 14-mar-2020, 8-may-2021
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *	- https://www.cs.uaf.edu/~cs331/notes/FirstFollow.pdf
 *	- https://de.wikipedia.org/wiki/LL(k)-Grammatik
 */

#ifndef __EXPR_PARSER_H__
#define __EXPR_PARSER_H__


#define USE_INTEGER
#ifdef USE_INTEGER
	typedef int t_value;
#else
	typedef double t_value;
#endif


extern void init_symbols();
extern void deinit_symbols();

extern t_value parse(const char* str);
extern void print_symbols();


#endif
