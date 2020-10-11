/**
 * C++20 module test
 * @author Tobias Weber
 * @date oct-20
 * @license: see 'LICENSE.EUPL' file
 */

export module cpp20_mod;
//import <cmath>;


export template<class T> T
module_fact(T t)
{
	if(t <= T{1})
		return T{1};
	return t * module_fact<T>(t-1);
}


export template<class T, class ...t_factors>
T module_prod(t_factors ...fact)
{
	return (fact * ...);
}
