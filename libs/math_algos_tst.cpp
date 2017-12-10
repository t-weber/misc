/**
 * container-agnostic math algorithms
 * @author Tobias Weber
 * @date 9-dec-17
 * @license: see 'LICENSE' file
 *
 * g++ -o math_algos_tst math_algos_tst.cpp -std=c++17 -fconcepts
 */

#include "math_algos.h"

#include <vector>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
namespace ublas = boost::numeric::ublas;

//#include <QtCore/QVector>
//#include <QtGui/QGenericMatrix>


using t_real = double;


int main()
{
	// using STL containers
	{
		using t_vec = std::vector<t_real>;
		t_vec vec1{{1, 2, 3}};
		t_vec vec2{{5, 6, 7}};

		t_real d = inner_prod<t_vec>(vec1, vec2);
		std::cout << d << "\n";

		t_vec vec3 = zero<t_vec>(3);
	}


	// using ublas classes
	{
		using t_vec = ublas::vector<t_real>;
		using t_mat = ublas::matrix<t_real>;

		t_vec vec1(3), vec2(3);
		vec1[0] = 1; vec1[1] = 2; vec1[2] = 3;
		vec2[0] = 7; vec2[1] = 8; vec2[2] = 9;

		t_real d = inner_prod<t_vec>(vec1, vec2);
		t_mat mat = outer_prod<t_mat, t_vec>(vec1, vec2);
		std::cout << d << "\n";
		std::cout << mat << "\n";

		t_vec vec3 = zero<t_vec>(3);
		vec3[1] = 1;
		vec3[2] = 1;
		t_mat mat3 = zero<t_mat>(3,3);

		t_mat matProj = ortho_projector<t_mat, t_vec>(vec1, 0);
		std::cout << matProj << "\n";


		auto newsys = orthonorm_sys<std::vector, t_vec>({vec1, vec2, vec3});
		for(const auto& vec : newsys)
			std::cout << vec << ", length: " << norm<t_vec>(vec) << "\n";
		std::cout << "v0 * v1 = " << inner_prod<t_vec>(newsys[0], newsys[1]) << "\n";
		std::cout << "v0 * v2 = " << inner_prod<t_vec>(newsys[0], newsys[2]) << "\n";
		std::cout << "v1 * v2 = " << inner_prod<t_vec>(newsys[1], newsys[2]) << "\n";
	}

	return 0;
}
