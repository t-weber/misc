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
	const t_mat I = unit<t_mat>(2);
	const t_mat& H = hadamard<t_mat>();
	t_vec down = m::create<t_vec>({ 1, 0 });
	t_vec up = m::create<t_vec>({ 0, 1 });

	t_mat I_H = outer<t_mat>(I, H);
	t_mat H_I = outer<t_mat>(H, I);

	std::cout << "H H^+ = " << m::trans(H) * H << std::endl;
	std::cout << "H^+ H = " << H * m::trans(H) << std::endl;

	std::cout << "\nH x H = " << outer<t_mat>(H, H) << std::endl;
	std::cout << "I x H = " << I_H << std::endl;
	std::cout << "H x I = " << H_I << std::endl;

	t_vec upup = m::outer_flat<t_vec, t_mat>(up, up);
	t_vec downdown = m::outer_flat<t_vec, t_mat>(down, down);
	t_vec downup = m::outer_flat<t_vec, t_mat>(down, up);

	t_vec vec1 = H*up;
	t_vec vec2 = H*down;
	t_vec twobitstate1 = m::outer_flat<t_vec, t_mat>(up, vec1);
	t_vec twobitstate4b = I_H * upup;

	std::cout << "\nH |up> = " << vec1 << std::endl;
	std::cout << "H |down> = " << vec2 << std::endl;
	std::cout << "|up> ^ H |up> = " << twobitstate1 << std::endl;
	std::cout << "I^H |up up> = " << twobitstate4b << std::endl;

	t_vec downdowndown = m::outer_flat<t_vec, t_mat>(downdown, down);
	t_vec downdownup = m::outer_flat<t_vec, t_mat>(downdown, up);
	t_vec downupdown = m::outer_flat<t_vec, t_mat>(downup, down);
	t_vec downupup = m::outer_flat<t_vec, t_mat>(downup, up);

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

	std::cout << "\nH^I^H |down down down> = " << threebitstate1b << std::endl;
	std::cout << "H^I^I |down down down> = " << threebitstate1c << std::endl;
	std::cout << "H^I^I |down down up> = " << threebitstate2b << std::endl;
	std::cout << "I^I^H |down down up> = " << threebitstate2c << std::endl;
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
