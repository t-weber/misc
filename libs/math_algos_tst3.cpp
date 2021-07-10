/**
 * tests the container-agnostic math algorithms
 * @author Tobias Weber
 * @date jul-2021
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o math_algos_tst3 math_algos_tst3.cpp
 */

#include "math_algos.h"
#include "math_conts.h"

using namespace m;
using namespace m_ops;


template<class t_scalar, class t_quat>
void quat_tests()
{
	t_quat q1{2., 1., 3., 5.};
	t_quat q1_inv = inv(q1);
	t_quat q1_norm = normalise(q1);

	m_ops::operator<<(std::cout, q1) << std::endl;
	m_ops::operator<<(std::cout, q1_inv) << std::endl;
	m_ops::operator<<(std::cout, q1_norm) << std::endl;
	m_ops::operator<<(std::cout, q1*q1_inv) << std::endl;
	m_ops::operator<<(std::cout, q1_inv*q1) << std::endl;
}


int main()
{
	using t_real = double;
	using t_quat = quat<t_real>;
	//using t_vec = vec<t_real, std::vector>;
	//using t_mat = mat<t_real, std::vector>;

	//using t_cplx = std::complex<t_real>;
	//using t_vec_cplx = vec<t_cplx, std::vector>;
	//using t_mat_cplx = mat<t_cplx, std::vector>;

	quat_tests<t_real, t_quat>();

	return 0;
}
