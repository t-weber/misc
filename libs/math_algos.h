/**
 * container-agnostic math algorithms
 * @author Tobias Weber
 * @date dec-17
 * @license: see 'LICENSE' file
 */

#ifndef __MATH_ALGOS_H__
#define __MATH_ALGOS_H__

#include "math_concepts.h"

#include <cmath>
#include <tuple>
#include <vector>
#include <limits>

namespace m {

template<typename T> T pi = T(M_PI);


// ----------------------------------------------------------------------------
// n-dim algos
// ----------------------------------------------------------------------------

/**
 * are two scalars equal within an epsilon range?
 */
template<class T>
bool equals(T t1, T t2, T eps = std::numeric_limits<T>::epsilon())
requires is_scalar<T>
{
	return std::abs(t1 - t2) <= eps;
}

/**
 * are two vectors equal within an epsilon range?
 */
template<class t_vec>
bool equals(const t_vec& vec1, const t_vec& vec2,
	typename t_vec::value_type eps = std::numeric_limits<typename t_vec::value_type>::epsilon())
requires is_basic_vec<t_vec>
{
	using T = typename t_vec::value_type;

	if(vec1.size() != vec2.size())
		return false;

	for(std::size_t i=0; i<vec1.size(); ++i)
	{
		if(!equals<T>(vec1[i], vec2[i], eps))
			return false;
	}

	return true;
}

/**
 * are two matrices equal within an epsilon range?
 */
template<class t_mat>
bool equals(const t_mat& mat1, const t_mat& mat2,
	typename t_mat::value_type eps = std::numeric_limits<typename t_mat::value_type>::epsilon())
requires is_mat<t_mat>
{
	using T = typename t_mat::value_type;

	if(mat1.size1() != mat2.size1() || mat1.size2() != mat2.size2())
		return false;

	for(std::size_t i=0; i<mat1.size1(); ++i)
	{
		for(std::size_t j=0; j<mat1.size2(); ++j)
		{
			if(!equals<T>(mat1(i,j), mat2(i,j), eps))
				return false;
		}
	}

	return true;
}


/**
 * set submatrix to unity
 */
template<class t_mat>
void unity(t_mat& mat, std::size_t rows_begin, std::size_t cols_begin, std::size_t rows_end, std::size_t cols_end)
requires is_mat<t_mat>
{
	for(std::size_t i=rows_begin; i<rows_end; ++i)
		for(std::size_t j=cols_begin; j<cols_end; ++j)
			mat(i,j) = (i==j ? 1 : 0);
}


/**
 * unit matrix
 */
template<class t_mat>
t_mat unity(std::size_t N1, std::size_t N2)
requires is_mat<t_mat>
{
	t_mat mat;
	if constexpr(is_dyn_mat<t_mat>)
		mat = t_mat(N1, N2);

	unity<t_mat>(mat, 0,0, mat.size1(),mat.size2());
	return mat;
}


/**
 * unit matrix
 */
template<class t_mat>
t_mat unity(std::size_t N=0)
requires is_mat<t_mat>
{
	return unity<t_mat>(N,N);
}


/**
 * zero matrix
 */
template<class t_mat>
t_mat zero(std::size_t N1, std::size_t N2)
requires is_mat<t_mat>
{
	t_mat mat;
	if constexpr(is_dyn_mat<t_mat>)
		mat = t_mat(N1, N2);

	for(std::size_t i=0; i<mat.size1(); ++i)
		for(std::size_t j=0; j<mat.size2(); ++j)
			mat(i,j) = 0;

	return mat;
}

/**
 * zero matrix
 */
template<class t_mat>
t_mat zero(std::size_t N=0)
requires is_mat<t_mat>
{
	return zero<t_mat>(N, N);
}


/**
 * zero vector
 */
template<class t_vec>
t_vec zero(std::size_t N=0)
requires is_basic_vec<t_vec>
{
	t_vec vec;
	if constexpr(is_dyn_vec<t_vec>)
		vec = t_vec(N);

	for(std::size_t i=0; i<vec.size(); ++i)
		vec[i] = 0;

	return vec;
}


/**
 * tests for zero vector
 */
template<class t_vec>
bool equals_0(const t_vec& vec,
	typename t_vec::value_type eps = std::numeric_limits<typename t_vec::value_type>::epsilon())
requires is_basic_vec<t_vec>
{
	return equals<t_vec>(vec, zero<t_vec>(vec.size()), eps);
}

/**
 * tests for zero matrix
 */
template<class t_mat>
bool equals_0(const t_mat& mat,
	typename t_mat::value_type eps = std::numeric_limits<typename t_mat::value_type>::epsilon())
requires is_mat<t_mat>
{
	return equals<t_mat>(mat, zero<t_mat>(mat.size1(), mat.size2()), eps);
}


/**
 * transpose matrix
 * WARNING: not possible for static non-square matrix!
 */
template<class t_mat>
t_mat trans(const t_mat& mat)
requires is_mat<t_mat>
{
	t_mat mat2;
	if constexpr(is_dyn_mat<t_mat>)
		mat2 = t_mat(mat.size2(), mat.size1());

	for(std::size_t i=0; i<mat.size1(); ++i)
		for(std::size_t j=0; j<mat.size2(); ++j)
			mat2(j,i) = mat(i,j);

	return mat2;
}


/**
 * create vector from initializer_list
 */
template<class t_vec>
t_vec create(const std::initializer_list<typename t_vec::value_type>& lst)
requires is_basic_vec<t_vec>
{
	t_vec vec;
	if constexpr(is_dyn_vec<t_vec>)
		vec = t_vec(lst.size());

	auto iterLst = lst.begin();
	for(std::size_t i=0; i<vec.size(); ++i)
	{
		if(iterLst != lst.end())
		{
			vec[i] = *iterLst;
			std::advance(iterLst, 1);
		}
		else	// vector larger than given list?
		{
			vec[i] = 0;
		}
	}

	return vec;
}


/**
 * create matrix from nested initializer_lists in columns[rows] order
 */
template<class t_mat,
	template<class...> class t_cont_outer = std::initializer_list,
	template<class...> class t_cont = std::initializer_list>
t_mat create(const t_cont_outer<t_cont<typename t_mat::value_type>>& lst)
requires is_mat<t_mat>
{
	const std::size_t iCols = lst.size();
	const std::size_t iRows = lst.begin()->size();

	t_mat mat = unity<t_mat>(iRows, iCols);

	auto iterCol = lst.begin();
	for(std::size_t iCol=0; iCol<iCols; ++iCol)
	{
		auto iterRow = iterCol->begin();
		for(std::size_t iRow=0; iRow<iRows; ++iRow)
		{
			mat(iRow, iCol) = *iterRow;
			std::advance(iterRow, 1);
		}

		std::advance(iterCol, 1);
	}

	return mat;
}


/**
 * create matrix from column (or row) vectors
 */
template<class t_mat, class t_vec, template<class...> class t_cont_outer = std::initializer_list>
t_mat create(const t_cont_outer<t_vec>& lst, bool bRow = 0)
requires is_mat<t_mat>
{
	const std::size_t iCols = lst.size();
	const std::size_t iRows = lst.begin()->size();

	t_mat mat = unity<t_mat>(iRows, iCols);

	auto iterCol = lst.begin();
	for(std::size_t iCol=0; iCol<iCols; ++iCol)
	{
		for(std::size_t iRow=0; iRow<iRows; ++iRow)
			mat(iRow, iCol) = (*iterCol)[iRow];
		std::advance(iterCol, 1);
	}

	if(bRow) mat = trans<t_mat>(mat);
	return mat;
}


/**
 * create matrix from initializer_list in column/row order
 */
template<class t_mat>
t_mat create(const std::initializer_list<typename t_mat::value_type>& lst)
requires is_mat<t_mat>
{
	const std::size_t N = std::sqrt(lst.size());

	t_mat mat = unity<t_mat>(N, N);

	auto iter = lst.begin();
	for(std::size_t iRow=0; iRow<N; ++iRow)
	{
		for(std::size_t iCol=0; iCol<N; ++iCol)
		{
			mat(iRow, iCol) = *iter;
			std::advance(iter, 1);
		}
	}

	return mat;
}


/**
 * get a column vector from a matrix
 */
template<class t_mat, class t_vec>
t_vec col(const t_mat& mat, std::size_t col)
requires is_mat<t_mat> && is_basic_vec<t_vec>
{
	t_vec vec;
	if constexpr(is_dyn_vec<t_vec>)
		vec = t_vec(mat.size1());

	for(std::size_t i=0; i<mat.size1(); ++i)
		vec[i] = mat(i, col);

	return vec;
}

/**
 * get a row vector from a matrix
 */
template<class t_mat, class t_vec>
t_vec row(const t_mat& mat, std::size_t row)
requires is_mat<t_mat> && is_basic_vec<t_vec>
{
	t_vec vec;
	if constexpr(is_dyn_vec<t_vec>)
		vec = t_vec(mat.size2());

	for(std::size_t i=0; i<mat.size2(); ++i)
		vec[i] = mat(row, i);

	return vec;
}


/**
 * inner product
 */
template<class t_vec>
typename t_vec::value_type inner(const t_vec& vec1, const t_vec& vec2)
requires is_basic_vec<t_vec>
{
	typename t_vec::value_type val(0);

	for(std::size_t i=0; i<vec1.size(); ++i)
		val += vec1[i]*vec2[i];

	return val;
}


/**
 * 2-norm
 */
template<class t_vec>
typename t_vec::value_type norm(const t_vec& vec)
requires is_basic_vec<t_vec>
{
	return std::sqrt(inner<t_vec>(vec, vec));
}


/**
 * outer product
 */
template<class t_mat, class t_vec>
t_mat outer(const t_vec& vec1, const t_vec& vec2)
requires is_basic_vec<t_vec> && is_mat<t_mat>
{
	const std::size_t N1 = vec1.size();
	const std::size_t N2 = vec2.size();

	t_mat mat;
	if constexpr(is_dyn_mat<t_mat>)
		mat = t_mat(N1, N2);

	for(std::size_t n1=0; n1<N1; ++n1)
		for(std::size_t n2=0; n2<N2; ++n2)
			mat(n1, n2) = vec1[n1]*vec2[n2];

	return mat;
}


/**
 * matrix to project onto vector: P = |v><v|
 * from: |x'> = <v|x> * |v> = |v><v|x> = |v><v| * |x>
 */
template<class t_mat, class t_vec>
t_mat projector(const t_vec& vec, bool bIsNormalised = true)
requires is_vec<t_vec> && is_mat<t_mat>
{
	if(bIsNormalised)
	{
		return outer<t_mat, t_vec>(vec, vec);
	}
	else
	{
		const auto len = norm<t_vec>(vec);
		t_vec _vec = vec / len;
		return outer<t_mat, t_vec>(_vec, _vec);
	}
}


/**
 * project vector vec onto another vector vecProj
 */
template<class t_vec>
t_vec project(const t_vec& vec, const t_vec& vecProj, bool bIsNormalised = true)
requires is_vec<t_vec>
{
	if(bIsNormalised)
	{
		return inner<t_vec>(vec, vecProj) * vecProj;
	}
	else
	{
		const auto len = norm<t_vec>(vecProj);
		t_vec _vecProj = vecProj / len;
		return inner<t_vec>(vec, _vecProj) * _vecProj;
	}
}


/**
 * project vector vec onto the line lineOrigin + lam*lineDir
 * shift line to go through origin, calculate projection and shift back
 * returns [closest point, distance]
 */
template<class t_vec>
std::tuple<t_vec, typename t_vec::value_type> project_line(const t_vec& vec,
	const t_vec& lineOrigin, const t_vec& lineDir, bool bIsNormalised = true)
requires is_vec<t_vec>
{
	const t_vec ptShifted = vec - lineOrigin;
	const t_vec ptProj = project<t_vec>(ptShifted, lineDir, bIsNormalised);
	const t_vec ptNearest = lineOrigin + ptProj;

	const typename t_vec::value_type dist = norm<t_vec>(vec - ptNearest);
	return std::make_tuple(ptNearest, dist);
}


/**
 * matrix to project onto plane perpendicular to vector: P = 1-|v><v|
 * from completeness relation: 1 = sum_i |v_i><v_i| = |x><x| + |y><y| + |z><z|
 */
template<class t_mat, class t_vec>
t_mat ortho_projector(const t_vec& vec, bool bIsNormalised = true)
requires is_vec<t_vec> && is_mat<t_mat>
{
	const std::size_t iSize = vec.size();
	return unity<t_mat>(iSize) -
		projector<t_mat, t_vec>(vec, bIsNormalised);
}


/**
 * matrix to mirror on plane perpendicular to vector: P = 1 - 2*|v><v|
 * subtracts twice its projection onto the plane normal from the vector
 */
template<class t_mat, class t_vec>
t_mat ortho_mirror_op(const t_vec& vec, bool bIsNormalised = true)
requires is_vec<t_vec> && is_mat<t_mat>
{
	using T = typename t_vec::value_type;
	const std::size_t iSize = vec.size();

	return unity<t_mat>(iSize) -
		T(2)*projector<t_mat, t_vec>(vec, bIsNormalised);
}


/**
 * matrix to mirror [a, b, c, ...] into, e.g.,  [a, b', 0, 0]
 */
template<class t_mat, class t_vec>
t_mat ortho_mirror_zero_op(const t_vec& vec, std::size_t row)
requires is_vec<t_vec> && is_mat<t_mat>
{
	using T = typename t_vec::value_type;
	const std::size_t N = vec.size();

	t_vec vecSub = zero<t_vec>(N);
	for(std::size_t i=0; i<row; ++i)
		vecSub[i] = vec[i];

	// norm of rest vector
	T n = T(0);
	for(std::size_t i=row; i<N; ++i)
		n += vec[i]*vec[i];
	vecSub[row] = std::sqrt(n);

	const t_vec vecOp = vec - vecSub;

	// nothing to do -> return unit matrix
	if(equals_0<t_vec>(vecOp))
		return unity<t_mat>(vecOp.size(), vecOp.size());

	return ortho_mirror_op<t_mat, t_vec>(vecOp, false);
}


/**
 * QR decomposition of a matrix
 * returns [Q, R]
 */
template<class t_mat, class t_vec>
std::tuple<t_mat, t_mat> qr(const t_mat& mat)
requires is_mat<t_mat> && is_vec<t_vec>
{
	using T = typename t_mat::value_type;
	const std::size_t rows = mat.size1();
	const std::size_t cols = mat.size2();
	const std::size_t N = std::min(cols, rows);

	t_mat R = mat;
	t_mat Q = unity<t_mat>(N, N);

	for(std::size_t icol=0; icol<N-1; ++icol)
	{
		t_vec vecCol = col<t_mat, t_vec>(R, icol);
		t_mat matMirror = ortho_mirror_zero_op<t_mat, t_vec>(vecCol, icol);
		Q = Q * matMirror;
		R = matMirror * R;
	}

	return std::make_tuple(Q, R);
}


/**
 * project vector vec onto plane through the origin and perpendicular to vector vecNorm
 */
template<class t_vec>
t_vec ortho_project(const t_vec& vec, const t_vec& vecNorm, bool bIsNormalised = true)
requires is_vec<t_vec>
{
	const std::size_t iSize = vec.size();
	return vec - project<t_vec>(vec, vecNorm, bIsNormalised);
}


/**
 * project vector vec onto plane perpendicular to vector vecNorm with distance d
 * vecNorm has to be normalised and plane in Hessian form: x*vecNorm = d
 */
template<class t_vec>
t_vec ortho_project_plane(const t_vec& vec,
	const t_vec& vecNorm, typename t_vec::value_type d)
requires is_vec<t_vec>
{
	// project onto plane through origin
	t_vec vecProj0 = ortho_project<t_vec>(vec, vecNorm, 1);
	// add distance of plane to origin
	return vecProj0 + d*vecNorm;
}


/**
 * mirror a vector on a plane perpendicular to vector vecNorm with distance d
 * vecNorm has to be normalised and plane in Hessian form: x*vecNorm = d
 */
template<class t_vec>
t_vec ortho_mirror_plane(const t_vec& vec,
	const t_vec& vecNorm, typename t_vec::value_type d)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;

