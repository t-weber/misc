/**
 * tests the container-agnostic math algorithms
 * @author Tobias Weber
 * @date jul-2021
 * @license: see 'LICENSE.EUPL' file
 *
 * @see references for algorithms:
 * 	- (Kuipers02): J. B. Kuipers, ISBN: 0-691-05872-5 (2002).
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o math_algos_tst3 math_algos_tst3.cpp
 */

#include "math_algos.h"
#include "math_conts.h"

using namespace m;
using namespace m_ops;


template<class t_scalar, class t_quat, class t_vec, class t_mat, class t_mat_cplx>
void quat_tests()
{
	t_scalar eps = 1e-6;

	// basic algebraic properties, see: (Kuipers02), p. 106
	std::cout << "testing basic algebraic properties\n";
	t_quat qi{0., 1., 0., 0.};
	t_quat qj{0., 0., 1., 0.};
	t_quat qk{0., 0., 0., 1.};
	std::cout << std::boolalpha << m::equals<t_quat>(qi*qi, t_quat(-1., 0., 0., 0.), eps) << std::endl;
	std::cout << std::boolalpha << m::equals<t_quat>(qj*qj, t_quat(-1., 0., 0., 0.), eps) << std::endl;
	std::cout << std::boolalpha << m::equals<t_quat>(qk*qk, t_quat(-1., 0., 0., 0.), eps) << std::endl;
	std::cout << std::boolalpha << m::equals<t_quat>(qi*qj*qk, t_quat(-1., 0., 0., 0.), eps) << std::endl;
	std::cout << std::boolalpha << m::equals<t_quat>(qi*qj, qk, eps) << std::endl;
	std::cout << std::boolalpha << m::equals<t_quat>(qj*qk, qi, eps) << std::endl;
	std::cout << std::boolalpha << m::equals<t_quat>(qk*qi, qj, eps) << std::endl;
	std::cout << std::boolalpha << m::equals<t_quat>(qj*qi, -qk, eps) << std::endl;
	std::cout << std::boolalpha << m::equals<t_quat>(qk*qj, -qi, eps) << std::endl;
	std::cout << std::boolalpha << m::equals<t_quat>(qi*qk, -qj, eps) << std::endl;


	t_quat q1{2., 1., 3., 5.};
	t_vec vec1 = q1.template imag<t_vec>();
	t_quat q1_inv = inv(q1);
	t_quat q1_norm = normalise(q1);

	std::cout << "q1 = ";
	m_ops::operator<<(std::cout, q1) << std::endl;
	std::cout << "v1 = ";
	m_ops::operator<<(std::cout, vec1) << std::endl;
	std::cout << "q1_inv = ";
	m_ops::operator<<(std::cout, q1_inv) << std::endl;
	std::cout << "q1_norm = ";
	m_ops::operator<<(std::cout, q1_norm) << std::endl;
	std::cout << "q1/q1 = ";
	m_ops::operator<<(std::cout, q1 / q1) << std::endl;
	std::cout << "log(exp(q1)) = ";
	m_ops::operator<<(std::cout, m::log<t_quat, t_vec>((m::exp<t_quat, t_vec>(q1_norm)))) << std::endl;
	std::cout << "exp(q1)*exp(-q1) = ";
	m_ops::operator<<(std::cout, m::exp<t_quat, t_vec>(q1) * m::exp<t_quat, t_vec>(-q1)) << std::endl;
	std::cout << "q1*q1_inv = ";
	m_ops::operator<<(std::cout, q1*q1_inv) << std::endl;
	std::cout << "q1_inv*q1 = ";
	m_ops::operator<<(std::cout, q1_inv*q1) << std::endl;
	std::cout << "q1 == 0: ";
	std::cout << std::boolalpha << m::equals_0<t_quat>(q1, eps) << std::endl;


	std::cout << "\ncalculating quaternion and rotation matrix representing the same rotation\n";
	t_vec axis2 = m::create<t_vec>({1, 2, 1});
	t_scalar angle2 = 0.123;
	t_quat q2 = m::from_rotaxis<t_quat, t_vec>(axis2, angle2);
	t_mat rot2 = m::rotation<t_mat, t_vec>(axis2, angle2, false);

	std::cout << "q2 = ";
	m_ops::operator<<(std::cout, q2) << std::endl;
	auto [axis2b, angle2b] = m::to_rotaxis<t_quat, t_vec>(q2);
	std::cout << "rotation axis: ";
	m_ops::operator<<(std::cout, axis2b) << ", angle: " << angle2b << std::endl;

	std::cout << "axes equal: " << std::boolalpha
		<< m::equals<t_vec>(axis2/m::norm<t_vec>(axis2), axis2b, eps)
		<< std::endl;
	std::cout << "angles equal: " << std::boolalpha
		<< m::equals<t_scalar>(angle2, angle2b, eps)
		<< std::endl;

	std::cout << "rot2: ";
	m_ops::operator<<(std::cout, rot2) << std::endl;

	// convert quaternion to SO(3) rotation matrix
	t_mat mat2_so3 = m::to_so3<t_quat, t_vec, t_mat>(q2);
	std::cout << "so3:  ";
	m_ops::operator<<(std::cout, mat2_so3) << std::endl;

	std::cout << "\ndirectly calculating the trafo matrix from the canonical basis vector system\n";
	t_vec base1 = m::mult<t_quat, t_vec>(q2, m::create<t_vec>({1, 0, 0}));
	t_vec base2 = m::mult<t_quat, t_vec>(q2, m::create<t_vec>({0, 1, 0}));
	t_vec base3 = m::mult<t_quat, t_vec>(q2, m::create<t_vec>({0, 0, 1}));
	std::cout << "transformed basis vectors:\n";
	m_ops::operator<<(std::cout, base1) << std::endl;
	m_ops::operator<<(std::cout, base2) << std::endl;
	m_ops::operator<<(std::cout, base3) << std::endl;
	bool basis_equal =
		m::equals<t_scalar>(base1[0], mat2_so3(0,0), eps) &&
		m::equals<t_scalar>(base1[1], mat2_so3(1,0), eps) &&
		m::equals<t_scalar>(base1[2], mat2_so3(2,0), eps) &&
		m::equals<t_scalar>(base2[0], mat2_so3(0,1), eps) &&
		m::equals<t_scalar>(base2[1], mat2_so3(1,1), eps) &&
		m::equals<t_scalar>(base2[2], mat2_so3(2,1), eps) &&
		m::equals<t_scalar>(base3[0], mat2_so3(0,2), eps) &&
		m::equals<t_scalar>(base3[1], mat2_so3(1,2), eps) &&
		m::equals<t_scalar>(base3[2], mat2_so3(2,2), eps);
		std::cout << "basis equals so3 matrix: " << std::boolalpha << basis_equal << std::endl;

	std::cout << "\nconvertint quaternion to SU(2) matrix\n";
	t_mat_cplx mat1_su2 = m::to_su2<t_quat, t_vec, t_mat_cplx>(q1_norm);
	std::cout << "su2:  ";
	m_ops::operator<<(std::cout, mat1_su2) << std::endl;

	std::cout << "SO(3) matrices equal: " << std::boolalpha
		<< m::equals<t_mat>(rot2, mat2_so3, eps)
		<< std::endl;


	std::cout << "\nrotating the same vector with a matrix and a quaternion operator\n";
	t_vec vec2 = m::create<t_vec>({1, 2, 3});
	t_vec vec2_rot1 = rot2 * vec2;
	t_vec vec2_rot2 = q2 * vec2;
	std::cout << "result: ";
	m_ops::operator<<(std::cout, vec2_rot1) << std::endl;

	std::cout << "rotated vectors equal: " << std::boolalpha
		<< m::equals<t_vec>(vec2_rot1, vec2_rot2, eps)
		<< std::endl;

	// direct calculation
	t_vec axis2_n = axis2/m::norm<t_vec>(axis2);
	t_vec vec2_rot1b = (1 - std::cos(angle2)) * m::inner<t_vec>(axis2_n, vec2)*axis2_n +
		vec2 * std::cos(angle2) + m::cross<t_vec>({axis2_n, vec2}) * std::sin(angle2);
	//m_ops::operator<<(std::cout, vec2_rot1b) << std::endl;

	/*t_scalar c2 = std::cos(0.5*angle2);
	t_scalar s2 = std::sin(0.5*angle2);
	t_vec vec2_rot2b = - m::cross<t_vec>({m::cross<t_vec>({axis2_n, vec2}), axis2_n}) * s2*s2
		- 2. * m::cross<t_vec>({vec2, axis2_n}) * c2*s2
		+ m::inner<t_vec>(axis2_n, vec2) * axis2_n * s2*s2
		+ vec2 * c2*c2
		//+ 0.5*vec2 + 0.5 * vec2 * std::cos(angle2)
		;*/

	t_vec vec2_rot2b =
		0.5 * (- m::cross<t_vec>({m::cross<t_vec>({axis2_n, vec2}), axis2_n}) + m::inner<t_vec>(axis2_n, vec2) * axis2_n)
		- 0.5 * (- m::cross<t_vec>({m::cross<t_vec>({axis2_n, vec2}), axis2_n}) + m::inner<t_vec>(axis2_n, vec2) * axis2_n) * std::cos(angle2)
		- m::cross<t_vec>({vec2, axis2_n}) * std::sin(angle2)
		+ 0.5*vec2 + 0.5 * vec2 * std::cos(angle2)
		;
	//m_ops::operator<<(std::cout, vec2_rot2b) << std::endl;

	std::cout << "rotated vectors equal with direct calculation 1: " << std::boolalpha
		<< m::equals<t_vec>(vec2_rot1, vec2_rot1b, eps)
		<< std::endl;
	std::cout << "rotated vectors equal with direct calculation 2: " << std::boolalpha
		<< m::equals<t_vec>(vec2_rot2, vec2_rot2b, eps)
		<< std::endl;

	//m_ops::operator<<(std::cout, m::mult<t_mat, t_vec>(rot2, vec2)) << std::endl;
	//m_ops::operator<<(std::cout, m::mult<t_quat, t_vec>(q2, vec2)) << std::endl;


	std::cout << "\ntesting slerp\n";
	for(t_scalar t=0.; t<=1; t+=0.25)
	{
		t_quat qs = slerp<t_quat, t_vec>(q1_norm, q2, t);

		std::cout << "slerp(q1, q2, " << t << ") = ";
		m_ops::operator<<(std::cout, qs) << std::endl;
	}
}


int main()
{
	using t_real = double;
	using t_quat = quat<t_real>;
	using t_vec = vec<t_real, std::vector>;
	using t_mat = mat<t_real, std::vector>;

	using t_cplx = std::complex<t_real>;
	//using t_vec_cplx = vec<t_cplx, std::vector>;
	using t_mat_cplx = mat<t_cplx, std::vector>;

	quat_tests<t_real, t_quat, t_vec, t_mat, t_mat_cplx>();

	return 0;
}
