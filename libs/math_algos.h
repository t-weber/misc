/**
 * container-agnostic math algorithms
 * @author Tobias Weber
 * @date dec-17
 * @license see 'LICENSE.EUPL' file
 */

#ifndef __MATH_ALGOS_H__
#define __MATH_ALGOS_H__

#include "math_concepts.h"
//#include "math_conts.h"

#include <cmath>
#include <complex>
#include <tuple>
#include <vector>
#include <limits>
#include <algorithm>
#include <numeric>
//#include <iostream>


namespace m {

template<typename T> constexpr T pi = T(M_PI);
template<typename T> T golden = T(0.5) + std::sqrt(T(5))/T(2);


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
 * are two complex numbers equal within an epsilon range?
 */
template<class T>
bool equals(const T& t1, const T& t2,
	typename T::value_type eps = std::numeric_limits<typename T::value_type>::epsilon())
requires is_complex<T>
{
	return (std::abs(t1.real() - t2.real()) <= eps) &&
		(std::abs(t1.imag() - t2.imag()) <= eps);
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

	// size has to be equal
	if(vec1.size() != vec2.size())
		return false;

	// check each element
	for(std::size_t i=0; i<vec1.size(); ++i)
	{
		if constexpr(is_complex<decltype(eps)>)
		{
			if(!equals<T>(vec1[i], vec2[i], eps.real()))
				return false;
		}
		else
		{
			if(!equals<T>(vec1[i], vec2[i], eps))
				return false;
		}
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
 * set submatrix to unit
 */
template<class t_mat>
void unit(t_mat& mat, std::size_t rows_begin, std::size_t cols_begin, std::size_t rows_end, std::size_t cols_end)
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
t_mat unit(std::size_t N1, std::size_t N2)
requires is_mat<t_mat>
{
	t_mat mat;
	if constexpr(is_dyn_mat<t_mat>)
		mat = t_mat(N1, N2);

	unit<t_mat>(mat, 0,0, mat.size1(),mat.size2());
	return mat;
}


/**
 * unit matrix
 */
template<class t_mat>
t_mat unit(std::size_t N=0)
requires is_mat<t_mat>
{
	return unit<t_mat>(N,N);
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
 * create matrix from nested initializer_lists in columns/rows order
 */
template<class t_mat,
	template<class...> class t_cont_outer = std::initializer_list,
	template<class...> class t_cont = std::initializer_list>
t_mat create(const t_cont_outer<t_cont<typename t_mat::value_type>>& lst)
requires is_mat<t_mat>
{
	const std::size_t iCols = lst.size();
	const std::size_t iRows = lst.begin()->size();

	t_mat mat = unit<t_mat>(iRows, iCols);

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
t_mat create(const t_cont_outer<t_vec>& lst, bool bRow = false)
requires is_mat<t_mat> && is_basic_vec<t_vec>
{
	const std::size_t iCols = lst.size();
	const std::size_t iRows = lst.begin()->size();

	t_mat mat = unit<t_mat>(iRows, iCols);

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

	t_mat mat = unit<t_mat>(N, N);

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
 * inner product <vec1|vec2>
 */
template<class t_vec>
typename t_vec::value_type inner(const t_vec& vec1, const t_vec& vec2)
requires is_basic_vec<t_vec>
{
	typename t_vec::value_type val(0);

	for(std::size_t i=0; i<vec1.size(); ++i)
	{
		if constexpr(is_complex<typename t_vec::value_type>)
			val += std::conj(vec1[i]) * vec2[i];
		else
			val += vec1[i] * vec2[i];
	}

	return val;
}


/**
 * inner product between two vectors of different type
 */
template<class t_vec1, class t_vec2>
typename t_vec1::value_type inner(const t_vec1& vec1, const t_vec2& vec2)
requires is_basic_vec<t_vec1> && is_basic_vec<t_vec2>
{
	if(vec1.size()==0 || vec2.size()==0)
		return typename t_vec1::value_type{};

	// first element
	auto val = vec1[0]*vec2[0];

	// remaining elements
	for(std::size_t i=1; i<std::min(vec1.size(), vec2.size()); ++i)
	{
		if constexpr(is_complex<typename t_vec1::value_type>)
		{
			auto prod = std::conj(vec1[i]) * vec2[i];
			val = val + prod;
		}
		else
		{
			auto prod = vec1[i]*vec2[i];
			val = val + prod;
		}
	}

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
		{
			if constexpr(is_complex<typename t_vec::value_type>)
				mat(n1, n2) = std::conj(vec1[n1]) * vec2[n2];
			else
				mat(n1, n2) = vec1[n1]*vec2[n2];
		}

	return mat;
}



// ----------------------------------------------------------------------------
// with metric

/**
 * covariant metric tensor
 * g_{i,j} = e_i * e_j
 */
template<class t_mat, class t_vec, template<class...> class t_cont=std::initializer_list>
t_mat metric(const t_cont<t_vec>& basis_co)
requires is_basic_mat<t_mat> && is_basic_vec<t_vec>
{
	const std::size_t N = basis_co.size();

	t_mat g_co;
	if constexpr(is_dyn_mat<t_mat>)
		g_co = t_mat(N, N);

	auto iter_i = basis_co.begin();
	for(std::size_t i=0; i<N; ++i)
	{
		auto iter_j = basis_co.begin();
		for(std::size_t j=0; j<N; ++j)
		{
			g_co(i,j) = inner<t_vec>(*iter_i, *iter_j);
			std::advance(iter_j, 1);
		}
		std::advance(iter_i, 1);
	}

	return g_co;
}


/**
 * lower index using metric
 */
template<class t_mat, class t_vec>
t_vec lower_index(const t_mat& metric_co, const t_vec& vec_contra)
requires is_basic_mat<t_mat> && is_basic_vec<t_vec>
{
	const std::size_t N = vec_contra.size();
	t_vec vec_co = zero<t_vec>(N);

	for(std::size_t i=0; i<N; ++i)
		for(std::size_t j=0; j<N; ++j)
			vec_co[i] += metric_co(i,j) * vec_contra[j];

	return vec_co;
}


/**
 * raise index using metric
 */
template<class t_mat, class t_vec>
t_vec raise_index(const t_mat& metric_contra, const t_vec& vec_co)
requires is_basic_mat<t_mat> && is_basic_vec<t_vec>
{
	const std::size_t N = vec_co.size();
	t_vec vec_contra = zero<t_vec>(N);

	for(std::size_t i=0; i<N; ++i)
		for(std::size_t j=0; j<N; ++j)
			vec_contra[i] += metric_contra(i,j) * vec_co[j];

	return vec_contra;
}


/**
 * inner product using metric
 */
template<class t_mat, class t_vec>
typename t_vec::value_type inner(const t_mat& metric_co, const t_vec& vec1_contra, const t_vec& vec2_contra)
requires is_basic_mat<t_mat> && is_basic_vec<t_vec>
{
	t_vec vec2_co = lower_index<t_mat, t_vec>(metric_co, vec2_contra);
	return inner<t_vec>(vec1_contra, vec2_co);
}


/**
 * 2-norm using metric
 */
template<class t_mat, class t_vec>
typename t_vec::value_type norm(const t_mat& metric_co, const t_vec& vec_contra)
requires is_basic_vec<t_vec>
{
	return std::sqrt(inner<t_mat, t_vec>(metric_co, vec_contra, vec_contra));
}
// ----------------------------------------------------------------------------



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
		const t_vec _vecProj = vecProj / len;
		return inner<t_vec>(vec, _vecProj) * _vecProj;
	}
}


/**
 * project vector vec onto another vector vecProj
 * don't multiply with direction vector
 */
template<class t_vec>
typename t_vec::value_type
project_scalar(const t_vec& vec, const t_vec& vecProj, bool bIsNormalised = true)
requires is_vec<t_vec>
{
	if(bIsNormalised)
	{
		return inner<t_vec>(vec, vecProj);
	}
	else
	{
		const auto len = norm<t_vec>(vecProj);
		const t_vec _vecProj = vecProj / len;
		return inner<t_vec>(vec, _vecProj);
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
 * matrix to project onto orthogonal complement (plane perpendicular to vector): P = 1-|v><v|
 * from completeness relation: 1 = sum_i |v_i><v_i| = |x><x| + |y><y| + |z><z|
 */
template<class t_mat, class t_vec>
t_mat ortho_projector(const t_vec& vec, bool bIsNormalised = true)
requires is_vec<t_vec> && is_mat<t_mat>
{
	const std::size_t iSize = vec.size();
	return unit<t_mat>(iSize) -
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

	return unit<t_mat>(iSize) -
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
		return unit<t_mat>(vecOp.size(), vecOp.size());

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
	t_mat Q = unit<t_mat>(N, N);

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
 * (e.g. used to calculate magnetic interaction vector M_perp)
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
 * find orthonormal substitute basis for vector space (Gram-Schmidt algo)
 * remove orthogonal projections to all other base vectors: |i'> = (1 - sum_{j<i} |j><j|) |i>
 */
template<class t_vec,
	template<class...> class t_cont_in = std::initializer_list,
	template<class...> class t_cont_out = std::vector>
t_cont_out<t_vec> orthonorm_sys(const t_cont_in<t_vec>& sys)
requires is_vec<t_vec>
{
	t_cont_out<t_vec> newsys;

	const std::size_t N = sys.size();
	for(const t_vec& vecSys : sys)
	{
		t_vec vecOrthoProj = vecSys;

		// subtract projections to other basis vectors
		for(const t_vec& vecNewSys : newsys)
			vecOrthoProj -= project<t_vec>(vecSys, vecNewSys, true);

		// normalise
		vecOrthoProj /= norm<t_vec>(vecOrthoProj);
		newsys.emplace_back(std::move(vecOrthoProj));
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
 * trace
 */
template<class t_mat>
typename t_mat::value_type trace(const t_mat& mat)
requires is_mat<t_mat>
{
	using T = typename t_mat::value_type;
	T _tr = T(0);

	std::size_t N = std::min(mat.size1(), mat.size2());
	for(std::size_t i=0; i<N; ++i)
		_tr += mat(i,i);

	return _tr;
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
 * gets reciprocal basis vectors |b_i> from real basis vectors |a_i> (and vice versa)
 * c: multiplicative constant (c=2*pi for physical lattices, c=1 for mathematics)
 *
 * Def.: <b_i | a_j> = c * delta(i,j)  =>
 *
 * e.g. 2d case:
 *                   ( a_1x  a_2x )
 *                   ( a_1y  a_2y )
 *
 * ( b_1x  b_1y )    (    1     0 )
 * ( b_2x  b_2y )    (    0     1 )
 *
 * B^t * A = I
 * A = B^(-t)
 */
template<class t_mat, class t_vec,
	template<class...> class t_cont_in = std::initializer_list,
	template<class...> class t_cont_out = std::vector>
t_cont_out<t_vec> recip(const t_cont_in<t_vec>& lstReal, typename t_vec::value_type c=1)
requires is_mat<t_mat> && is_basic_vec<t_vec>
{
	const t_mat basis = create<t_mat, t_vec, t_cont_in>(lstReal);
	auto [basis_inv, bOk] = inv<t_mat>(basis);
	basis_inv *= c;

	t_cont_out<t_vec> lstRecip;
	for(std::size_t currow=0; currow<basis_inv.size1(); ++currow)
	{
		const t_vec rowvec = row<t_mat, t_vec>(basis_inv, currow);
		lstRecip.emplace_back(std::move(rowvec));
	}

	return lstRecip;
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
 * intersection of plane <x|n> = d and line |org> + lam*|dir>
 * returns [position of intersection, 0: no intersection, 1: intersection, 2: line on plane, line parameter lambda]
 * insert |x> = |org> + lam*|dir> in plane equation:
 * <org|n> + lam*<dir|n> = d
 * lam = (d - <org|n>) / <dir|n>
 */
template<class t_vec>
std::tuple<t_vec, int, typename t_vec::value_type>
intersect_line_plane(
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
 * intersection of a sphere  and a line |org> + lam*|dir>
 * returns vector of intersections
 * insert |x> = |org> + lam*|dir> in sphere equation <x-mid | x-mid> = r^2
 * For solution, see: https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
 */
template<class t_vec, template<class...> class t_cont = std::vector>
t_cont<t_vec>
intersect_line_sphere(
	const t_vec& lineOrg, const t_vec& lineDir,
	const t_vec& sphereOrg, typename t_vec::value_type sphereRad,
	bool bLineDirIsNormalised = false)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;

	auto vecDiff = sphereOrg-lineOrg;
	auto proj = project_scalar<t_vec>(vecDiff, lineDir, bLineDirIsNormalised);
	auto rt = proj*proj + sphereRad*sphereRad - inner<t_vec>(vecDiff, vecDiff);

	// no intersection
	if(rt < T(0)) return t_cont<t_vec>{};

	// one intersection
	if(equals(rt, T(0))) return t_cont<t_vec>{{ lineOrg + proj*lineDir }};

	// two intersections
	auto val = std::sqrt(rt);
	auto lam1 = proj + val;
	auto lam2 = proj - val;
	return t_cont<t_vec>{{ lineOrg + lam1*lineDir, lineOrg + lam2*lineDir }};
}


/**
 * average vector or matrix
 */
template<class ty, template<class...> class t_cont = std::vector>
ty avg(const t_cont<ty>& vecs)
requires is_vec<ty> || is_mat<ty>
{
	if(vecs.size() == 0)
		return ty();

	typename ty::value_type num = 1;
	ty vec = *vecs.begin();

	auto iter = vecs.begin();
	std::advance(iter, 1);

	for(; iter!=vecs.end(); std::advance(iter, 1))
	{
		vec += *iter;
		++num;
	}
	vec /= num;

	return vec;
}


/**
 * intersection of a polygon and a line
 * returns [position of intersection, intersects?, line parameter lambda]
 */
template<class t_vec, template<class ...> class t_cont = std::vector>
std::tuple<t_vec, bool, typename t_vec::value_type>
intersect_line_poly(
	const t_vec& lineOrg, const t_vec& lineDir,
	const t_cont<t_vec>& poly)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;

	// middle point
	const t_vec mid = avg<t_vec, t_cont>(poly);

	// calculate polygon plane
	const t_vec vec0 = poly[0] - mid;
	const t_vec vec1 = poly[1] - mid;
	t_vec planeNorm = cross<t_vec>({vec0, vec1});
	planeNorm /= norm<t_vec>(planeNorm);
	const T planeD = inner<t_vec>(poly[0], planeNorm);

	// intersection with plane
	auto [vec, intersects, lam] = intersect_line_plane<t_vec>(lineOrg, lineDir, planeNorm, planeD);
	if(intersects != 1)
		return std::make_tuple(t_vec(), false, T(0));

	// is intersection point contained in polygon?
	const t_vec* vecFirst = &(*poly.rbegin());
	for(auto iter=poly.begin(); iter!=poly.end(); std::advance(iter, 1))
	{
		const t_vec* vecSecond = &(*iter);
		const t_vec edge = *vecSecond - *vecFirst;

		// plane through edge
		t_vec edgeNorm = cross<t_vec>({edge, planeNorm});
		edgeNorm /= norm<t_vec>(edgeNorm);
		const T edgePlaneD = inner<t_vec>(*vecFirst, edgeNorm);

		// side of intersection
		const T ptEdgeD = inner<t_vec>(vec, edgeNorm);

		// outside polygon?
		if(ptEdgeD > edgePlaneD)
			return std::make_tuple(t_vec(), false, T(0));

		vecFirst = vecSecond;
	}

	// intersects with polygon
	return std::make_tuple(vec, true, lam);
}


/**
 * intersection of a polygon (transformed with a matrix) and a line
 * returns [position of intersection, intersects?, line parameter lambda]
 */
template<class t_vec, class t_mat, template<class ...> class t_cont = std::vector>
std::tuple<t_vec, bool, typename t_vec::value_type>
intersect_line_poly(
	const t_vec& lineOrg, const t_vec& lineDir,
	const t_cont<t_vec>& _poly, const t_mat& mat)
requires is_vec<t_vec> && is_mat<t_mat>
{
	auto poly = _poly;

	// transform each vertex of the polygon
	// TODO: check for homogeneous coordinates!
	for(t_vec& vec : poly)
		vec = mat * vec;

	return intersect_line_poly<t_vec, t_cont>(lineOrg, lineDir, poly);
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


/**
 * uv coordinates of a point inside a polygon defined by three vertices
 */
template<class t_vec>
t_vec poly_uv_ortho(const t_vec& vert1, const t_vec& vert2, const t_vec& vert3,
	const t_vec& uv1, const t_vec& uv2, const t_vec& uv3,
	const t_vec& _pt)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;

	t_vec vec12 = vert2 - vert1;
	t_vec vec13 = vert3 - vert1;

	t_vec uv12 = uv2 - uv1;
	t_vec uv13 = uv3 - uv1;


	// ----------------------------------------------------
	// orthonormalisation
	const T len12 = norm<t_vec>(vec12);
	const T len13 = norm<t_vec>(vec13);
	const T lenuv12 = norm<t_vec>(uv12);
	const T lenuv13 = norm<t_vec>(uv13);
	auto vecBasis = orthonorm_sys<t_vec, std::initializer_list, std::vector>({vec12, vec13});
	auto uvBasis = orthonorm_sys<t_vec, std::initializer_list, std::vector>({uv12, uv13});
	vec12 = vecBasis[0]*len12; vec13 = vecBasis[1]*len13;
	uv12 = uvBasis[0]*lenuv12; uv13 = uvBasis[1]*lenuv13;
	// ----------------------------------------------------


	const t_vec pt = _pt - vert1;

	// project a point onto a vector and return the fraction along that vector
	auto project_lam = [](const t_vec& vec, const t_vec& vecProj) -> T
	{
		const T len = norm<t_vec>(vecProj);
		const t_vec _vecProj = vecProj / len;
		T lam = inner<t_vec>(vec, _vecProj);
		return lam / len;
	};

	T lam12 = project_lam(pt, vec12);
	T lam13 = project_lam(pt, vec13);

	// uv coordinates at specified point
	const t_vec uv_pt = uv1 + lam12*uv12 + lam13*uv13;
	return uv_pt;
}


/**
 * uv coordinates of a point inside a polygon defined by three vertices
 * (more general version than poly_uv_ortho)
 */
template<class t_mat, class t_vec>
t_vec poly_uv(const t_vec& vert1, const t_vec& vert2, const t_vec& vert3,
	const t_vec& uv1, const t_vec& uv2, const t_vec& uv3,
	const t_vec& _pt)
requires is_mat<t_mat> && is_vec<t_vec>
{
	using T = typename t_vec::value_type;

	t_vec vec12 = vert2 - vert1;
	t_vec vec13 = vert3 - vert1;
	t_vec vecnorm = cross<t_vec>({vec12, vec13});

	// basis
	const t_mat basis = create<t_mat, t_vec>({vec12, vec13, vecnorm}, false);

	// reciprocal basis, RECI = REAL^(-T)
	const auto [basisInv, bOk] = inv<t_mat>(basis);
	if(!bOk) return zero<t_vec>(uv1.size());

	t_vec pt = _pt - vert1;		// real pt
	pt = basisInv * pt;			// reciprocal pt

	// uv coordinates at specified point
	t_vec uv12 = uv2 - uv1;
	t_vec uv13 = uv3 - uv1;

	// pt has components in common reciprocal basis
	// assumes that both vector and uv coordinates have the same reciprocal basis
	return uv1 + pt[0]*uv12 + pt[1]*uv13;
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
		mat = unit<t_mat>(mat.size1(), mat.size2());

	mat(0,0) = 0; 		mat(0,1) = -vec[2]; 	mat(0,2) = vec[1];
	mat(1,0) = vec[2]; 	mat(1,1) = 0; 			mat(1,2) = -vec[0];
	mat(2,0) = -vec[1]; mat(2,1) = vec[0]; 		mat(2,2) = 0;

	return mat;
}


/**
 * SO(3) matrix to rotate around an axis
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
	unit<t_mat>(matProj, 3,3, matProj.size1(), matProj.size2());
	return matProj;
}


/**
 * matrix to rotate vector vec1 into vec2
 */
template<class t_mat, class t_vec>
t_mat rotation(const t_vec& vec1, const t_vec& vec2)
requires is_vec<t_vec> && is_mat<t_mat>
{
	using t_real = typename t_vec::value_type;
	constexpr t_real eps = 1e-6;

	// rotation axis
	t_vec axis = cross<t_vec>({ vec1, vec2 });
	const t_real lenaxis = norm<t_vec>(axis);

	// rotation angle
	const t_real angle = std::atan2(lenaxis, inner<t_vec>(vec1, vec2));
	//std::cout << angle << " " << std::fmod(angle, pi<t_real>) << std::endl;

	// collinear vectors?
	if(equals<t_real>(angle, 0, eps))
		return unit<t_mat>(vec1.size());
	// antiparallel vectors?
	if(equals<t_real>(std::abs(angle), pi<t_real>, eps))
	{
		t_mat mat = -unit<t_mat>(vec1.size());
		// e.g. homogeneous coordinates -> only have -1 on the first 3 diagonal elements
		for(std::size_t i=3; i<std::min(mat.size1(), mat.size2()); ++i)
			mat(i,i) = 1;
		return mat;
	}

	axis /= lenaxis;
	t_mat mat = rotation<t_mat, t_vec>(axis, angle, true);
	return mat;
}



/**
 * extracts lines from polygon object, takes input from e.g. create_cube()
 * returns [point pairs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
t_cont<t_vec> create_lines(const t_cont<t_vec>& vertices, const t_cont<t_cont<std::size_t>>& faces)
requires is_vec<t_vec>
{
	t_cont<t_vec> lineverts;

	auto line_already_seen = [&lineverts](const t_vec& vec1, const t_vec& vec2) -> bool
	{
		auto iter = lineverts.begin();

		while(1)
		{
			const t_vec& linevec1 = *iter;
			std::advance(iter, 1); if(iter == lineverts.end()) break;
			const t_vec& linevec2 = *iter;

			if(equals<t_vec>(vec1, linevec1) && equals<t_vec>(vec2, linevec2))
				return true;
			if(equals<t_vec>(vec1, linevec2) && equals<t_vec>(vec2, linevec1))
				return true;

			std::advance(iter, 1); if(iter == lineverts.end()) break;
		}

		return false;
	};

	for(const auto& face : faces)
	{
		// iterator to last point
		auto iter1 = face.begin();
		std::advance(iter1, face.size()-1);

		for(auto iter2 = face.begin(); iter2 != face.end(); std::advance(iter2, 1))
		{
			const t_vec& vec1 = vertices[*iter1];
			const t_vec& vec2 = vertices[*iter2];

			//if(!line_already_seen(vec1, vec2))
			{
				lineverts.push_back(vec1);
				lineverts.push_back(vec2);
			}

			iter1 = iter2;
		}
	}

	return lineverts;
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
	t_cont<t_vec> triag_normals;
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

		const t_vec *puv1 = nullptr;
		const t_vec *puv2 = nullptr;
		const t_vec *puv3 = nullptr;

		typename t_cont<t_vec>::const_iterator iterFaceUVIdx;
		if(iterUVs != uvs.end() && iterFaceUVIdx != iterUVs->end())
		{
			iterFaceUVIdx = iterUVs->begin();

			puv1 = &(*iterFaceUVIdx);
			std::advance(iterFaceUVIdx, 1);
			puv2 = &(*iterFaceUVIdx);
		}

		// iterate over face vertices
		while(1)
		{
			std::advance(iterFaceVertIdx, 1);
			if(iterFaceVertIdx == iterFaces->end())
				break;
			std::size_t vert3Idx = *iterFaceVertIdx;

			if(iterUVs != uvs.end() && iterFaceUVIdx != iterUVs->end())
			{
				std::advance(iterFaceUVIdx, 1);
				puv3 = &(*iterFaceUVIdx);
			}

			// create triangle
			triangles.push_back(vertices[vert1Idx]);
			triangles.push_back(vertices[vert2Idx]);
			triangles.push_back(vertices[vert3Idx]);

			// triangle normal
			triag_normals.push_back(*iterNorms);
			//triag_normals.push_back(*iterNorms);
			//triag_normals.push_back(*iterNorms);

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

	return std::make_tuple(triangles, triag_normals, vert_uvs);
}


/**
 * subdivides triangles
 * input: [triangle vertices, normals, uvs]
 * returns [triangles, face normals, vertex uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_vec>, t_cont<t_vec>>
subdivide_triangles(const std::tuple<t_cont<t_vec>, t_cont<t_vec>, t_cont<t_vec>>& tup)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;

	const t_cont<t_vec>& vertices = std::get<0>(tup);
	const t_cont<t_vec>& normals = std::get<1>(tup);
	const t_cont<t_vec>& uvs = std::get<2>(tup);

	t_cont<t_vec> vertices_new;
	t_cont<t_vec> normals_new;
	t_cont<t_vec> uvs_new;


	// iterate over triplets forming triangles
	auto itervert = vertices.begin();
	auto iternorm = normals.begin();
	auto iteruv = uvs.begin();

	while(itervert != vertices.end())
	{
		const t_vec& vec1 = *itervert;
		std::advance(itervert, 1); if(itervert == vertices.end()) break;
		const t_vec& vec2 = *itervert;
		std::advance(itervert, 1); if(itervert == vertices.end()) break;
		const t_vec& vec3 = *itervert;
		std::advance(itervert, 1);

		const t_vec vec12mid = avg<t_vec>({ vec1, vec2 });
		const t_vec vec23mid = avg<t_vec>({ vec2, vec3 });
		const t_vec vec31mid = avg<t_vec>({ vec3, vec1 });

		// triangle 1
		vertices_new.push_back(vec1);
		vertices_new.push_back(vec12mid);
		vertices_new.push_back(vec31mid);

		// triangle 2
		vertices_new.push_back(vec12mid);
		vertices_new.push_back(vec2);
		vertices_new.push_back(vec23mid);

		// triangle 3
		vertices_new.push_back(vec31mid);
		vertices_new.push_back(vec23mid);
		vertices_new.push_back(vec3);

		// triangle 4
		vertices_new.push_back(vec12mid);
		vertices_new.push_back(vec23mid);
		vertices_new.push_back(vec31mid);


		// duplicate normals for the four sub-triangles
		if(iternorm != normals.end())
		{
			normals_new.push_back(*iternorm);
			normals_new.push_back(*iternorm);
			normals_new.push_back(*iternorm);
			normals_new.push_back(*iternorm);

			std::advance(iternorm, 1);
		}


		// uv coords
		if(iteruv != uvs.end())
		{
			// uv coords at vertices
			const t_vec& uv1 = *iteruv;
			std::advance(iteruv, 1); if(iteruv == uvs.end()) break;
			const t_vec& uv2 = *iteruv;
			std::advance(iteruv, 1); if(iteruv == uvs.end()) break;
			const t_vec& uv3 = *iteruv;
			std::advance(iteruv, 1);

			const t_vec uv12mid = avg<t_vec>({ uv1, uv2 });
			const t_vec uv23mid = avg<t_vec>({ uv2, uv3 });
			const t_vec uv31mid = avg<t_vec>({ uv3, uv1 });

			// uvs of triangle 1
			uvs_new.push_back(uv1);
			uvs_new.push_back(uv12mid);
			uvs_new.push_back(uv31mid);

			// uvs of triangle 2
			uvs_new.push_back(uv12mid);
			uvs_new.push_back(uv2);
			uvs_new.push_back(uv23mid);

			// uvs of triangle 3
			uvs_new.push_back(uv31mid);
			uvs_new.push_back(uv23mid);
			uvs_new.push_back(uv3);

			// uvs of triangle 4
			uvs_new.push_back(uv12mid);
			uvs_new.push_back(uv23mid);
			uvs_new.push_back(uv31mid);
		}
	}

	return std::make_tuple(vertices_new, normals_new, uvs_new);
}


/**
 * subdivides triangles (with specified number of iterations)
 * input: [triangle vertices, normals, uvs]
 * returns [triangles, face normals, vertex uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_vec>, t_cont<t_vec>>
subdivide_triangles(const std::tuple<t_cont<t_vec>, t_cont<t_vec>, t_cont<t_vec>>& tup, std::size_t iters)
requires is_vec<t_vec>
{
	auto tupDiv = tup;
	for(std::size_t i=0; i<iters; ++i)
		tupDiv = subdivide_triangles<t_vec, t_cont>(tupDiv);
	return tupDiv;
}


/**
 * create the faces of a sphere
 * input: [triangle vertices, normals, uvs] (like subdivide_triangles)
 * returns [triangles, face normals, vertex uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_vec>, t_cont<t_vec>>
spherify(const std::tuple<t_cont<t_vec>, t_cont<t_vec>, t_cont<t_vec>>& tup,
	typename t_vec::value_type rad = 1)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;

	const t_cont<t_vec>& vertices = std::get<0>(tup);
	const t_cont<t_vec>& normals = std::get<1>(tup);
	const t_cont<t_vec>& uvs = std::get<2>(tup);

	t_cont<t_vec> vertices_new;
	t_cont<t_vec> normals_new;


	// vertices
	for(t_vec vec : vertices)
	{
		vec /= norm<t_vec>(vec);
		vec *= rad;
		vertices_new.emplace_back(std::move(vec));
	}


	// normals
	auto itervert = vertices.begin();
	// iterate over triplets forming triangles
	while(itervert != vertices.end())
	{
		const t_vec& vec1 = *itervert;
		std::advance(itervert, 1); if(itervert == vertices.end()) break;
		const t_vec& vec2 = *itervert;
		std::advance(itervert, 1); if(itervert == vertices.end()) break;
		const t_vec& vec3 = *itervert;
		std::advance(itervert, 1);

		t_vec vecmid = avg<t_vec>({ vec1, vec2, vec3 });
		vecmid /= norm<t_vec>(vecmid);
		normals_new.emplace_back(std::move(vecmid));
	}

	return std::make_tuple(vertices_new, normals_new, uvs);
}

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// 3-dim solids
// ----------------------------------------------------------------------------

/**
 * create a plane
 * returns [vertices, face vertex indices, face normals, face uvs]
 */
template<class t_mat, class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_cont<std::size_t>>, t_cont<t_vec>, t_cont<t_cont<t_vec>>>
create_plane(const t_vec& norm, typename t_vec::value_type l=1)
requires is_vec<t_vec>
{
	using t_real = typename t_vec::value_type;
	//using t_mat = m::mat<t_real, std::vector>;
	//using namespace m_ops;

	t_vec norm_old = create<t_vec>({ 0, 0, -1 });
	t_mat rot = rotation<t_mat, t_vec>(norm_old, norm);

	t_cont<t_vec> vertices =
	{
		create<t_vec>({ -l, -l, 0. }),	// vertex 0
		create<t_vec>({ +l, -l, 0. }),	// vertex 1
		create<t_vec>({ +l, +l, 0. }),	// vertex 2
		create<t_vec>({ -l, +l, 0. }),	// vertex 3
	};

	// rotate according to given normal
	for(t_vec& vec : vertices)
		vec = rot * vec;

	t_cont<t_cont<std::size_t>> faces = { { 0, 1, 2, 3 } };
	t_cont<t_vec> normals = { norm };

	t_cont<t_cont<t_vec>> uvs =
	{{
		create<t_vec>({0,0}),
		create<t_vec>({1,0}),
		create<t_vec>({1,1}),
		create<t_vec>({0,1}),
	}};

	return std::make_tuple(vertices, faces, normals, uvs);
}



/**
 * create a disk
 * returns [vertices, face vertex indices, face normals, face uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_cont<std::size_t>>, t_cont<t_vec>, t_cont<t_cont<t_vec>>>
create_disk(typename t_vec::value_type r = 1, std::size_t num_points=32)
requires is_vec<t_vec>
{
	using t_real = typename t_vec::value_type;

	// vertices
	t_cont<t_vec> vertices;

	// inner vertex
	//vertices.push_back(create<t_vec>({ 0, 0, 0 }));
	for(std::size_t pt=0; pt<num_points; ++pt)
	{
		const t_real phi = t_real(pt)/t_real(num_points) * t_real(2)*pi<t_real>;
		const t_real c = std::cos(phi);
		const t_real s = std::sin(phi);

		// outer vertices
		t_vec vert = create<t_vec>({ r*c, r*s, 0 });
		vertices.emplace_back(std::move(vert));
	}

	// faces, normals & uvs
	t_cont<t_cont<std::size_t>> faces;
	t_cont<t_vec> normals;
	t_cont<t_cont<t_vec>> uvs;	// TODO

	// directly generate triangles
	/*for(std::size_t face=0; face<num_points; ++face)
	{
		std::size_t idx0 = face + 1;	// outer 1
		std::size_t idx1 = (face == num_points-1 ? 1 : face + 2);	// outer 2
		std::size_t idx2 = 0;	// inner

		faces.push_back({ idx0, idx1, idx2 });
	}*/

	t_cont<std::size_t> face(num_points);
	std::iota(face.begin(), face.end(), 0);
	faces.push_back(face);
	normals.push_back(create<t_vec>({0,0,1}));

	return std::make_tuple(vertices, faces, normals, uvs);
}


/**
 * create a cone
 * returns [vertices, face vertex indices, face normals, face uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_cont<std::size_t>>, t_cont<t_vec>, t_cont<t_cont<t_vec>>>
create_cone(typename t_vec::value_type r = 1, typename t_vec::value_type h = 1,
	bool bWithCap = true, std::size_t num_points = 32)
requires is_vec<t_vec>
{
	using t_real = typename t_vec::value_type;

	// vertices
	t_cont<t_vec> vertices;

	// inner vertex
	vertices.push_back(create<t_vec>({ 0, 0, h }));

	for(std::size_t pt=0; pt<num_points; ++pt)
	{
		const t_real phi = t_real(pt)/t_real(num_points) * t_real(2)*pi<t_real>;
		const t_real c = std::cos(phi);
		const t_real s = std::sin(phi);

		// outer vertices
		t_vec vert = create<t_vec>({ r*c, r*s, 0 });
		vertices.emplace_back(std::move(vert));
	}

	// faces, normals & uvs
	t_cont<t_cont<std::size_t>> faces;
	t_cont<t_vec> normals;
	t_cont<t_cont<t_vec>> uvs;	// TODO

	for(std::size_t face=0; face<num_points; ++face)
	{
			std::size_t idx0 = face + 1;	// outer 1
			std::size_t idx1 = (face == num_points-1 ? 1 : face + 2);	// outer 2
			std::size_t idx2 = 0;	// inner

			faces.push_back({ idx0, idx1, idx2 });


			t_vec n = cross<t_vec>({vertices[idx2]-vertices[idx0], vertices[idx1]-vertices[idx0]});
			n /= norm<t_vec>(n);

			normals.push_back(n);
	}


	if(bWithCap)
	{
		const auto [disk_vertices, disk_faces, disk_normals, disk_uvs] = create_disk<t_vec, t_cont>(r, num_points);

		// vertex indices have to be adapted for merging
		const std::size_t vert_start_idx = vertices.size();
		vertices.insert(vertices.end(), disk_vertices.begin(), disk_vertices.end());

		auto disk_faces_bottom = disk_faces;
		for(auto& disk_face : disk_faces_bottom)
		{
			for(auto& disk_face_idx : disk_face)
				disk_face_idx += vert_start_idx;
			std::reverse(disk_face.begin(), disk_face.end());
		}
		faces.insert(faces.end(), disk_faces_bottom.begin(), disk_faces_bottom.end());

		for(const auto& normal : disk_normals)
			normals.push_back(-normal);

		uvs.insert(uvs.end(), disk_uvs.begin(), disk_uvs.end());
	}

	return std::make_tuple(vertices, faces, normals, uvs);
}


/**
 * create a cylinder
 * cyltype: 0 (no caps), 1 (with caps), 2 (arrow)
 * returns [vertices, face vertex indices, face normals, face uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_cont<std::size_t>>, t_cont<t_vec>, t_cont<t_cont<t_vec>>>
create_cylinder(typename t_vec::value_type r = 1, typename t_vec::value_type h = 1,
	int cyltype = 0, std::size_t num_points = 32,
	typename t_vec::value_type arrow_r = 1.5, typename t_vec::value_type arrow_h = 0.5)
requires is_vec<t_vec>
{
	using t_real = typename t_vec::value_type;

	// vertices
	t_cont<t_vec> vertices;
	t_cont<t_real> vertices_u;

	for(std::size_t pt=0; pt<num_points; ++pt)
	{
		const t_real u = t_real(pt)/t_real(num_points);
		const t_real phi = u * t_real(2)*pi<t_real>;
		const t_real c = std::cos(phi);
		const t_real s = std::sin(phi);

		t_vec top = create<t_vec>({ r*c, r*s, h*t_real(0.5) });
		t_vec bottom = create<t_vec>({ r*c, r*s, -h*t_real(0.5) });

		vertices.emplace_back(std::move(top));
		vertices.emplace_back(std::move(bottom));

		vertices_u.push_back(u);
	}

	// faces, normals & uvs
	t_cont<t_cont<std::size_t>> faces;
	t_cont<t_vec> normals;
	t_cont<t_cont<t_vec>> uvs;

	for(std::size_t face=0; face<num_points; ++face)
	{
		std::size_t idx0 = face*2 + 0;	// top 1
		std::size_t idx1 = face*2 + 1;	// bottom 1
		std::size_t idx2 = (face >= num_points-1 ? 1 : face*2 + 3);	// bottom 2
		std::size_t idx3 = (face >= num_points-1 ? 0 : face*2 + 2);	// top 2

		t_vec n = cross<t_vec>({vertices[idx3]-vertices[idx0], vertices[idx1]-vertices[idx0]});
		n /= norm<t_vec>(n);

		faces.push_back({ idx0, idx1, idx2, idx3 });
		normals.emplace_back(std::move(n));


		t_real u1 = vertices_u[idx0];
		t_real u2 = (face >= num_points-1 ? 1 : vertices_u[idx3]);
		uvs.push_back({ create<t_vec>({u1,1}), create<t_vec>({u1,0}),
			create<t_vec>({u2,0}), create<t_vec>({u2,1}) });
	}


	if(cyltype > 0)
	{
		const auto [disk_vertices, disk_faces, disk_normals, disk_uvs] = create_disk<t_vec, t_cont>(r, num_points);

		// bottom lid
		// vertex indices have to be adapted for merging
		std::size_t vert_start_idx = vertices.size();
		const t_vec top = create<t_vec>({ 0, 0, h*t_real(0.5) });

		for(const auto& disk_vert : disk_vertices)
			vertices.push_back(disk_vert - top);

		auto disk_faces_bottom = disk_faces;
		for(auto& disk_face : disk_faces_bottom)
		{
			for(auto& disk_face_idx : disk_face)
				disk_face_idx += vert_start_idx;
			std::reverse(disk_face.begin(), disk_face.end());
		}
		faces.insert(faces.end(), disk_faces_bottom.begin(), disk_faces_bottom.end());

		for(const auto& normal : disk_normals)
			normals.push_back(-normal);

		uvs.insert(uvs.end(), disk_uvs.begin(), disk_uvs.end());


		vert_start_idx = vertices.size();

		if(cyltype == 1)	// top lid
		{
			for(const auto& disk_vert : disk_vertices)
				vertices.push_back(disk_vert + top);

			auto disk_faces_top = disk_faces;
			for(auto& disk_face : disk_faces_top)
				for(auto& disk_face_idx : disk_face)
					disk_face_idx += vert_start_idx;
			faces.insert(faces.end(), disk_faces_top.begin(), disk_faces_top.end());

			for(const auto& normal : disk_normals)
				normals.push_back(normal);

			uvs.insert(uvs.end(), disk_uvs.begin(), disk_uvs.end());
		}
		else if(cyltype == 2)	// arrow top
		{
			// no need to cap the arrow if the radii are equal
			bool bConeCap = !equals<t_real>(r, arrow_r);

			const auto [cone_vertices, cone_faces, cone_normals, cone_uvs] =
				create_cone<t_vec, t_cont>(arrow_r, arrow_h, bConeCap, num_points);

			for(const auto& cone_vert : cone_vertices)
				vertices.push_back(cone_vert + top);

			auto cone_faces_top = cone_faces;
			for(auto& cone_face : cone_faces_top)
				for(auto& cone_face_idx : cone_face)
					cone_face_idx += vert_start_idx;
			faces.insert(faces.end(), cone_faces_top.begin(), cone_faces_top.end());

			for(const auto& normal : cone_normals)
				normals.push_back(normal);

			uvs.insert(uvs.end(), cone_uvs.begin(), cone_uvs.end());
		}
	}

	return std::make_tuple(vertices, faces, normals, uvs);
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
 * create the faces of a icosahedron
 * returns [vertices, face vertex indices, face normals, face uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_cont<std::size_t>>, t_cont<t_vec>, t_cont<t_cont<t_vec>>>
create_icosahedron(typename t_vec::value_type l = 1)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;
	const T g = golden<T>;

	t_cont<t_vec> vertices =
	{
		create<t_vec>({ 0, -l, -g*l }), create<t_vec>({ 0, -l, +g*l }),
		create<t_vec>({ 0, +l, -g*l }), create<t_vec>({ 0, +l, +g*l }),

		create<t_vec>({ -g*l, 0, -l }), create<t_vec>({ -g*l, 0, +l }),
		create<t_vec>({ +g*l, 0, -l }), create<t_vec>({ +g*l, 0, +l }),

		create<t_vec>({ -l, -g*l, 0 }), create<t_vec>({ -l, +g*l, 0 }),
		create<t_vec>({ +l, -g*l, 0 }), create<t_vec>({ +l, +g*l, 0 }),
	};

	t_cont<t_cont<std::size_t>> faces =
	{
		{ 4, 2, 0 }, { 0, 6, 10 }, { 10, 7, 1 }, { 1, 3, 5 }, { 5, 9, 4 },
		{ 7, 10, 6 }, { 6, 0, 2 }, { 2, 4, 9 }, { 9, 5, 3 }, { 3, 1, 7 },
		{ 0, 10, 8 }, { 10, 1, 8 }, { 1, 5, 8 }, { 5, 4, 8 }, { 4, 0, 8 },
		{ 3, 7, 11 }, { 7, 6, 11 }, { 6, 2, 11 }, { 2, 9, 11 }, { 9, 3, 11 },
	};


	t_cont<t_vec> normals;
	normals.reserve(faces.size());

	for(const auto& face : faces)
	{
		auto iter = face.begin();
		const t_vec& vec1 = *(vertices.begin() + *iter); std::advance(iter,1);
		const t_vec& vec2 = *(vertices.begin() + *iter); std::advance(iter,1);
		const t_vec& vec3 = *(vertices.begin() + *iter);

		const t_vec vec12 = vec2 - vec1;
		const t_vec vec13 = vec3 - vec1;

		t_vec n = cross<t_vec>({vec12, vec13});
		n /= norm<t_vec>(n);
		normals.emplace_back(std::move(n));
	}

	// TODO
	t_cont<t_cont<t_vec>> uvs =
	{
	};

	return std::make_tuple(vertices, faces, normals, uvs);
}


/**
 * create the faces of a octahedron
 * returns [vertices, face vertex indices, face normals, face uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_cont<std::size_t>>, t_cont<t_vec>, t_cont<t_cont<t_vec>>>
create_octahedron(typename t_vec::value_type l = 1)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;

	t_cont<t_vec> vertices =
	{
		create<t_vec>({ +l, 0, 0 }),	// vertex 0
		create<t_vec>({ 0, +l, 0 }),	// vertex 1
		create<t_vec>({ 0, 0, +l }),	// vertex 2

		create<t_vec>({ -l, 0, 0 }),	// vertex 3
		create<t_vec>({ 0, -l, 0 }),	// vertex 4
		create<t_vec>({ 0, 0, -l }),	// vertex 5
	};

	t_cont<t_cont<std::size_t>> faces =
	{
		{ 2, 0, 1 }, { 0, 5, 1 }, { 5, 3, 1 }, { 3, 2, 1 },	// upper half
		{ 0, 2, 4 }, { 5, 0, 4 }, { 3, 5, 4 }, { 2, 3, 4 },	// lower half
	};


	const T len = std::sqrt(3);

	t_cont<t_vec> normals =
	{
		create<t_vec>({ +1/len, +1/len, +1/len }),
		create<t_vec>({ +1/len, +1/len, -1/len }),
		create<t_vec>({ -1/len, +1/len, -1/len }),
		create<t_vec>({ -1/len, +1/len, +1/len }),

		create<t_vec>({ +1/len, -1/len, +1/len }),
		create<t_vec>({ +1/len, -1/len, -1/len }),
		create<t_vec>({ -1/len, -1/len, -1/len }),
		create<t_vec>({ -1/len, -1/len, +1/len }),
	};

	t_cont<t_cont<t_vec>> uvs =
	{
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },

		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
	};

	return std::make_tuple(vertices, faces, normals, uvs);
}


/**
 * create the faces of a tetrahedron
 * returns [vertices, face vertex indices, face normals, face uvs]
 */
template<class t_vec, template<class...> class t_cont = std::vector>
std::tuple<t_cont<t_vec>, t_cont<t_cont<std::size_t>>, t_cont<t_vec>, t_cont<t_cont<t_vec>>>
create_tetrahedron(typename t_vec::value_type l = 1)
requires is_vec<t_vec>
{
	using T = typename t_vec::value_type;

	t_cont<t_vec> vertices =
	{
		create<t_vec>({ -l, -l, +l }),	// vertex 0
		create<t_vec>({ +l, +l, +l }),	// vertex 1
		create<t_vec>({ -l, +l, -l }),	// vertex 2
		create<t_vec>({ +l, -l, -l }),	// vertex 3
	};

	t_cont<t_cont<std::size_t>> faces =
	{
		{ 1, 2, 0 }, { 2, 1, 3 },	// connected to upper edge
		{ 0, 3, 1 }, { 3, 0, 2 },	// connected to lower edge
	};


	const T len = std::sqrt(3);

	t_cont<t_vec> normals =
	{
		create<t_vec>({ -1/len, +1/len, +1/len }),
		create<t_vec>({ +1/len, +1/len, -1/len }),
		create<t_vec>({ +1/len, -1/len, +1/len }),
		create<t_vec>({ -1/len, -1/len, -1/len }),
	};

	t_cont<t_cont<t_vec>> uvs =
	{
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
		{ create<t_vec>({0,0}), create<t_vec>({1,0}), create<t_vec>({0.5,1}) },
	};

	return std::make_tuple(vertices, faces, normals, uvs);
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


/**
 * translation matrix in homogeneous coordinates
 */
template<class t_mat, class t_real = typename t_mat::value_type>
t_mat hom_translation(t_real x, t_real y, t_real z)
requires is_mat<t_mat>
{
	return create<t_mat>({
		1., 	0., 	0., 	x,
		0., 	1., 	0., 	y,
		0.,		0.,		1., 	z,
		0.,		0.,		0.,		1.
	});
}


/**
 * scaling matrix in homogeneous coordinates
 */
template<class t_mat, class t_real = typename t_mat::value_type>
t_mat hom_scaling(t_real x, t_real y, t_real z)
requires is_mat<t_mat>
{
	return create<t_mat>({
		x, 		0., 	0., 	0.,
		0., 	y, 		0., 	0.,
		0.,		0.,		z, 		0.,
		0.,		0.,		0.,		1.
	});
}

// ----------------------------------------------------------------------------





// ----------------------------------------------------------------------------
// complex algos
// ----------------------------------------------------------------------------

/**
 * SU(2) generators, pauli matrices sig_i = 2*S_i
 */
template<class t_mat>
const t_mat& su2_matrix(std::size_t which)
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	using t_cplx = typename t_mat::value_type;
	constexpr t_cplx c0(0,0);
	constexpr t_cplx c1(1,0);
	constexpr t_cplx cI(0,1);

	static const t_mat mat[] =
	{
		create<t_mat>({{c0, c1}, { c1,  c0}}),	// x
		create<t_mat>({{c0, cI}, {-cI,  c0}}),	// y
		create<t_mat>({{c1, c0}, { c0, -c1}}),	// z
	};

	return mat[which];
}


/**
 * get a vector of pauli matrices
 */
template<class t_vec>
t_vec su2_matrices(bool bIncludeUnit = false)
requires is_basic_vec<t_vec> && is_mat<typename t_vec::value_type>
	&& is_complex<typename t_vec::value_type::value_type>
{
	using t_mat = typename t_vec::value_type;

	t_vec vec;
	if(bIncludeUnit)
		vec.emplace_back(unit<t_mat>(2));
	for(std::size_t i=0; i<3; ++i)
		vec.emplace_back(su2_matrix<t_mat>(i));

	return vec;
}


/**
 * project the vector of SU(2) matrices onto a vector
 * proj = <sigma|vec>
 */
template<class t_vec, class t_mat>
t_mat proj_su2(const t_vec& vec, bool bIsNormalised=1)
requires is_vec<t_vec> && is_mat<t_mat>
{
	typename t_vec::value_type len = 1;
	if(!bIsNormalised)
		len = norm<t_vec>(vec);

	const auto sigma = su2_matrices<std::vector<t_mat>>(false);
	return inner<std::vector<t_mat>, t_vec>(sigma, vec);
}


/**
 * SU(2) ladders
 */
template<class t_mat>
const t_mat& su2_ladder(std::size_t which)
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	using t_cplx = typename t_mat::value_type;
	constexpr t_cplx cI(0,1);
	constexpr t_cplx c05(0.5, 0);

	static const t_mat mat[] =
	{
		c05*su2_matrix<t_mat>(0) + c05*cI*su2_matrix<t_mat>(1),	// up
		c05*su2_matrix<t_mat>(0) - c05*cI*su2_matrix<t_mat>(1),	// down
	};

	return mat[which];
}


/**
 * SU(3) generators, gell-mann matrices
 * see: https://de.wikipedia.org/wiki/Gell-Mann-Matrizen
 */
template<class t_mat>
const t_mat& su3_matrix(std::size_t which)
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	using t_cplx = typename t_mat::value_type;
	using t_real = typename t_cplx::value_type;
	constexpr t_cplx c0(0,0);
	constexpr t_cplx c1(1,0);
	constexpr t_cplx c2(2,0);
	constexpr t_cplx cI(0,1);
	constexpr t_real s3 = std::sqrt(3.);

	static const t_mat mat[] =
	{
		create<t_mat>({{c0,c1,c0}, {c1,c0,c0}, {c0,c0,c0}}),			// 1
		create<t_mat>({{c0,cI,c0}, {-cI,c0,c0}, {c0,c0,c0}}),			// 2
		create<t_mat>({{c1,c0,c0}, {c0,-c1,c0}, {c0,c0,c0}}),			// 3
		create<t_mat>({{c0,c0,c1}, {c0,c0,c0}, {c1,c0,c0}}),			// 4
		create<t_mat>({{c0,c0,cI}, {c0,c0,c0}, {-cI,c0,c0}}),			// 5
		create<t_mat>({{c0,c0,c0}, {c0,c0,c1}, {c0,c1,c0}}),			// 6
		create<t_mat>({{c0,c0,c0}, {c0,c0,cI}, {c0,-cI,c0}}),			// 7
		create<t_mat>({{c1/s3,c0,c0}, {c0,c1/s3,c0}, {c0,c0,-c2/s3*c1}}),	// 8
	};

	return mat[which];
}


/**
 * crystallographic B matrix, B = 2pi * A^(-T)
 * after: https://en.wikipedia.org/wiki/Fractional_coordinates
 */
template<class t_mat, class t_real = typename t_mat::value_type>
t_mat B_matrix(t_real a, t_real b, t_real c, t_real _aa, t_real _bb, t_real _cc)
requires is_mat<t_mat>
{
	const t_real sc = std::sin(_cc);
	const t_real ca = std::cos(_aa);
	const t_real cb = std::cos(_bb);
	const t_real cc = std::cos(_cc);
	const t_real rr = std::sqrt(1. + 2.*ca*cb*cc - (ca*ca + cb*cb + cc*cc));

	return t_real(2)*pi<t_real> * create<t_mat>({
		1./a,				0.,				0.,
		-1./a * cc/sc,		1./b * 1./sc,			0.,
		(cc*ca - cb)/(a*sc*rr), 	(cb*cc-ca)/(b*sc*rr),		sc/(c*rr)
	});
}


/**
 * general structure factor calculation
 * e.g. type T as vector (complex number) for magnetic (nuclear) structure factor
 * Ms_or_bs:
	- nuclear scattering lengths for nuclear neutron scattering or
	- atomic form factors for x-ray scattering
	- magnetisation (* magnetic form factor) for magnetic neutron scattering
 * Rs: atomic positions
 * Q: scattering vector G for nuclear scattering or G+k for magnetic scattering with propagation vector k
 * fs: optional magnetic form factors
 */
template<class t_vec, class T = t_vec, template<class...> class t_cont = std::vector,
	class t_cplx = std::complex<double>>
T structure_factor(const t_cont<T>& Ms_or_bs, const t_cont<t_vec>& Rs, const t_vec& Q, const t_vec* fs=nullptr)
requires is_basic_vec<t_vec>
{
	using t_real = typename t_cplx::value_type;
	constexpr t_cplx cI(0,1);
	constexpr t_real twopi = pi<t_real> * t_real(2);
	constexpr t_real expsign = -1;

	T F;
	if constexpr(is_vec<T>)
		F = zero<T>(Rs.begin()->size());	// always 3 dims...
	else if constexpr(is_complex<T>)
		F = T(0);

	auto iterM_or_b = Ms_or_bs.begin();
	auto iterR = Rs.begin();
	typename t_vec::const_iterator iterf;
	if(fs) iterf = fs->begin();

	while(iterM_or_b != Ms_or_bs.end() && iterR != Rs.end())
	{
		// if form factors are given, use them, otherwise set to 1
		t_real f = t_real(1);
		if(fs)
		{
			auto fval = *iterf;
			if constexpr(is_complex<decltype(fval)>)
				f = fval.real();
			else
				f = fval;
		}

		// structure factor
		F += (*iterM_or_b) * f * std::exp(expsign * cI * twopi * inner<t_vec>(Q, *iterR));

		// next M or b if available (otherwise keep current)
		auto iterM_or_b_next = std::next(iterM_or_b, 1);
		if(iterM_or_b_next != Ms_or_bs.end())
			iterM_or_b = iterM_or_b_next;

		if(fs)
		{
			// next form factor if available (otherwise keep current)
			auto iterf_next = std::next(iterf, 1);
			if(iterf_next != fs->end())
				iterf = iterf_next;
		}

		// next atomic position
		std::advance(iterR, 1);
	}

	return F;
}


// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------
// polarisation
// ----------------------------------------------------------------------------


/**
 * conjugate complex vector
 */
template<class t_vec>
t_vec conj(const t_vec& vec)
requires is_basic_vec<t_vec>
{
	const std::size_t N = vec.size();
	t_vec vecConj = zero<t_vec>(N);

	for(std::size_t iComp=0; iComp<N; ++iComp)
	{
		if constexpr(is_complex<typename t_vec::value_type>)
			vecConj[iComp] = std::conj(vec[iComp]);
		else	// simply copy non-complex vector
			vecConj[iComp] = vec[iComp];
	}

	return vecConj;
}


/**
 * hermitian conjugate complex matrix
 */
template<class t_mat>
t_mat herm(const t_mat& mat)
requires is_basic_mat<t_mat>
{
	t_mat mat2;
	if constexpr(is_dyn_mat<t_mat>)
		mat2 = t_mat(mat.size2(), mat.size1());

	for(std::size_t i=0; i<mat.size1(); ++i)
		for(std::size_t j=0; j<mat.size2(); ++j)
		{
			if constexpr(is_complex<typename t_mat::value_type>)
				mat2(j,i) = std::conj(mat(i,j));
			else	// simply transpose non-complex matrix
				mat2(j,i) = mat(i,j);
		}

	return mat2;
}


/**
 * polarisation density matrix
 *
 * eigenvector expansion of a state: |psi> = a_i |xi_i>
 * mean value of operator with mixed states:
 * <A> = p_i * <a_i|A|a_i>
 * <A> = tr( A * p_i * |a_i><a_i| )
 * <A> = tr( A * rho )
 * polarisation density matrix: rho = 0.5 * (1 + <P|sigma>)
 */
template<class t_vec, class t_mat>
t_mat pol_density_mat(const t_vec& P, typename t_vec::value_type c=0.5)
requires is_vec<t_vec> && is_mat<t_mat>
{
	return (unit<t_mat>(2,2) + proj_su2<t_vec, t_mat>(P, true)) * c;
}


/**
 * Blume-Maleev equation (see: https://doi.org/10.1016/B978-044451050-1/50006-9 - p. 225)
 * returns scattering intensity and final polarisation vector
 */
template<class t_vec, typename t_cplx = typename t_vec::value_type>
std::tuple<t_cplx, t_vec> blume_maleev(const t_vec& P_i, const t_vec& Mperp, const t_cplx& N)
requires is_vec<t_vec>
{
	const t_vec MperpConj = conj(Mperp);
	const t_cplx NConj = std::conj(N);
	constexpr t_cplx imag(0, 1);

	// ------------------------------------------------------------------------
	// intensity
	// nuclear
	t_cplx I = N*NConj;

	// nuclear-magnetic
	I += NConj*inner<t_vec>(P_i, Mperp);
	I += N*inner<t_vec>(Mperp, P_i);

	// magnetic, non-chiral
	I += inner<t_vec>(Mperp, Mperp);

	// magnetic, chiral
	I += -imag * inner<t_vec>(P_i, cross<t_vec>({ Mperp, MperpConj }));
	// ------------------------------------------------------------------------

	// ------------------------------------------------------------------------
	// polarisation vector
	// nuclear
	t_vec P_f = P_i * N*NConj;

	// nuclear-magnetic
	P_f += NConj * Mperp;
	P_f += N * MperpConj;
	P_f += imag * N * cross<t_vec>({ P_i, MperpConj });
	P_f += -imag * NConj * cross<t_vec>({ P_i, Mperp });

	// magnetic, non-chiral
	P_f += Mperp * inner<t_vec>(Mperp, P_i);
	P_f += MperpConj * inner<t_vec>(P_i, Mperp);
	P_f += -P_i * inner<t_vec>(Mperp, Mperp);

	// magnetic, chiral
	P_f += imag * cross<t_vec>({ Mperp, MperpConj });
	// ------------------------------------------------------------------------

	return std::make_tuple(I, P_f/I);
}


/**
 * Blume-Maleev equation (see: https://doi.org/10.1016/B978-044451050-1/50006-9 - p. 225)
 * calculate indirectly with density matrix
 *
 * V   = N*1 + <Mperp|sigma>
 * I   = 0.5 * tr( V^H V rho )
 * P_f = 0.5 * tr( V^H sigma V rho ) / I
 *
 * returns scattering intensity and final polarisation vector
 */
template<class t_mat, class t_vec, typename t_cplx = typename t_vec::value_type>
std::tuple<t_cplx, t_vec> blume_maleev_indir(const t_vec& P_i, const t_vec& Mperp, const t_cplx& N)
requires is_mat<t_mat> && is_vec<t_vec>
{
	// spin-1/2
	constexpr t_cplx c = 0.5;

	// vector of pauli matrices
	const auto sigma = su2_matrices<std::vector<t_mat>>(false);

	// density matrix
	const auto density = pol_density_mat<t_vec, t_mat>(P_i, c);

	// potential
	const auto V_mag = proj_su2<t_vec, t_mat>(Mperp, true);
	const auto V_nuc = N * unit<t_mat>(2);
	const auto V = V_nuc + V_mag;
	const auto VConj = herm(V);

	// scattering intensity
	t_cplx I = c * trace(VConj*V * density/c);

	// ------------------------------------------------------------------------
	// scattered polarisation vector
	const auto m0 = (VConj * sigma[0]) * V * density/c;
	const auto m1 = (VConj * sigma[1]) * V * density/c;
	const auto m2 = (VConj * sigma[2]) * V * density/c;

	t_vec P_f = create<t_vec>({ c*trace(m0), c*trace(m1), c*trace(m2) });
	// ------------------------------------------------------------------------

	return std::make_tuple(I, P_f/I);
}


// ----------------------------------------------------------------------------
}
#endif