	t_vec vecProj = ortho_project_plane<t_vec>(vec, vecNorm, d);
	return vec - T(2)*(vec - vecProj);
}


/**
 * find orthonormal substitute base for vector space (Gram-Schmidt algo)
 * get orthogonal projections: |i'> = (1 - sum_{j<i} |j><j|) |i>
 */
template<template<class...> class t_cont, class t_vec>
t_cont<t_vec> orthonorm_sys(const t_cont<t_vec>& sys)
requires is_vec<t_vec>
{
	const std::size_t N = sys.size();
	t_cont<t_vec> newsys;

	for(std::size_t i=0; i<N; ++i)
	{
		t_vec vecOrthoProj = sys[i];

		// subtract projections to other base vectors
		for(std::size_t j=0; j<newsys.size(); ++j)
			vecOrthoProj -= project<t_vec>(sys[i], newsys[j], true);

		// normalise
		vecOrthoProj /= norm<t_vec>(vecOrthoProj);
		newsys.push_back(vecOrthoProj);
	}

	return newsys;
}



/**
 * linearise a matrix to a vector container
 */
template<class t_mat, template<class...> class t_cont>
t_cont<typename t_mat::value_type> flatten(const t_mat& mat)
requires is_mat<t_mat> && is_basic_vec<t_cont<typename t_mat::value_type>>
{
	using T = typename t_mat::value_type;
	t_cont<T> vec;

	for(std::size_t iRow=0; iRow<mat.size1(); ++iRow)
		for(std::size_t iCol=0; iCol<mat.size2(); ++iCol)
			vec.push_back(mat(iRow, iCol));

	return vec;
}


