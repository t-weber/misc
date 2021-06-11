/**
 * tests the container-agnostic math algorithms
 * @author Tobias Weber
 * @date jun-2021
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o math_algos_tst3 math_algos_tst3.cpp
 */

#include <vector>
#include <iostream>

#include "math_algos.h"
#include "math_conts.h"
using namespace m;
using namespace m_ops;


template<class t_scalar, class t_vec, class t_mat>
void qm_tests()
{
	const t_mat& H = hadamard<t_mat>();
	std::cout << "H H^+ = " << m::trans(H) * H << std::endl;
	std::cout << "H^+ H = " << H * m::trans(H) << std::endl;

	t_vec up = m::create<t_vec>({ 1, 0 });
	t_vec down = m::create<t_vec>({ 0, 1 });
	std::cout << "H |up> = " << H*up << std::endl;
	std::cout << "H |down> = " << H*down << std::endl;
}


int main()
{
	using t_real = double;
	using t_cplx = std::complex<t_real>;
	//using t_vec = vec<t_real, std::vector>;
	//using t_mat = mat<t_real, std::vector>;
	using t_vec_cplx = vec<t_cplx, std::vector>;
	using t_mat_cplx = mat<t_cplx, std::vector>;

	qm_tests<t_cplx, t_vec_cplx, t_mat_cplx>();
	return 0;
}
