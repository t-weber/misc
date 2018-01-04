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


template<class T>
ublas::vector<T> operator*(const ublas::matrix<T>& mat, const ublas::vector<T>& vec)
{
	return ublas::prod(mat, vec);
}

template<class T>
ublas::matrix<T> operator*(const ublas::matrix<T>& m1, const ublas::matrix<T>& m2)
{
	return ublas::prod(m1, m2);
}


template<class t_vec, class t_mat>
void vecmat_tsts()
{
	using t_real = typename t_vec::value_type;
	std::cout << "Using " 
		<< "t_vec = " << ty::type_id_with_cvr<t_vec>().pretty_name() << ", "
		<< "t_mat = " << ty::type_id_with_cvr<t_mat>().pretty_name() << "\n";

	t_vec vec1 = create<t_vec>({1, 2, 3}),
		vec2 = create<t_vec>({7, 8, 9});

	std::cout << inner<t_vec>(vec1, vec2) << "\n";
	t_mat mat = outer<t_mat, t_vec>(vec1, vec2);
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

	auto [matInv, bInvExists] = inv<t_mat>(mat3);
	std::cout << "\ninverse: " << std::boolalpha << bInvExists << "\n";
	std::cout << matInv(0,0) << " " << matInv(0,1) << " " << matInv(0,2) << "\n";
	std::cout << matInv(1,0) << " " << matInv(1,1) << " " << matInv(1,2) << "\n";
	std::cout << matInv(2,0) << " " << matInv(2,1) << " " << matInv(2,2) << "\n";

	t_mat matE = mat3*matInv;
	std::cout << matE(0,0) << " " << matE(0,1) << " " << matE(0,2) << "\n";
	std::cout << matE(1,0) << " " << matE(1,1) << " " << matE(1,2) << "\n";
	std::cout << matE(2,0) << " " << matE(2,1) << " " << matE(2,2) << "\n";
	std::cout << "\n";


	auto newsys = orthonorm_sys<std::vector, t_vec>({vec1, vec2, vec3});
	for(const auto& vec : newsys)
		std::cout << vec[0] << " " << vec[1] << " " << vec[2] << ", length: " << norm<t_vec>(vec) << "\n";
	std::cout << "v0 * v1 = " << inner<t_vec>(newsys[0], newsys[1]) << "\n";
	std::cout << "v0 * v2 = " << inner<t_vec>(newsys[0], newsys[2]) << "\n";
	std::cout << "v1 * v2 = " << inner<t_vec>(newsys[1], newsys[2]) << "\n";


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


	std::cout << "\nmirror\n";
	t_real a=1.23, b=23, c=4;
	t_vec vecToMirror = create<t_vec>({a, b, c});
	{	// mirror to [x 0 0]
		//t_vec vecMirror = vecToMirror - create<t_vec>({std::sqrt(a*a + b*b + c*c), 0, 0});
		//t_mat opMirror = ortho_mirror_op<t_mat, t_vec>(vecMirror, false);
		t_mat opMirror = ortho_mirror_zero_op<t_mat, t_vec>(vecToMirror, 0);
		t_vec vecMirrored = opMirror * vecToMirror;
		std::cout << vecMirrored[0] << " "  << vecMirrored[1]  << " " << vecMirrored[2] << "\n";
	}
	{ // mirror to [x y 0]
		//t_vec vecMirror = vecToMirror - create<t_vec>({a, std::sqrt(b*b + c*c), 0});
		//t_mat opMirror = ortho_mirror_op<t_mat, t_vec>(vecMirror, false);
		t_mat opMirror = ortho_mirror_zero_op<t_mat, t_vec>(vecToMirror, 1);
		t_vec vecMirrored = opMirror * vecToMirror;
		std::cout << vecMirrored[0] << " "  << vecMirrored[1]  << " " << vecMirrored[2] << "\n";
	}
	{ // mirror to [0 y 0]
		t_vec vecMirror = vecToMirror - create<t_vec>({0, std::sqrt(a*a + b*b + c*c), 0});
		t_mat opMirror = ortho_mirror_op<t_mat, t_vec>(vecMirror, false);
		t_vec vecMirrored = opMirror * vecToMirror;
		std::cout << vecMirrored[0] << " "  << vecMirrored[1]  << " " << vecMirrored[2] << "\n";
	}



	std::cout << "\nproject_line\n";
	t_vec lineOrigin = create<t_vec>({10., 20., 30.});
	t_vec lineDir = create<t_vec>({0., 1., 0.});
	t_vec vecPos = create<t_vec>({1., 2., 3.});
	auto [vecLineProj, lineDist] = project_line<t_vec>(vecPos, lineOrigin, lineDir, 0);
	std::cout << vecLineProj[0] << " "  << vecLineProj[1]  << " " << vecLineProj[2]
		<< ", dist: " << lineDist << "\n";

	std::cout << "dist pt-line: " << norm<t_vec>(vecPos-vecLineProj) << "\n";
	std::cout << "dist pt-line (direct): " << 
		norm<t_vec>(cross<t_vec>(vecPos - lineOrigin, lineDir)) / norm<t_vec>(lineDir)
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


	std::cout << "\nintersect_line_plane\n";
	auto [vecInters, bInters, lamInters] =
		intersect_line_plane<t_vec>(create<t_vec>({1,3,-5}), create<t_vec>({0,0,1}), 
			create<t_vec>({0,0,1}), 10);
	std::cout << std::boolalpha << bInters << ", " << vecInters[0] << " " << vecInters[1] << " " << vecInters[2] << "\n";

	std::cout << "\nintersect_line_line\n";
	t_vec line1[] = { create<t_vec>({0,1,0}), create<t_vec>({0,0,1}) };
	t_vec line2[] = { create<t_vec>({0,-1,0}), create<t_vec>({0.1,1,0.}) };
	auto [pt1, pt2, bValid, distLines, lam1, lam2] = m::intersect_line_line<t_vec>(
		line1[0], line1[1], line2[0], line2[1]);
	std::cout << std::boolalpha << bValid << ",  " << pt1[0] << " " << pt1[1] << " " << pt1[2] << ",  "
		<< pt2[0] << " " << pt2[1] << " " << pt2[2] << ",  dist: " << distLines << "\n";
	std::cout << "dist line-line (direct): " << det<t_mat>(create<t_mat, t_vec>({t_vec(line1[0]-line2[0]), line1[1], line2[1]}))
		/ norm<t_vec>(cross<t_vec>(line1[1], line2[1])) << "\n";

	std::cout << "\nintersect_plane_plane\n";
	m::intersect_plane_plane<t_vec, t_mat>(
		create<t_vec>({0,0,1}), 0,
		create<t_vec>({0,1,0}), 5);


	std::cout << "\nQR\n";
	t_mat matOrg = create<t_mat>({1, 23, 4,  5, -3, 23,  9, -3, -4});
	auto [Q, R] = qr<t_mat, t_vec>(matOrg);
	std::cout << Q(0,0) << " " << Q(0,1) << " " << Q(0,2) << "\n";
	std::cout << Q(1,0) << " " << Q(1,1) << " " << Q(1,2) << "\n";
	std::cout << Q(2,0) << " " << Q(2,1) << " " << Q(2,2) << "\n";	
	std::cout << R(0,0) << " " << R(0,1) << " " << R(0,2) << "\n";
	std::cout << R(1,0) << " " << R(1,1) << " " << R(1,2) << "\n";
	std::cout << R(2,0) << " " << R(2,1) << " " << R(2,2) << "\n";
	t_mat QR = Q*R;
	std::cout << QR(0,0) << " " << QR(0,1) << " " << QR(0,2) << "\n";
	std::cout << QR(1,0) << " " << QR(1,1) << " " << QR(1,2) << "\n";
	std::cout << QR(2,0) << " " << QR(2,1) << " " << QR(2,2) << "\n";
	std::cout << std::boolalpha << equals<t_mat>(matOrg, QR, 0.01) << "\n";
}