/**
 * submatrix removing a column/row from a matrix stored in a vector container
 */
template<class t_vec>
t_vec flat_submat(const t_vec& mat,
	std::size_t iNumRows, std::size_t iNumCols,
	std::size_t iRemRow, std::size_t iRemCol)
requires is_basic_vec<t_vec>
{
	t_vec vec;

	for(std::size_t iRow=0; iRow<iNumRows; ++iRow)
	{
		if(iRow == iRemRow)
			continue;

		for(std::size_t iCol=0; iCol<iNumCols; ++iCol)
		{
			if(iCol == iRemCol)
				continue;
			vec.push_back(mat[iRow*iNumCols + iCol]);
		}
	}

	return vec;
}


/**
 * determinant from a square matrix stored in a vector container
 */
template<class t_vec>
typename t_vec::value_type flat_det(const t_vec& mat, std::size_t iN)
requires is_basic_vec<t_vec>
{
	using T = typename t_vec::value_type;

	// special cases
	if(iN == 0)
		return 0;
	else if(iN == 1)
		return mat[0];
	else if(iN == 2)
		return mat[0]*mat[3] - mat[1]*mat[2];

	// recursively expand determiant along a row
	T fullDet = T(0);
	std::size_t iRow = 0;

	for(std::size_t iCol=0; iCol<iN; ++iCol)
	{
		const T elem = mat[iRow*iN + iCol];
		if(equals<T>(elem, 0))
			continue;

		const T sgn = ((iRow+iCol) % 2) == 0 ? T(1) : T(-1);
		const t_vec subMat = flat_submat<t_vec>(mat, iN, iN, iRow, iCol);
		const T subDet = flat_det<t_vec>(subMat, iN-1) * sgn;

		fullDet += elem * subDet;
	}

	return fullDet;
}


