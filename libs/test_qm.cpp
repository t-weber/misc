/**
 * tests the container-agnostic math algorithms
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date jun-2021
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o test_qm test_qm.cpp
 */

#include <vector>
#include <iostream>
#include <fstream>

#include "math_algos.h"
#include "math_conts.h"

using namespace m;
using namespace m_ops;


/**
 * get total operator of the circuit:
 *
 * qubit 1: ---one_pre_1---|                              |---one_post_1---
 *                         |---two_pre---two---two_post---|
 * qubit 2: ---one_pre_2---|                              |---one_post_2---
 *
 * with one-qubit operators "one*" and two-qubit operator "two"
 */
template<class t_mat> requires is_mat<t_mat>
static t_mat circuit_total_op(
	const t_mat& one_pre_1, const t_mat& one_pre_2,
	const t_mat& two_pre, const t_mat& two, const t_mat& two_post,
	const t_mat& one_post_1, const t_mat& one_post_2)
{
	t_mat pre = outer<t_mat>(one_pre_1, one_pre_2);
	t_mat post = outer<t_mat>(one_post_1, one_post_2);

	return (post * two_post) * two * (two_pre * pre);
}


template<class t_mat> requires is_mat<t_mat>
static bool check_hadamard(std::size_t n)
{
	t_mat H = hadamard<t_mat>();
	t_mat H1 = H;
	for(std::size_t i=1; i<n; ++i)
		H1 = outer<t_mat>(H1, H);

	t_mat H2 = hadamard<t_mat>(n);

	//operator<<(std::cout, H1) << std::endl;
	//operator<<(std::cout, H2) << std::endl;

	return equals<t_mat>(H1, H2, 1e-6);
}


template<class t_vec> requires is_vec<t_vec>
void write_state(std::ostream& ostr, const t_vec& state)
{
	using t_cplx = typename t_vec::value_type;
	using t_real = typename t_cplx::value_type;

	for(std::size_t i=0; i<state.size(); ++i)
	{
		t_real n = std::norm(state[i]);
		ostr << std::left << std::setw(16) << n << " ";
	}

	ostr << std::endl;
}


/**
 * grover search algorithm
 * @see https://en.wikipedia.org/wiki/Grover%27s_algorithm
 * @see "Effiziente Algorithmen" (2021), Kurs 1684, Fernuni Hagen (https://vu.fernuni-hagen.de/lvuweb/lvu/app/Kurs/01684), pp. 26-31.
 */
template<class t_mat, class t_vec>
requires is_mat<t_mat> && is_vec<t_vec>
static bool check_grover(std::size_t n, std::size_t num_steps, std::size_t idx_to_find)
{
	using t_val = typename t_vec::value_type;

	t_vec down = create<t_vec>({ 1, 0 });
	t_vec state = down;

	for(std::size_t i=0; i<n-1; ++i)
		state = outer_flat<t_vec, t_mat>(state, down);

	t_mat H = hadamard<t_mat>(n);
	state = H * state;
	std::cout << "state: " << state << std::endl;

	t_mat mirror = -ortho_mirror_op<t_mat, t_vec>(state, true);
	//std::cout << "mirror = " << mirror << std::endl;

	t_vec oracle_vec = m::zero<t_vec>(state.size());
	oracle_vec[idx_to_find] = 1;	// index to find
	t_mat mirror_oracle = ortho_mirror_op<t_mat, t_vec>(oracle_vec, true);
	//std::cout << "mirror_oracle = " << mirror_oracle << std::endl;


	std::ofstream ofstr("grover.dat");

	for(std::size_t step=0; step<num_steps; ++step)
	{
		state = mirror_oracle * state;
		std::cout << "\nmirror sign: " << state << std::endl;

		state = mirror * state;
		std::cout << "mirror at mean: " << state << std::endl;

		write_state(ofstr, state);
	}


	// check if the correct index has been recovered
	auto iter_max = std::max_element(state.begin(), state.end(),
		[](const t_val& val1, const t_val& val2) -> bool
			{ return std::norm(val1) < std::norm(val2); });

	return (iter_max - state.begin()) == std::ptrdiff_t(idx_to_find);
}


