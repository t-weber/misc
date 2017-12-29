/**
 * container-agnostic math algorithms
 * @author Tobias Weber
 * @date 9-dec-17
 * @license: see 'LICENSE' file
 *
 * g++ -fPIC -I/usr/include/qt5 -o math_algos_tst math_algos_tst.cpp -std=c++17 -fconcepts -lQt5Core
 */

#include <iostream>
#include "math_algos.h"
using namespace m;

#include <vector>
#include <array>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
namespace ublas = boost::numeric::ublas;

#include <boost/type_index.hpp>
namespace ty = boost::typeindex;

#include <QtCore/QVector>
#include <QtGui/QGenericMatrix>
#include <QtGui/QMatrix4x4>
#include <QtGui/QVector4D>


template<class t_vec, class t_mat>
void vecmat_tsts()
{
	using t_real = typename t_vec::value_type;
	std::cout << "Using " 
		<< "t_vec = " << ty::type_id_with_cvr<t_vec>().pretty_name() << ", "
		<< "t_mat = " << ty::type_id_with_cvr<t_vec>().pretty_name() << "\n";

	t_vec vec1 = create<t_vec>({1, 2, 3}),
		vec2 = create<t_vec>({7, 8, 9});

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

	mat3(0,0) = 1; mat3(0,1) = 2; mat3(0,2) = 3;
	mat3(1,0) = 1; mat3(1,1) = 2; mat3(1,2) = 2;
	mat3(2,0) = 3; mat3(2,1) = 2; mat3(2,2) = 1;
	std::cout << "det = " << det<t_mat>(mat3) << "\n";


	auto newsys = orthonorm_sys<std::vector, t_vec>({vec1, vec2, vec3});
	for(const auto& vec : newsys)
		std::cout << vec[0] << " " << vec[1] << " " << vec[2] << ", length: " << norm<t_vec>(vec) << "\n";
	std::cout << "v0 * v1 = " << inner_prod<t_vec>(newsys[0], newsys[1]) << "\n";
	std::cout << "v0 * v2 = " << inner_prod<t_vec>(newsys[0], newsys[2]) << "\n";
	std::cout << "v1 * v2 = " << inner_prod<t_vec>(newsys[1], newsys[2]) << "\n";


	std::cout << "\nrotation\n";
	auto matRot = rotation<t_mat, t_vec>(create<t_vec>({1,1,1}), 0.1, 0);
	std::cout << matRot(0,0) << " " << matRot(0,1) << " " << matRot(0,2) << "\n";
	std::cout << matRot(1,0) << " " << matRot(1,1) << " " << matRot(1,2) << "\n";
	std::cout << matRot(2,0) << " " << matRot(2,1) << " " << matRot(2,2) << "\n";


	std::cout << "\nproject_plane\n";
	t_vec vecNorm = create<t_vec>({0, 1, 0});
	t_real d = 5.;
	t_vec vecPlane = ortho_project_plane<t_vec>(vec1, vecNorm, d);
	std::cout << vecPlane[0] << " "  << vecPlane[1]  << " " << vecPlane[2] << "\n";
	vecPlane = ortho_mirror_plane<t_vec>(vec1, vecNorm, d);
	std::cout << vecPlane[0] << " "  << vecPlane[1]  << " " << vecPlane[2] << "\n";


	std::cout << "\nproject_line\n";
	t_vec lineOrigin = create<t_vec>({10., 20., 30.});
	t_vec lineDir = create<t_vec>({0., 1., 0.});
	t_vec vecPos = create<t_vec>({1., 2., 3.});
	t_vec vecLineProj = project_line<t_vec>(vecPos, lineOrigin, lineDir, 0);
	std::cout << vecLineProj[0] << " "  << vecLineProj[1]  << " " << vecLineProj[2] << "\n";

	std::cout << "dist pt-line: " << norm<t_vec>(vecPos-vecLineProj) << "\n";
	std::cout << "dist pt-line (direct): " << 
		norm<t_vec>(cross_prod<t_vec>(vecPos - lineOrigin, lineDir)) / norm<t_vec>(lineDir)
		<< "\n";


	std::cout << "\ncreate\n";
	t_mat matCreated = create<t_mat>({{1,2}, {3,4}});
	std::cout << matCreated(0,0) << " " << matCreated(0,1) << "\n";
	std::cout << matCreated(1,0) << " " << matCreated(1,1) << "\n";


	std::cout << "\nequals\n";
	std::cout << std::boolalpha << equals<t_vec>(create<t_vec>({1,2,3}), create<t_vec>({1,2,3})) << "\n";
	std::cout << std::boolalpha << equals<t_vec>(create<t_vec>({1,2,3.1}), create<t_vec>({1,2,3})) << "\n";
	std::cout << std::boolalpha << equals<t_mat>(create<t_mat>({{1,2}, {3,4}}), create<t_mat>({{1,2}, {3,4}})) << "\n";
	std::cout << std::boolalpha << equals<t_mat>(create<t_mat>({{1,2}, {3.1,4}}), create<t_mat>({{1,2}, {3,4}})) << "\n";
}


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
	
		std::cout << "\n\n";
	}


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

		std::cout << "\n\n";
	}


	// using Qt classes
	{
		using t_real = float;
		using t_vec = qvec_adapter<int, 3, t_real, QGenericMatrix>;
		using t_mat = qmat_adapter<int, 3, 3, t_real, QGenericMatrix>;

		vecmat_tsts<t_vec, t_mat>();
		std::cout << "\n\n";
	}


	// using specialised Qt classes
	{
		using t_vec = qvecN_adapter<int, 4, float, QVector4D>;
		using t_mat = qmatNN_adapter<int, 4, 4, float, QMatrix4x4>;

		vecmat_tsts<t_vec, t_mat>();
		std::cout << "\n\n";
	}


	// using ublas classes
	{
		using t_real = double;
		using t_vec = ublas::vector<t_real>;
		using t_mat = ublas::matrix<t_real>;

		vecmat_tsts<t_vec, t_mat>();
		std::cout << "\n\n";
	}

	return 0;
}