/**
 * determinant
 */
template<class t_mat>
typename t_mat::value_type det(const t_mat& mat)
requires is_mat<t_mat>
{
	using T = typename t_mat::value_type;

	if(mat.size1() != mat.size2())
		return 0;

	std::vector<T> matFlat = flatten<t_mat, std::vector>(mat);
	return flat_det<std::vector<T>>(matFlat, mat.size1());
}


/**
 * inverted matrix
 */
template<class t_mat>
std::tuple<t_mat, bool> inv(const t_mat& mat)
requires is_mat<t_mat>
{
	using T = typename t_mat::value_type;
	using t_vec = std::vector<T>;
	const std::size_t N = mat.size1();

	// fail if matrix is not square
	if(N != mat.size2())
		return std::make_tuple(t_mat(), false);

	const t_vec matFlat = flatten<t_mat, std::vector>(mat);
	const T fullDet = flat_det<t_vec>(matFlat, N);

	// fail if determinant is zero
	if(equals<T>(fullDet, 0))
	{
		//std::cerr << "det == 0" << std::endl;
		return std::make_tuple(t_mat(), false);	
	}

	t_mat matInv;
	if constexpr(is_dyn_mat<t_mat>)
		matInv = t_mat(N, N);

	for(std::size_t i=0; i<N; ++i)
	{
		for(std::size_t j=0; j<N; ++j)
		{
			const T sgn = ((i+j) % 2) == 0 ? T(1) : T(-1);
			const t_vec subMat = flat_submat<t_vec>(matFlat, N, N, i, j);
			matInv(j,i) = sgn * flat_det<t_vec>(subMat, N-1);
		}
	}

	matInv = matInv / fullDet;
	return std::make_tuple(matInv, true);
}