template<class t_vec, class t_mat>
void vecmat_tsts_hom()
{
	std::cout << "\nviewport\n";
	t_mat matVP = hom_viewport<t_mat>(800., 600.);
	std::cout << matVP(0,0) << " " << matVP(0,1) << " " << matVP(0,2) << " " << matVP(0,3) << "\n";
	std::cout << matVP(1,0) << " " << matVP(1,1) << " " << matVP(1,2) << " " << matVP(1,3) << "\n";
	std::cout << matVP(2,0) << " " << matVP(2,1) << " " << matVP(2,2) << " " << matVP(2,3) << "\n";
	std::cout << matVP(3,0) << " " << matVP(3,1) << " " << matVP(3,2) << " " << matVP(3,3) << "\n";


	std::cout << "\nperspective\n";
	t_mat matPersp = hom_perspective<t_mat>();
	std::cout << matPersp(0,0) << " " << matPersp(0,1) << " " << matPersp(0,2) << " " << matPersp(0,3) << "\n";
	std::cout << matPersp(1,0) << " " << matPersp(1,1) << " " << matPersp(1,2) << " " << matPersp(1,3) << "\n";
	std::cout << matPersp(2,0) << " " << matPersp(2,1) << " " << matPersp(2,2) << " " << matPersp(2,3) << "\n";
	std::cout << matPersp(3,0) << " " << matPersp(3,1) << " " << matPersp(3,2) << " " << matPersp(3,3) << "\n";

	auto fktProj = [&matPersp](const t_vec& vec1)
	{
		t_vec vec1p = matPersp * vec1;
		t_vec vec1p_div = vec1p / vec1p[3];
		std::cout << vec1[0] << " " << vec1[1] << " " << vec1[2] << " " << vec1[3] << "  ->  ";
		std::cout << vec1p[0] << " " << vec1p[1] << " " << vec1p[2] << " " << vec1p[3] << "  ->  ";
		std::cout << vec1p_div[0] << " " << vec1p_div[1] << " " << vec1p_div[2] << "\n";
	};

	fktProj(create<t_vec>({0, 0, -0.01, 1}));
	fktProj(create<t_vec>({0, 1, -0.01, 1}));
	fktProj(create<t_vec>({1, 1, -0.01, 1}));
	fktProj(create<t_vec>({0, 0, -100, 1}));
	fktProj(create<t_vec>({0, 1, -100, 1}));
	fktProj(create<t_vec>({1, 1, -100, 1}));
}


