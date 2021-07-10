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


template<class t_scalar, class t_quat, class t_vec>
void quat_tests()
{
	t_quat q1{2., 1., 3., 5.};
	t_vec vec1 = q1.template imag<t_vec>();
	t_quat q1_inv = inv(q1);
	t_quat q1_norm = normalise(q1);

	m_ops::operator<<(std::cout, q1) << std::endl;
	m_ops::operator<<(std::cout, vec1) << std::endl;
	m_ops::operator<<(std::cout, q1_inv) << std::endl;
	m_ops::operator<<(std::cout, q1_norm) << std::endl;
	m_ops::operator<<(std::cout, m::log<t_quat, t_vec>((m::exp<t_quat, t_vec>(q1_norm)))) << std::endl;
	m_ops::operator<<(std::cout, q1*q1_inv) << std::endl;
	m_ops::operator<<(std::cout, q1_inv*q1) << std::endl;
	std::cout << std::boolalpha << m::equals_0<t_quat>(q1) << std::endl;

	t_quat q2 = m::quat_from_rotaxis<t_quat, t_vec>(m::create<t_vec>({1, 2, 1}), 0.123);
	auto [axis2, angle2] = m::quat_to_rotaxis<t_quat, t_vec>(q2);
	m_ops::operator<<(std::cout, axis2) << ", angle: " << angle2 << std::endl;
}


int main()
{
	using t_real = double;
	using t_quat = quat<t_real>;
	using t_vec = vec<t_real, std::vector>;
	//using t_mat = mat<t_real, std::vector>;

	//using t_cplx = std::complex<t_real>;
	//using t_vec_cplx = vec<t_cplx, std::vector>;
	//using t_mat_cplx = mat<t_cplx, std::vector>;

	quat_tests<t_real, t_quat, t_vec>();

	return 0;
}