/**
 * intersection of plane <x|n> = d and line |org> + lam*|dir>
 * returns [position of intersection, 0: no intersection, 1: intersection, 2: line on plane, line parameter lambda]
 * insert |x> = |org> + lam*|dir> in plane equation:
 * <org|n> + lam*<dir|n> = d
 * lam = (d - <org|n>) / <dir|n>
 */
template<class t_vec>
std::tuple<t_vec, int, typename t_vec::value_type> intersect_line_plane(
	const t_vec& lineOrg, const t_vec& lineDir,
	const t_vec& planeNorm, typename t_vec::value_type plane_d)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;

	// are line and plane parallel?
	const T dir_n = inner<t_vec>(lineDir, planeNorm);
	if(equals<T>(dir_n, 0))
	{
		const T org_n = inner<t_vec>(lineOrg, planeNorm);
		// line on plane?
		if(equals<T>(org_n, plane_d))		
			return std::make_tuple(t_vec(), 2, T(0));
		// no intersection
		return std::make_tuple(t_vec(), 0, T(0));
	}

	const T org_n = inner<t_vec>(lineOrg, planeNorm);
	const T lam = (plane_d - org_n) / dir_n;

	const t_vec vecInters = lineOrg + lam*lineDir;
	return std::make_tuple(vecInters, 1, lam);
}


/**
 * intersection or closest points of lines |org1> + lam1*|dir1> and |org2> + lam2*|dir2>
 * returns [nearest position 1, nearest position 2, dist, valid?, line parameter 1, line parameter 2]
 * 
 * |org1> + lam1*|dir1>  =  |org2> + lam2*|dir2>
 * |org1> - |org2>  =  lam2*|dir2> - lam1*|dir1>
 * |org1> - |org2>  =  (dir2 | -dir1) * |lam2 lam1>
 * (dir2 | -dir1)^T * (|org1> - |org2>)  =  (dir2 | -dir1)^T * (dir2 | -dir1) * |lam2 lam1>
 * |lam2 lam1> = ((dir2 | -dir1)^T * (dir2 | -dir1))^(-1) * (dir2 | -dir1)^T * (|org1> - |org2>)
 */
template<class t_vec>
std::tuple<t_vec, t_vec, bool, typename t_vec::value_type, typename t_vec::value_type, typename t_vec::value_type> 
	intersect_line_line(
	const t_vec& line1Org, const t_vec& line1Dir,
	const t_vec& line2Org, const t_vec& line2Dir)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;
	const std::size_t N = line1Dir.size();

	const t_vec orgdiff = line1Org - line2Org;

	// direction matrix (symmetric)
	const T d11 = inner<t_vec>(line2Dir, line2Dir);
	const T d12 = -inner<t_vec>(line2Dir, line1Dir);
	const T d22 = inner<t_vec>(line1Dir, line1Dir);

	const T d_det = d11*d22 - d12*d12;

	// check if matrix is invertible
	if(equals<T>(d_det, 0))
		return std::make_tuple(t_vec(), t_vec(), false, 0, 0, 0);

	// inverse (symmetric)
	const T d11_i = d22 / d_det;
	const T d12_i = -d12 / d_det;
	const T d22_i = d11 / d_det;

	const t_vec v1 = d11_i*line2Dir - d12_i*line1Dir;
	const t_vec v2 = d12_i*line2Dir - d22_i*line1Dir;

	const T lam2 = inner<t_vec>(v1, orgdiff);
	const T lam1 = inner<t_vec>(v2, orgdiff);

	const t_vec pos1 = line1Org + lam1*line1Dir;
	const t_vec pos2 = line2Org + lam2*line2Dir;
	const T dist = norm<t_vec>(pos2-pos1);

	return std::make_tuple(pos1, pos2, true, dist, lam1, lam2);
}


/**
 * general n-dim cross product using determinant definition
 */
template<class t_vec, template<class...> class t_cont = std::initializer_list>
t_vec cross(const t_cont<t_vec>& vecs)
requires is_basic_vec<t_vec>
{
	using T = typename t_vec::value_type;
	// N also has to be equal to the vector size!
	const std::size_t N = vecs.size()+1;
	t_vec vec = zero<t_vec>(N);

	for(std::size_t iComp=0; iComp<N; ++iComp)
	{
		std::vector<T> mat = zero<std::vector<T>>(N*N);
		mat[0*N + iComp] = T(1);

		std::size_t iRow = 0;
		for(const t_vec& vec : vecs)
		{
			for(std::size_t iCol=0; iCol<N; ++iCol)
				mat[(iRow+1)*N + iCol] = vec[iCol];
			++iRow;
		}

		vec[iComp] = flat_det<decltype(mat)>(mat, N);
	}

	return vec;
}


/**
 * intersection of planes <x|n1> = d1 and <x|n2> = d2
 * returns line [org, dir, 0: no intersection, 1: intersection, 2: planes coincide]
 */
