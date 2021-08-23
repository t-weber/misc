/**
 * tests the container-agnostic math algorithms
 * @author Tobias Weber
 * @date aug-2021
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o math_algos_tst4 math_algos_tst4.cpp
 */

#include "math_algos.h"
#include "math_conts.h"


template<class t_scalar, class t_vec, class t_mat>
void persp_tests()
{
	t_mat P = m::hom_perspective_no_normalisation<t_mat>(5.);
	//auto [Pinv, bInvExists] = m::inv<t_mat, t_vec>(P);

	std::cout << "P      = ";
	m_ops::operator<<(std::cout, P);
	std::cout << std::endl;

	/*std::cout << "P^(-1) = ";
	m_ops::operator<<(std::cout, Pinv);
	std::cout << std::endl;*/
}


int main()
{
	using t_real = double;
	using t_vec = m::vec<t_real, std::vector>;
	using t_mat = m::mat<t_real, std::vector>;

	persp_tests<t_real, t_vec, t_mat>();
	return 0;
}
