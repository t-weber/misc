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


/**
 * get total operator of the circuit:
 *
 * qubit 1: ---one_pre_1---|                        |---one_post_1---
 *                         |---two_pre---two_post---|
 * qubit 2: ---one_pre_2---|                        |---one_post_2---
 *
 * with one-qubit operators "one*" and two-qubit operator "two"
 */
template<class t_mat> requires is_mat<t_mat>
static t_mat circuit_total_op(
	const t_mat& one_pre_1, const t_mat& one_pre_2,
	const t_mat& two_pre, const t_mat& two_post,
	const t_mat& one_post_1, const t_mat& one_post_2)
{
	t_mat pre = outer<t_mat>(one_pre_1, one_pre_2);
	t_mat post = outer<t_mat>(one_post_1, one_post_2);

	return (pre * two_pre) * (two_post * post);
}


template<class t_scalar, class t_vec, class t_mat>
requires is_mat<t_mat> && is_vec<t_vec>
void qm_tests()
{
	const t_mat I = unit<t_mat>(2);
	const t_mat& H = hadamard<t_mat>();
	t_vec down = create<t_vec>({ 1, 0 });
	t_vec up = create<t_vec>({ 0, 1 });

	t_mat I_H = outer<t_mat>(I, H);
	t_mat H_I = outer<t_mat>(H, I);

	std::cout << "H H^+ = " << trans(H) * H << std::endl;
	std::cout << "H^+ H = " << H * trans(H) << std::endl;

	std::cout << "\nH x H = " << outer<t_mat>(H, H) << std::endl;
	std::cout << "I x H = " << I_H << std::endl;
	std::cout << "H x I = " << H_I << std::endl;


	t_vec upup = outer_flat<t_vec, t_mat>(up, up);
	t_vec downdown = outer_flat<t_vec, t_mat>(down, down);
	t_vec downup = outer_flat<t_vec, t_mat>(down, up);

	t_vec vec1 = H*up;
	t_vec vec2 = H*down;
	t_vec twobitstate1 = outer_flat<t_vec, t_mat>(up, vec1);
	t_vec twobitstate4b = I_H * upup;

	std::cout << "\nH |up> = " << vec1 << std::endl;
	std::cout << "H |down> = " << vec2 << std::endl;
	std::cout << "|up> x H |up> = " << twobitstate1 << std::endl;
	std::cout << "I x H |up up> = " << twobitstate4b << std::endl;


	t_vec downdowndown = outer_flat<t_vec, t_mat>(downdown, down);
	t_vec downdownup = outer_flat<t_vec, t_mat>(downdown, up);
	t_vec downupdown = outer_flat<t_vec, t_mat>(downup, down);
	t_vec downupup = outer_flat<t_vec, t_mat>(downup, up);

	t_mat H_I_H = outer<t_mat>(outer<t_mat>(H, I), H);
	t_mat H_I_I = outer<t_mat>(outer<t_mat>(H, I), I);
	t_mat I_I_H = outer<t_mat>(outer<t_mat>(I, I), H);

	t_vec threebitstate1b = H_I_H * downdowndown;
	t_vec threebitstate1c = H_I_I * downdowndown;
	t_vec threebitstate2b = H_I_I * downdownup;
	t_vec threebitstate2c = I_I_H * downdownup;

	std::cout << "\n|down down down> = " << downdowndown << std::endl;
	std::cout << "|down down up> = " << downdownup << std::endl;
	std::cout << "|down up down> = " << downupdown << std::endl;
	std::cout << "|down up up> = " << downupup << std::endl;

	std::cout << "\nH x I x H |down down down> = " << threebitstate1b << std::endl;
	std::cout << "H x I x I |down down down> = " << threebitstate1c << std::endl;
	std::cout << "H x I x I |down down up> = " << threebitstate2b << std::endl;
	std::cout << "I x I x H |down down up> = " << threebitstate2c << std::endl;


	t_mat X = su2_matrix<t_mat>(0);
	t_mat Y = su2_matrix<t_mat>(1);
	t_mat Z = su2_matrix<t_mat>(2);
	t_mat C1 = cnot<t_mat>(0);
	t_mat C2 = cnot<t_mat>(1);
	t_mat I4 = unit<t_mat>(4);

	t_mat circ1_op = circuit_total_op<t_mat>(Y, X, C1, I4, X, Y);
	std::cout << "circuit total operator: " << circ1_op << std::endl;

	// see: https://en.wikipedia.org/wiki/Controlled_NOT_gate
	t_mat cnot_flipped_op = circuit_total_op<t_mat>(H, H, C1, I4, H, H);
	std::cout << std::boolalpha << equals<t_mat>(cnot_flipped_op, C2, 1e-6) << std::endl;


	t_mat density1 = outer<t_mat, t_vec>(up, up);
	t_mat density2 = outer<t_mat, t_vec>(down, down);
	t_vec bloch1 = bloch_vector<t_vec, t_mat>(density1);
	t_vec bloch2 = bloch_vector<t_vec, t_mat>(density2);
	std::cout << "\nbloch vector for |up>: " << bloch1 << std::endl;
	std::cout << "bloch vector for |down>: " << bloch2 << std::endl;


	//t_mat phi = phasegate_discrete<t_mat>(1.);
}


int main()
{
	using t_real = double;
	using t_cplx = std::complex<t_real>;
	using t_vec_cplx = vec<t_cplx, std::vector>;
	using t_mat_cplx = mat<t_cplx, std::vector>;

	qm_tests<t_cplx, t_vec_cplx, t_mat_cplx>();
	return 0;
}