template<class t_vec>
std::tuple<t_vec, t_vec, int>
	intersect_plane_plane(
	const t_vec& plane1Norm, typename t_vec::value_type plane1_d,
	const t_vec& plane2Norm, typename t_vec::value_type plane2_d)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;
	const std::size_t dim = plane1Norm.size();

	/*
	// alternate, direct calculation (TODO): 
	// (n1)               ( d1 )
	// (n2) * (x,y,z)^T = ( d2 )
	//
	// N x = d  ->  R x = Q^T d  ->  back-substitute
	
	const t_mat N = create<t_mat, t_vec>({plane1Norm, plane2Norm}, true);
	const auto [Q, R] = qr<t_mat, t_vec>(N);

	const T d[] =
	{
		Q(0,0)*plane1_d + Q(1,0)*plane2_d,
		Q(0,1)*plane1_d + Q(1,1)*plane2_d
	};*/

	t_vec lineDir = cross<t_vec>({plane1Norm, plane2Norm});
	const T lenCross = norm<t_vec>(lineDir);

	// planes parallel or coinciding
	if(equals<T>(lenCross, 0))
	{
		const bool bCoincide = equals<T>(plane1_d, plane2_d);
		return std::make_tuple(t_vec(), t_vec(), bCoincide ? 2 : 0);
	}

	lineDir /= lenCross;

	t_vec lineOrg = - cross<t_vec>({plane1Norm, lineDir}) * plane2_d
		+ cross<t_vec>({plane2Norm, lineDir}) * plane1_d;
	lineOrg /= lenCross;

	return std::make_tuple(lineOrg, lineDir, 1);
}

// ----------------------------------------------------------------------------





// ----------------------------------------------------------------------------
// 3-dim algos
// ----------------------------------------------------------------------------

/**
 * 3-dim cross product
 */
template<class t_vec>
t_vec cross(const t_vec& vec1, const t_vec& vec2)
requires is_basic_vec<t_vec>
{
	t_vec vec;

	// only valid for 3-vectors -> use first three components
	if(vec1.size() < 3 || vec2.size() < 3)
		return vec;

	if constexpr(is_dyn_vec<t_vec>)
		vec = t_vec(3);

	for(int i=0; i<3; ++i)
		vec[i] = vec1[(i+1)%3]*vec2[(i+2)%3] - vec1[(i+2)%3]*vec2[(i+1)%3];

	return vec;
}


/**
 * cross product matrix (3x3)
 */
template<class t_mat, class t_vec>
t_mat skewsymmetric(const t_vec& vec)
requires is_basic_vec<t_vec> && is_mat<t_mat>
{
	t_mat mat;
	if constexpr(is_dyn_mat<t_mat>)
		mat = t_mat(3,3);

	// if static matrix is larger than 3x3 (e.g. for homogeneous coordinates), initialise as identity
	if(mat.size1() > 3 || mat.size2() > 3)
		mat = unity<t_mat>(mat.size1(), mat.size2());

	mat(0,0) = 0; 		mat(0,1) = -vec[2]; 	mat(0,2) = vec[1];
	mat(1,0) = vec[2]; 	mat(1,1) = 0; 			mat(1,2) = -vec[0];
	mat(2,0) = -vec[1]; mat(2,1) = vec[0]; 		mat(2,2) = 0;

	return mat;
}


/**
 * matrix to rotate around an axis
 */
template<class t_mat, class t_vec>
t_mat rotation(const t_vec& axis, const typename t_vec::value_type angle, bool bIsNormalised=1)
requires is_vec<t_vec> && is_mat<t_mat>
{
	using t_real = typename t_vec::value_type;

	const t_real c = std::cos(angle);
	const t_real s = std::sin(angle);

	t_real len = 1;
	if(!bIsNormalised)
		len = norm<t_vec>(axis);

	// ----------------------------------------------------
	// special cases: rotations around [100], [010], [001]
	if(equals(axis, create<t_vec>({len,0,0})))
		return create<t_mat>({{1,0,0}, {0,c,s}, {0,-s,c}});
	else if(equals(axis, create<t_vec>({0,len,0})))
		return create<t_mat>({{c,0,-s}, {0,1,0}, {s,0,c}});
	else if(equals(axis, create<t_vec>({0,0,len})))
		return create<t_mat>({{c,s,0}, {-s,c,0}, {0,0,1}});

	// ----------------------------------------------------
	// general case
	// project along rotation axis
	t_mat matProj1 = projector<t_mat, t_vec>(axis, bIsNormalised);

	// project along axis 2 in plane perpendiculat to rotation axis
	t_mat matProj2 = ortho_projector<t_mat, t_vec>(axis, bIsNormalised) * c;

	// project along axis 3 in plane perpendiculat to rotation axis and axis 2
	t_mat matProj3 = skewsymmetric<t_mat, t_vec>(axis/len) * s;

	//std::cout << matProj1(3,3) <<  " " << matProj2(3,3) <<  " " << matProj3(3,3) << std::endl;
	t_mat matProj = matProj1 + matProj2 + matProj3;

	// if matrix is larger than 3x3 (e.g. for homogeneous cooridnates), fill up with identity
	unity<t_mat>(matProj, 3,3, matProj.size1(), matProj.size2());
	return matProj;
}