template<class t_mat, class t_vec>
requires is_mat<t_mat> && is_vec<t_vec>
static bool check_measurements(const t_vec& up, const t_vec& down, const t_vec& twobitstate)
{
	using t_val = typename t_vec::value_type;
	const t_mat I = unit<t_mat>(2);

	// measurement operators
	t_mat up_proj = projector<t_mat, t_vec>(up, false);
	t_mat down_proj = projector<t_mat, t_vec>(down, false);
	t_mat up_proj_bit1 = outer<t_mat>(up_proj, I);
	t_mat down_proj_bit1 = outer<t_mat>(down_proj, I);
	t_mat up_proj_bit2 = outer<t_mat>(I, up_proj);
	t_mat down_proj_bit2 = outer<t_mat>(I, down_proj);

	/*std::cout << "|down><down| x I = " << down_proj_bit1 << std::endl;
	std::cout << "|up><up| x I = " << up_proj_bit1 << std::endl;
	std::cout << "I x |down><down| = " << down_proj_bit2 << std::endl;
	std::cout << "I x |up><up| = " << up_proj_bit2 << std::endl;*/

	// numbering: 0=|down down>, 1=|down up>, 2=|up down>, 3=|up up>
	std::cout << "2-bit state: " << twobitstate << std::endl;

	t_val bit1_down1 = sum(down_proj_bit1*twobitstate);
	t_val bit1_down2 = twobitstate[0] + twobitstate[1];
	t_val bit1_up1 = sum(up_proj_bit1*twobitstate);
	t_val bit1_up2 = twobitstate[2] + twobitstate[3];

	t_val bit2_down1 = sum(down_proj_bit2*twobitstate);
	t_val bit2_down2 = twobitstate[0] + twobitstate[2];
	t_val bit2_up1 = sum(up_proj_bit2*twobitstate);
	t_val bit2_up2 = twobitstate[1] + twobitstate[3];

	bool bit1downok = m::equals<t_val>(bit1_down1, bit1_down2);
	bool bit1upok = m::equals<t_val>(bit1_up1, bit1_up2);
	bool bit2downok = m::equals<t_val>(bit2_down1, bit2_down2);
	bool bit2upok = m::equals<t_val>(bit2_up1, bit2_up2);

	std::cout << "bit1_down: " << bit1_down1 << " == " << bit1_down2 << ": " << bit1downok << std::endl;
	std::cout << "bit1_up: " << bit1_up1 << " == " << bit1_up2 << ": " << bit1upok << std::endl;

	std::cout << "bit2_down: " << bit2_down1 << " == " << bit2_down2 << ": " << bit2downok << std::endl;
	std::cout << "bit2_up: " << bit2_up1 << " == " << bit2_up2 << ": "  << bit2upok << std::endl;

	return bit1downok && bit1upok && bit2downok && bit2upok;
}


