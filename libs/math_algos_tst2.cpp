/**
 * tests the container-agnostic math algorithms
 * @author Tobias Weber
 * @date mar-2021
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -o math_algos_tst2 math_algos_tst2.cpp -std=c++20
 */

#include <vector>
#include <iostream>

#include "math_algos.h"
#include "math_conts.h"
using namespace m;
using namespace m_ops;


template<class t_scalar, class t_vec, class t_mat>
void det_tests()
{
	// compare: LinearAlgebra.det([1 23 4; 5 -3 23; 9 -3 -4])  ->  5350
	t_mat mat1 = create<t_mat>({1, 23, 4,  5, -3, 23,  9, -3, -4});
	t_scalar det1 = det<t_mat, t_vec>(mat1);
	std::cout << "det = " << det1 << ", ok = "
		<< equals<t_scalar>(det1, 5350, 1e-4) << std::endl;

	// compare: LinearAlgebra.det([1 23 4 3; 5 -3 23 4; 9 3 -4 -10; -3 4 1 -2])

	t_mat mat2 = create<t_mat>({1, 23, 4, 3,  5, -3, 23, 4,  9, 3, -4, -10,  -3, 4, 1, -2});
	t_scalar det2 = det<t_mat, t_vec>(mat2);
	std::cout << "det = " << det2 << ", ok = "
		<< equals<t_scalar>(det2, -30485., 1e-4) << std::endl;
}


template<class t_mat1, class t_mat2>
void conv_tests()
{
	t_mat1 mat1 = create<t_mat1>({1, 23, 4,  5, -3, 23,  9, -3, -4});
	t_mat2 mat2 = convert<t_mat2, t_mat1>(mat1);

	std::cout << mat1 << std::endl;
	std::cout << mat2 << std::endl;
}


template<class t_mat, class t_vec>
void qr_tests()
{
	t_mat mat = create<t_mat>({1, 23, 4,  5, -3, 23,  9, -3, -4});
	auto [Q, R, num_mirr] = qr<t_mat, t_vec>(mat);
	auto Q2 = orthonorm<t_mat, t_vec>(mat);

	std::cout << "M = " << mat << std::endl;
	std::cout << "Q = " << Q << std::endl;
	std::cout << "Q2 = " << Q2 << std::endl;
	std::cout << "R = " << R << std::endl;
	std::cout << "QR = " << Q*R << std::endl;
}


int main()
{
	using t_real = double;
	using t_cplx = std::complex<t_real>;
	using t_vec = vec<t_real, std::vector>;
	using t_mat = mat<t_real, std::vector>;
	using t_vec_cplx = vec<t_cplx, std::vector>;
	using t_mat_cplx = mat<t_cplx, std::vector>;

	det_tests<t_real, t_vec, t_mat>();
	det_tests<t_cplx, t_vec_cplx, t_mat_cplx>();

	conv_tests<t_mat, t_mat_cplx>();
	qr_tests<t_mat, t_vec>();

	return 0;
}