/**
 * create the faces of a cube
 * returns [vertices, face vertex indices, face normals, face uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_cont<std::size_t>>, t_cont<t_vec>, t_cont<t_cont<t_vec>>>
create_cube(typename t_vec::value_type l = 1)
requires is_vec<t_vec>
{
	t_cont<t_vec> vertices =
	{
		create<t_vec>({ +l, -l, -l }),	// vertex 0
		create<t_vec>({ -l, -l, -l }),	// vertex 1
		create<t_vec>({ -l, +l, -l }),	// vertex 2
		create<t_vec>({ +l, +l, -l }),	// vertex 3

		create<t_vec>({ -l, -l, +l }),	// vertex 4
		create<t_vec>({ +l, -l, +l }),	// vertex 5
		create<t_vec>({ +l, +l, +l }),	// vertex 6
		create<t_vec>({ -l, +l, +l }),	// vertex 7
	};

	t_cont<t_cont<std::size_t>> faces =
	{
		{ 0, 1, 2, 3 },	// -z face
		{ 4, 5, 6, 7 },	// +z face
		{ 1, 0, 5, 4 }, // -y face
		{ 7, 6, 3, 2 },	// +y face
		{ 1, 4, 7, 2 },	// -x face
		{ 5, 0, 3, 6 },	// +x face
	};

	t_cont<t_vec> normals =
	{
		create<t_vec>({ 0, 0, -1 }),	// -z face
		create<t_vec>({ 0, 0, +1 }),	// +z face
		create<t_vec>({ 0, -1, 0 }),	// -y face
		create<t_vec>({ 0, +1, 0 }),	// +y face
		create<t_vec>({ -1, 0, 0 }),	// -x face
		create<t_vec>({ +1, 0, 0 }),	// +x face
	};

	t_cont<t_cont<t_vec>> uvs =
	{
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({1,1}), create<t_vec>({0,1}) },	// -z face
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({1,1}), create<t_vec>({0,1}) },	// +z face
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({1,1}), create<t_vec>({0,1}) },	// -y face
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({1,1}), create<t_vec>({0,1}) },	// +y face
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({1,1}), create<t_vec>({0,1}) },	// -x face
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({1,1}), create<t_vec>({0,1}) },	// +x face
	};

	return std::make_tuple(vertices, faces, normals, uvs);
}


/**
 * triangulates polygon object, takes input from e.g. create_cube()
 * returns [triangles, face normals, vertex uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_vec>, t_cont<t_vec>>
create_triangles(const std::tuple<t_cont<t_vec>, t_cont<t_cont<std::size_t>>, t_cont<t_vec>, t_cont<t_cont<t_vec>>>& tup)
requires is_vec<t_vec>
{
	const t_cont<t_vec>& vertices = std::get<0>(tup);
	const t_cont<t_cont<std::size_t>>& faces = std::get<1>(tup);
	const t_cont<t_vec>& normals = std::get<2>(tup);
	const t_cont<t_cont<t_vec>>& uvs = std::get<3>(tup);

	t_cont<t_vec> triangles;
	t_cont<t_vec> vert_normals;
	t_cont<t_vec> vert_uvs;

	auto iterFaces = faces.begin();
	auto iterNorms = normals.begin();
	auto iterUVs = uvs.begin();

	// iterate over faces
	while(iterFaces != faces.end())
	{
		// triangulate faces
		auto iterFaceVertIdx = iterFaces->begin();
		std::size_t vert1Idx = *iterFaceVertIdx;
		std::advance(iterFaceVertIdx, 1);
		std::size_t vert2Idx = *iterFaceVertIdx;

		auto iterFaceUVIdx = iterUVs->begin();
		const t_vec *puv1 = nullptr;
		const t_vec *puv2 = nullptr;
		const t_vec *puv3 = nullptr;
		if(iterFaceUVIdx != iterUVs->end())
		{
			puv1 = &(*iterFaceUVIdx);
			std::advance(iterFaceUVIdx, 1);
			puv2 = &(*iterFaceUVIdx);
		}


		while(1)
		{
			std::advance(iterFaceVertIdx, 1);
			if(iterFaceVertIdx == iterFaces->end())
				break;
			std::size_t vert3Idx = *iterFaceVertIdx;

			if(iterFaceUVIdx != iterUVs->end())
			{
				std::advance(iterFaceUVIdx, 1);
				puv3 = &(*iterFaceUVIdx);
			}

			// create triangle
			triangles.push_back(vertices[vert1Idx]);
			triangles.push_back(vertices[vert2Idx]);
			triangles.push_back(vertices[vert3Idx]);

			// triangle normal
			vert_normals.push_back(*iterNorms);
			//vert_normals.push_back(*iterNorms);
			//vert_normals.push_back(*iterNorms);

			// triangle vertex uvs
			if(puv1 && puv2 && puv3)
			{
				vert_uvs.push_back(*puv1);
				vert_uvs.push_back(*puv2);
				vert_uvs.push_back(*puv3);
			}


			// next vertex
			vert2Idx = vert3Idx;
			puv2 = puv3;
		}


		std::advance(iterFaces, 1);
		if(iterNorms != normals.end()) std::advance(iterNorms, 1);
		if(iterUVs != uvs.end()) std::advance(iterUVs, 1);
	}

	return std::make_tuple(triangles, vert_normals, vert_uvs);
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// 3-dim algos in homogeneous coordinates
// ----------------------------------------------------------------------------

/**
 * project a homogeneous vector to screen coordinates 
 * returns [vecPersp, vecScreen]
 */
