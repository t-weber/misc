/**
 * container-agnostic math algorithms
 * @author Tobias Weber
 * @date 9-dec-17
 * @license: see 'LICENSE' file
 *
 * g++ -o math_algos_tst math_algos_tst.cpp -std=c++17 -fconcepts -lQtCore
 */

#include <iostream>
#include "math_algos.h"

#include <vector>
#include <array>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
namespace ublas = boost::numeric::ublas;

#include <boost/type_index.hpp>
namespace ty = boost::typeindex;

#include <QtCore/QVector>
//#include <QtGui/QVector3D>
#include <QtGui/QGenericMatrix>


int main()
{
	// using dynamic STL containers
	{
		using t_real = double;
		using t_vec = std::vector<t_real>;
		using t_mat = ublas::matrix<t_real>;
		std::cout << "Using " << ty::type_id_with_cvr<t_vec>().pretty_name() << "\n";

		t_vec vec1{{1, 2, 3}};
		t_vec vec2{{7, 8, 9}};
		std::cout << inner_prod<t_vec>(vec1, vec2) << "\n";

		t_vec vec3 = zero<t_vec>(3);
		t_mat mat1 = outer_prod<t_mat, t_vec>(vec1, vec2);
		std::cout << mat1 << "\n";
	}

	std::cout << "\n";


	// using static STL containers
	{
		using t_real = double;
		using t_vec = std::array<t_real, 3>;
		using t_mat = ublas::matrix<t_real>;
		std::cout << "Using " << ty::type_id_with_cvr<t_vec>().pretty_name() << "\n";

		t_vec vec1{{1, 2, 3}};
		t_vec vec2{{7, 8, 9}};
		std::cout << inner_prod<t_vec>(vec1, vec2) << "\n";

		t_vec vec3 = zero<t_vec>(3);
		t_mat mat1 = outer_prod<t_mat, t_vec>(vec1, vec2);
		std::cout << mat1 << "\n";
	}

	std::cout << "\n";


	// using Qt classes
	{
		using t_real = float;
		using t_vec = qvec_adapter<int, 3, t_real, QGenericMatrix>;
		using t_mat = qmat_adapter<int, 3, 3, t_real, QGenericMatrix>;
		std::cout << "Using " << ty::type_id_with_cvr<t_vec>().pretty_name() << "\n";

		t_vec vec1, vec2;
		vec1[0] = 1; vec1[1] = 2; vec1[2] = 3;
		vec2[0] = 7; vec2[1] = 8; vec2[2] = 9;

		std::cout << inner_prod<t_vec>(vec1, vec2) << "\n";
		t_mat mat = outer_prod<t_mat, t_vec>(vec1, vec2);
		std::cout << mat(0,0) << " " << mat(0,1) << " " << mat(0,2) << "\n";
		std::cout << mat(1,0) << " " << mat(1,1) << " " << mat(1,2) << "\n";
		std::cout << mat(2,0) << " " << mat(2,1) << " " << mat(2,2) << "\n";

		t_vec vec3 = zero<t_vec>(3);
		vec3[1] = 1;
		vec3[2] = 1;
		t_mat mat3 = zero<t_mat>(3,3);

		t_mat matProj = ortho_projector<t_mat, t_vec>(vec1, 0);
		std::cout << matProj(0,0) << " " << matProj(0,1) << " " << matProj(0,2) << "\n";
		std::cout << matProj(1,0) << " " << matProj(1,1) << " " << matProj(1,2) << "\n";
		std::cout << matProj(2,0) << " " << matProj(2,1) << " " << matProj(2,2) << "\n";


		auto newsys = orthonorm_sys<std::vector, t_vec>({vec1, vec2, vec3});
		for(const auto& vec : newsys)
			std::cout << vec[0] << " " << vec[1] << " " << vec[2] << ", length: " << norm<t_vec>(vec) << "\n";
		std::cout << "v0 * v1 = " << inner_prod<t_vec>(newsys[0], newsys[1]) << "\n";
		std::cout << "v0 * v2 = " << inner_prod<t_vec>(newsys[0], newsys[2]) << "\n";
		std::cout << "v1 * v2 = " << inner_prod<t_vec>(newsys[1], newsys[2]) << "\n";


		auto matRot = rotation<t_mat, t_vec>(vec3, 0.1, 0);
		std::cout << matRot(0,0) << " " << matRot(0,1) << " " << matRot(0,2) << "\n";
		std::cout << matRot(1,0) << " " << matRot(1,1) << " " << matRot(1,2) << "\n";
		std::cout << matRot(2,0) << " " << matRot(2,1) << " " << matRot(2,2) << "\n";
	}

	std::cout << "\n";


	// using ublas classes
	{
		using t_real = double;
		using t_vec = ublas::vector<t_real>;
		using t_mat = ublas::matrix<t_real>;
		std::cout << "Using " << ty::type_id_with_cvr<t_vec>().pretty_name() << "\n";

		t_vec vec1(3), vec2(3);
		vec1[0] = 1; vec1[1] = 2; vec1[2] = 3;
		vec2[0] = 7; vec2[1] = 8; vec2[2] = 9;

		std::cout << inner_prod<t_vec>(vec1, vec2) << "\n";
		t_mat mat = outer_prod<t_mat, t_vec>(vec1, vec2);
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


		std::cout << rotation<t_mat, t_vec>(vec3, 0.1, 0) << "\n";
	}

	return 0;
}
