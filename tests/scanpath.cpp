/**
 * quick scan path calculation
 * @author Tobias Weber
 * @date 29-apr-18
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -o scanpath scanpath.cpp -std=c++17 -fconcepts
 */

#include <iostream>

#include "../libs/math_algos.h"
#include "../libs/math_conts.h"
using namespace m;
using namespace m_ops;

using t_real = double;
using t_vec = std::vector<t_real>;
using t_mat = mat<t_real, std::vector>;


static void calc(const t_vec& bragg, const t_vec& orient1, const t_vec& orient2, t_real rot,
	bool bNormalise = true)
{
	t_vec orientUp = m::cross<t_vec>(orient1, orient2);
	t_mat R = m::rotation<t_mat, t_vec>(orientUp, rot/180.*M_PI, 0);

	t_vec vecNewOrient1 = R*orient1;
	t_vec vecNewOrient2 = R*orient2;

	t_real lenOrient1 = bNormalise ? m::inner<t_vec>(vecNewOrient1, vecNewOrient1) : 1;
	t_real lenOrient2 = bNormalise ? m::inner<t_vec>(vecNewOrient2, vecNewOrient2) : 1;

	t_real u = m::inner<t_vec>(bragg, vecNewOrient1) / lenOrient1;
	t_real v = m::inner<t_vec>(bragg, vecNewOrient2) / lenOrient2;

	std::cout << u << " * [" << vecNewOrient1
		<< "] + " << v << " * [" << vecNewOrient2
		<< "]    =    [" << u*vecNewOrient1 + v*vecNewOrient2 
		<< "]" << std::endl;
}


int main(int argc, char **argv)
{
	t_real bragg[3], orient1[3], orient2[3];
	t_real rot;

	std::cout << "Bragg peak: ";
	std::cin >> bragg[0] >> bragg[1] >> bragg[2];
	std::cout << "Orientation vector 1: ";
	std::cin >> orient1[0] >> orient1[1] >> orient1[2];
	std::cout << "Orientation vector 2: ";
	std::cin >> orient2[0] >> orient2[1] >> orient2[2];
	std::cout << "Rotation: ";
	std::cin >> rot;

	t_vec vecBragg = m::create<t_vec>({ bragg[0], bragg[1], bragg[2] });
	t_vec vecOrient1 = m::create<t_vec>({ orient1[0], orient1[1], orient1[2] });
	t_vec vecOrient2 = m::create<t_vec>({ orient2[0], orient2[1], orient2[2] });

	calc(vecBragg, vecOrient1, vecOrient2, rot, true);
	return 0;
}