template<class t_scalar, class t_vec, class t_mat>
requires is_mat<t_mat> && is_vec<t_vec>
void qm_tests()
{
	const t_mat I = unit<t_mat>(2);
	const t_mat& H = hadamard<t_mat>();
	t_vec down = create<t_vec>({ 1, 0 });
	t_vec up = create<t_vec>({ 0, 1 });

	// measurement operators
	t_mat up_proj = projector<t_mat, t_vec>(up, false);
	t_mat down_proj = projector<t_mat, t_vec>(down, false);
	t_mat up_proj_bit1 =outer<t_mat>(up_proj, I);
	t_mat down_proj_bit1 =outer<t_mat>(down_proj, I);
	t_mat up_proj_bit2 =outer<t_mat>(I, up_proj);
	t_mat down_proj_bit2 =outer<t_mat>(I, down_proj);

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
	t_vec updown = outer_flat<t_vec, t_mat>(up, down);

	t_vec vec1 = H*up;
	t_vec vec2 = H*down;
	t_vec twobitstate1 = outer_flat<t_vec, t_mat>(up, vec1);
	t_vec twobitstate2 = outer_flat<t_vec, t_mat>(down, up);
	t_vec twobitstate4b = I_H * upup;

	std::cout << "\nH |up> = " << vec1 << std::endl;
	std::cout << "H |down> = " << vec2 << std::endl;
	std::cout << "|up><up| |up> = " << up_proj*up << std::endl;
	std::cout << "|down><down| |up> = " << down_proj*up << std::endl;
	std::cout << "|up><up| H |up> = " << up_proj*vec1 << std::endl;
	std::cout << "|down><down| H |up> = " << down_proj*vec1 << std::endl;

	std::cout << "bit1_up: (|up><up| x I) (|up> x H |up>) = " << up_proj_bit1*twobitstate1 << std::endl;
	std::cout << "bit1_down: (|down><down| x I) (|up> x H |up>) = " << down_proj_bit1*twobitstate1 << std::endl;
	std::cout << "bit2_up: (I x |up><up|) (|up> x H |up>) = " << up_proj_bit2*twobitstate1 << std::endl;
	std::cout << "bit2_down: (I x |down><down|) (|up> x H |up>) = " << down_proj_bit2*twobitstate1 << std::endl;

	std::cout << "\n|up> x H |up> = " << twobitstate1 << std::endl;
	std::cout << "I x H |up up> = " << twobitstate4b << std::endl;

	std::cout << "\nmeasurments 1\n" << std::boolalpha
		<< check_measurements<t_mat, t_vec>(up, down, twobitstate1) << std::endl;
	std::cout << "\nmeasurments 2\n" << std::boolalpha
		<< check_measurements<t_mat, t_vec>(up, down, twobitstate2) << std::endl;

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

	t_mat circ1_op = circuit_total_op<t_mat>(Y, X, C1, I4, I4, X, Y);
	std::cout << "circuit total operator: " << circ1_op << std::endl;


	// see: https://en.wikipedia.org/wiki/Controlled_NOT_gate
	t_mat cnot_flipped_op = circuit_total_op<t_mat>(H, H, C1, I4, I4, H, H);
	std::cout << "\n" << std::boolalpha << equals<t_mat>(cnot_flipped_op, C2, 1e-6) << std::endl;


	t_mat U1 = cunitary<t_mat>(Y, 0);
	t_mat U2 = cunitary<t_mat>(Y, 1);
	t_mat U3 = cunitary<t_mat>(X, 1);
	t_mat cunitary_flipped_op = circuit_total_op<t_mat>(H, H, U1, I4, I4, H, H);
	std::cout << "\n" << cunitary_flipped_op << "\n" << U2 << std::endl;
	std::cout << std::boolalpha << equals<t_mat>(cunitary_flipped_op, U2, 1e-6) << ", ";
	std::cout << std::boolalpha << equals<t_mat>(U3, C2, 1e-6) << std::endl;


	// swap state
	// see: (Bronstein08): I. N. Bronstein et al., ISBN: 978-3-8171-2017-8 (2008), Ch. 22 (Zusatzkapitel.pdf), p. 28
	t_mat swap_op = circuit_total_op<t_mat>(I, I, C1, C2, C1, I, I);
	std::cout << "\nSWAP |up down> = " << swap_op * updown;
	std::cout << ", ok = " << std::boolalpha << equals<t_vec>(swap_op * updown, downup, 1e-6) << std::endl;
	std::cout << "SWAP |down up> = " << swap_op * downup;
	std::cout << ", ok = " << std::boolalpha << equals<t_vec>(swap_op * downup, updown, 1e-6) << std::endl;


	// bloch vector
	t_mat density1 = outer<t_mat, t_vec>(up, up);
	t_mat density2 = outer<t_mat, t_vec>(down, down);
	t_vec bloch1 = bloch_vector<t_vec, t_mat>(density1);
	t_vec bloch2 = bloch_vector<t_vec, t_mat>(density2);
	std::cout << "\nbloch vector for |up>: " << bloch1 << std::endl;
	std::cout << "bloch vector for |down>: " << bloch2 << std::endl;


	//t_mat phi = phasegate_discrete<t_mat>(1.);


	std::cout << "\n" << std::boolalpha << check_grover<t_mat, t_vec>(4, 8, 5) << std::endl;
}


int main()
{
	using t_real = double;
	using t_cplx = std::complex<t_real>;
	using t_vec_cplx = vec<t_cplx, std::vector>;
	using t_mat_cplx = mat<t_cplx, std::vector>;

	std::cout << "Hadamard 2x2 ok = " << std::boolalpha << check_hadamard<t_mat_cplx>(1) << std::endl;
	std::cout << "Hadamard 4x4 ok = " << std::boolalpha << check_hadamard<t_mat_cplx>(2) << std::endl;
	std::cout << "Hadamard 8x8 ok = " << std::boolalpha << check_hadamard<t_mat_cplx>(3) << std::endl;
	std::cout << "Hadamard 16x16 ok = " << std::boolalpha << check_hadamard<t_mat_cplx>(4) << std::endl;
	std::cout << std::endl;

	qm_tests<t_cplx, t_vec_cplx, t_mat_cplx>();
	return 0;
}