template<class t_vec, class t_mat>
void vecmat_tsts_nonsquare()
{
	{
		std::cout << "\nQR -- non-square matrix\n";
		t_mat matOrg = create<t_mat>({{1, 23},  {5, -3},  {9, -3}});
		auto [Q, R] = qr<t_mat, t_vec>(matOrg);
		t_mat QR = Q*R;
		std::cout << "org = " << matOrg << "\n";
		std::cout << "Q = " << Q << "\n";
		std::cout << "R = " << R << "\n";
		std::cout << "QR = " << QR << "; " << std::boolalpha << equals<t_mat>(matOrg, QR, 0.01) << "\n";
	}

	{
		std::cout << "\nQR -- special case 1\n";
		t_mat matOrg = create<t_mat>({{3.4, 0},  {5, -3},  {9, -3}});
		auto [Q, R] = qr<t_mat, t_vec>(matOrg);
		t_mat QR = Q*R;
		std::cout << "org = " << matOrg << "\n";
		std::cout << "Q = " << Q << "\n";
		std::cout << "R = " << R << "\n";
		std::cout << "QR = " << QR << "; " << std::boolalpha << equals<t_mat>(matOrg, QR, 0.01) << "\n";
	}


	{
		std::cout << "\nQR -- special case 2\n";
		t_mat matOrg = create<t_mat>({{0, 0, 0}, {3.4, 4, 3},  {5, -3, -1},  {9, -3, 2}});
		auto [Q, R] = qr<t_mat, t_vec>(matOrg);
		t_mat QR = Q*R;
		std::cout << "org = " << matOrg << "\n";
		std::cout << "Q = " << Q << "\n";
		std::cout << "R = " << R << "\n";
		std::cout << "QR = " << QR << "; " << std::boolalpha << equals<t_mat>(matOrg, QR, 0.01) << "\n";
	}
}