template<class t_mat, class t_vec>
std::tuple<t_vec, t_vec> hom_to_screen_coords(const t_vec& vec4,
	const t_mat& matModelView, const t_mat& matProj, const t_mat& matViewport,
	bool bFlipY = false, bool bFlipX = false)
requires is_vec<t_vec> && is_mat<t_mat>
{
	// perspective trafo and divide
	t_vec vecPersp = matProj * matModelView * vec4;
	vecPersp /= vecPersp[3];

	// viewport trafo
	t_vec vec = matViewport * vecPersp;

	// flip y coordinate
	if(bFlipY) vec[1] = matViewport(1,1)*2 - vec[1];
	// flip x coordinate
	if(bFlipX) vec[0] = matViewport(0,0)*2 - vec[0];

	return std::make_tuple(vecPersp, vec);
}


/**
 * calculate world coordinates from screen coordinates
 * (vary zPlane to get the points of the z-line at constant (x,y))
 */
template<class t_mat, class t_vec>
t_vec hom_from_screen_coords(
	typename t_vec::value_type xScreen, typename t_vec::value_type yScreen, typename t_vec::value_type zPlane,
	const t_mat& matModelView_inv, const t_mat& matProj_inv, const t_mat& matViewport_inv,
	const t_mat* pmatViewport = nullptr, bool bFlipY = false, bool bFlipX = false)
requires is_vec<t_vec> && is_mat<t_mat>
{
	t_vec vecScreen = create<t_vec>({xScreen, yScreen, zPlane, 1.});

	// flip y coordinate
	if(pmatViewport && bFlipY) vecScreen[1] = (*pmatViewport)(1,1)*2 - vecScreen[1];
	// flip x coordinate
	if(pmatViewport && bFlipX) vecScreen[0] = (*pmatViewport)(0,0)*2 - vecScreen[0];

	t_vec vecWorld = matModelView_inv * matProj_inv * matViewport_inv * vecScreen;

	vecWorld /= vecWorld[3];
	return vecWorld;
}


/**
 * calculate line from screen coordinates
 * returns [pos, dir]
 */
template<class t_mat, class t_vec>
std::tuple<t_vec, t_vec> hom_line_from_screen_coords(
	typename t_vec::value_type xScreen, typename t_vec::value_type yScreen,
	typename t_vec::value_type z1, typename t_vec::value_type z2,
	const t_mat& matModelView_inv, const t_mat& matProj_inv, const t_mat& matViewport_inv,
	const t_mat* pmatViewport = nullptr, bool bFlipY = false, bool bFlipX = false)
requires is_vec<t_vec> && is_mat<t_mat>
{
	const t_vec lineOrg = hom_from_screen_coords<t_mat, t_vec>(xScreen, yScreen, z1, matModelView_inv, matProj_inv,
		matViewport_inv, pmatViewport, bFlipY, bFlipX);
	const t_vec linePos2 = hom_from_screen_coords<t_mat, t_vec>(xScreen, yScreen, z2, matModelView_inv, matProj_inv,
		matViewport_inv, pmatViewport, bFlipY, bFlipX);

	t_vec lineDir = linePos2 - lineOrg;
	lineDir /= norm<t_vec>(lineDir);

	return std::make_tuple(lineOrg, lineDir);
}


/**
 * perspective matrix (homogeneous 4x4)
 * see: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
 */
template<class t_mat>
t_mat hom_perspective(
	typename t_mat::value_type n = 0.01, typename t_mat::value_type f = 100.,
	typename t_mat::value_type fov = 0.5*pi<typename t_mat::value_type>, typename t_mat::value_type ratio = 3./4.,
	bool bRHS = true, bool bZ01 = false)
requires is_mat<t_mat>
{
	using T = typename t_mat::value_type;
	const T c = 1./std::tan(0.5 * fov);
	const T n0 = bZ01 ? T(0) : n;
	const T sc = bZ01 ? T(1) : T(2);
	const T zs = bRHS ? T(1) : T(-1);

	//         ( x*c*r                           )      ( -x*c*r/z                         )
	//         ( y*c                             )      ( -y*c/z                           )
	// P * x = ( z*(n0+f)/(n-f) + w*sc*n*f/(n-f) )  =>  ( -(n0+f)/(n-f) - w/z*sc*n*f/(n-f) )
	//         ( -z                              )      ( 1                                )
	return create<t_mat>({
		c*ratio, 	0., 	0., 				0.,
		0, 			c, 		0., 				0.,
		0.,			0.,		zs*(n0+f)/(n-f), 	sc*n*f/(n-f),
		0.,			0.,		-zs,				0. 
	});
}


/**
 * viewport matrix (homogeneous 4x4)
 * see: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glViewport.xml
 */
template<class t_mat>
t_mat hom_viewport(typename t_mat::value_type w, typename t_mat::value_type h,
	typename t_mat::value_type n = 0, typename t_mat::value_type f = 1)
requires is_mat<t_mat>
{
	using T = typename t_mat::value_type;

	return create<t_mat>({
		T(0.5)*w, 	0., 		0., 			T(0.5)*w,
		0, 			T(0.5)*h, 	0., 			T(0.5)*h,
		0.,			0.,			T(0.5)*(f-n), 	T(0.5)*(f+n),
		0.,			0.,			0.,				1. 
	});
}

// ----------------------------------------------------------------------------

}
#endif
