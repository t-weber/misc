/**
 * parser test -- runtime library
 * @author Tobias Weber
 * @date 17-apr-20
 * @license: see 'LICENSE.GPL' file
 */

#include <stdio.h>
#include <cstdint>

using t_real = double;
using t_int = std::int64_t;


extern "C" t_real ext_determinant(t_real* M, t_int m, t_int n)
{
	// TODO
	return 0.;
}