int main()
{
	constexpr bool bUseSTL = 0;
	constexpr bool bUseQt = 0;
	constexpr bool bUseUblas = 1;

	// using dynamic STL containers
	if constexpr(bUseSTL)
	{
		using t_real = double;
		using t_vec = std::vector<t_real>;
		using t_mat = ublas::matrix<t_real>;
		std::cout << "Using " << ty::type_id_with_cvr<t_vec>().pretty_name() << "\n";

		t_vec vec1{{1, 2, 3}};
		t_vec vec2{{7, 8, 9}};
		std::cout << inner<t_vec>(vec1, vec2) << "\n";

		t_vec vec3 = zero<t_vec>(3);
		t_mat mat1 = outer<t_mat, t_vec>(vec1, vec2);
		std::cout << mat1 << "\n";
	
		std::cout << "----------------------------------------\n";
		std::cout << "\n\n";
	}


	// using static STL containers
	if constexpr(bUseSTL && bUseUblas)
	{
		using t_real = double;
		using t_vec = std::array<t_real, 3>;
		using t_mat = ublas::matrix<t_real>;
		std::cout << "Using " << ty::type_id_with_cvr<t_vec>().pretty_name() << "\n";

		t_vec vec1{{1, 2, 3}};
		t_vec vec2{{7, 8, 9}};
		std::cout << inner<t_vec>(vec1, vec2) << "\n";

		t_vec vec3 = zero<t_vec>(3);
		t_mat mat1 = outer<t_mat, t_vec>(vec1, vec2);
		std::cout << mat1 << "\n";

		std::cout << "----------------------------------------\n";
		std::cout << "\n\n";
	}


	// using Qt classes
	if constexpr(bUseQt)
	{
		using t_real = float;
		using t_vec = qvec_adapter<int, 3, t_real, QGenericMatrix>;
		using t_mat = qmat_adapter<int, 3, 3, t_real, QGenericMatrix>;

		vecmat_tsts<t_vec, t_mat>();
		std::cout << "----------------------------------------\n";
		std::cout << "\n\n";
	}


	// using specialised Qt classes
	if constexpr(bUseQt)
	{
		using t_vec = qvecN_adapter<int, 4, float, QVector4D>;
		using t_mat = qmatNN_adapter<int, 4, 4, float, QMatrix4x4>;

		vecmat_tsts<t_vec, t_mat>();
		vecmat_tsts_hom<t_vec, t_mat>();

		auto [matInv, bInvExists] = inv<t_mat>(create<t_mat>(
		{
			1, 2, 3, 4,
			4, 3, 2, 1,
			1, 0, 1, 1,
			1, 5, 9, 10
		}));
		std::cout << "\ninverse 2: " << std::boolalpha << bInvExists << "\n";
		std::cout << matInv(0,0) << " " << matInv(0,1) << " " << matInv(0,2) << " " << matInv(0,3) << "\n";
		std::cout << matInv(1,0) << " " << matInv(1,1) << " " << matInv(1,2) << " " << matInv(1,3) << "\n";
		std::cout << matInv(2,0) << " " << matInv(2,1) << " " << matInv(2,2) << " " << matInv(2,3) << "\n";
		std::cout << matInv(3,0) << " " << matInv(3,1) << " " << matInv(3,2) << " " << matInv(3,3) << "\n";

		std::cout << "----------------------------------------\n";
		std::cout << "\n\n";
	}

	// using ublas classes
	if constexpr(bUseUblas)
	{
		using t_real = double;
		using t_vec = ublas::vector<t_real>;
		using t_mat = ublas::matrix<t_real>;

		vecmat_tsts<t_vec, t_mat>();
		vecmat_tsts_hom<t_vec, t_mat>();
		vecmat_tsts_nonsquare<t_vec, t_mat>();

		std::cout << "----------------------------------------\n";
		std::cout << "\n\n";
	}

	return 0;
}
