/**
 * containers and operators for use with math algorithms
 * @author Tobias Weber
 * @date jan-18
 * @license: see 'LICENSE.EUPL' file
 */

#ifndef __MATH_CONTS_H__
#define __MATH_CONTS_H__

#include "math_concepts.h"
#include <cassert>
#include <vector>


namespace m {

// ----------------------------------------------------------------------------
// matrix
// ----------------------------------------------------------------------------

template<class T=double, template<class...> class t_cont = std::vector>
requires is_basic_vec<t_cont<T>> && is_dyn_vec<t_cont<T>>
class mat
{
public:
	using value_type = T;
	using container_type = t_cont<T>;

	mat() = default;
	mat(std::size_t ROWS, std::size_t COLS) : m_data(ROWS*COLS), m_rowsize(ROWS), m_colsize(COLS) {}
	~mat() = default;

	std::size_t size1() const { return m_rowsize; }
	std::size_t size2() const { return m_colsize; }
	const T& operator()(std::size_t row, std::size_t col) const { return m_data[row*m_colsize + col]; }
	T& operator()(std::size_t row, std::size_t col) { return m_data[row*m_colsize + col]; }

private:
	container_type m_data;
	std::size_t m_rowsize, m_colsize;
};
// ----------------------------------------------------------------------------

}


namespace m_ops {
// ----------------------------------------------------------------------------
// vector operators
// ----------------------------------------------------------------------------

/**
 * unary +
 */
template<class t_vec>
const t_vec& operator+(const t_vec& vec1)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	return vec1;
}


/**
 * unary -
 */
template<class t_vec>
t_vec operator-(const t_vec& vec1)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	t_vec vec(vec1.size());

	for(std::size_t i=0; i<vec1.size(); ++i)
		vec[i] = -vec1[i];

	return vec;
}


/**
 * binary +
 */
template<class t_vec>
t_vec operator+(const t_vec& vec1, const t_vec& vec2)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	if constexpr(m::is_dyn_vec<t_vec>)
		assert((vec1.size() == vec2.size()));
	else
		static_assert(vec1.size() == vec2.size());

	t_vec vec(vec1.size());

	for(std::size_t i=0; i<vec1.size(); ++i)
		vec[i] = vec1[i] + vec2[i];

	return vec;
}


/**
 * binary -
 */
template<class t_vec>
t_vec operator-(const t_vec& vec1, const t_vec& vec2)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	return vec1 + (-vec2);
}


/**
 * vector * scalar
 */
template<class t_vec>
t_vec operator*(const t_vec& vec1, typename t_vec::value_type d)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	t_vec vec(vec1.size());

	for(std::size_t i=0; i<vec1.size(); ++i)
		vec[i] = vec1[i] * d;

	return vec;
}


/**
 * scalar * vector
 */
template<class t_vec>
t_vec operator*(typename t_vec::value_type d, const t_vec& vec)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	return vec * d;
}

/**
 * vector / scalar
 */
template<class t_vec>
t_vec operator/(const t_vec& vec, typename t_vec::value_type d)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	using T = typename t_vec::value_type;
	return vec * (T(1)/d);
}


/**
 * vector += vector
 */
template<class t_vec>
t_vec& operator+=(t_vec& vec1, const t_vec& vec2)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	vec1 = vec1 + vec2;
	return vec1;
}

/**
 * vector -= vector
 */
template<class t_vec>
t_vec& operator-=(t_vec& vec1, const t_vec& vec2)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	vec1 = vec1 - vec2;
	return vec1;
}


/**
 * vector *= scalar
 */
template<class t_vec>
t_vec& operator*=(t_vec& vec1, typename t_vec::value_type d)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	vec1 = vec1 * d;
	return vec1;
}

/**
 * vector /= scalar
 */
template<class t_vec>
t_vec& operator/=(t_vec& vec1, typename t_vec::value_type d)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	vec1 = vec1 / d;
	return vec1;
}



/**
 * operator <<
 */
template<class t_vec>
std::ostream& operator<<(std::ostream& ostr, const t_vec& vec)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	const std::size_t N = vec.size();

	for(std::size_t i=0; i<N; ++i)
	{
		ostr << vec[i];
		if(i < N-1)
			ostr << ", ";
	}

	return ostr;
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// matrix operators
// ----------------------------------------------------------------------------

/**
 * unary +
 */
template<class t_mat>
const t_mat& operator+(const t_mat& mat1)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	return mat1;
}


/**
 * unary -
 */
template<class t_mat>
t_mat operator-(const t_mat& mat1)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	t_mat mat(mat1.size1(), mat1.size2());

	for(std::size_t i=0; i<mat1.size1(); ++i)
		for(std::size_t j=0; j<mat1.size2(); ++j)
			mat(i,j) = -mat1(i,j);

	return mat;
}


/**
 * binary +
 */
template<class t_mat>
t_mat operator+(const t_mat& mat1, const t_mat& mat2)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	if constexpr(m::is_dyn_mat<t_mat>)
		assert((mat1.size1() == mat2.size1() && mat1.size2() == mat2.size2()));
	else
		static_assert(mat1.size1() == mat2.size1() && mat1.size2() == mat2.size2());

	t_mat mat(mat1.size1(), mat1.size2());

	for(std::size_t i=0; i<mat1.size1(); ++i)
		for(std::size_t j=0; j<mat1.size2(); ++j)
			mat(i,j) = mat1(i,j) + mat2(i,j);

	return mat;
}


/**
 * binary -
 */
template<class t_mat>
t_mat operator-(const t_mat& mat1, const t_mat& mat2)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	return mat1 + (-mat2);
}


/**
 * matrix * scalar
 */
template<class t_mat>
t_mat operator*(const t_mat& mat1, typename t_mat::value_type d)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	t_mat mat(mat1.size1(), mat1.size2());

	for(std::size_t i=0; i<mat1.size1(); ++i)
		for(std::size_t j=0; j<mat1.size2(); ++j)
			mat(i,j) = mat1(i,j) * d;

	return mat;
}

/**
 * scalar * matrix
 */
template<class t_mat>
t_mat operator*(typename t_mat::value_type d, const t_mat& mat)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	return mat * d;
}

/**
 * matrix / scalar
 */
template<class t_mat>
t_mat operator/(const t_mat& mat, typename t_mat::value_type d)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	using T = typename t_mat::value_type;
	return mat * (T(1)/d);
}


/**
 * matrix-matrix product
 */
template<class t_mat>
t_mat operator*(const t_mat& mat1, const t_mat& mat2)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	if constexpr(m::is_dyn_mat<t_mat>)
		assert((mat1.size2() == mat2.size1()));
	else
		static_assert(mat1.size2() == mat2.size1());

	t_mat matRet(mat1.size1(), mat2.size2());

	for(std::size_t row=0; row<matRet.size1(); ++row)
	{
		for(std::size_t col=0; col<matRet.size2(); ++col)
		{
			matRet(row, col) = 0;
			for(std::size_t i=0; i<mat1.size2(); ++i)
				matRet(row, col) += mat1(row, i) * mat2(i, col);
		}
	}

	return matRet;
}


/**
 * matrix *= scalar
 */
template<class t_mat>
t_mat& operator*=(t_mat& mat1, typename t_mat::value_type d)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	mat1 = mat1 * d;
	return mat1;
}

/**
 * matrix /= scalar
 */
template<class t_mat>
t_mat& operator/=(t_mat& mat1, typename t_mat::value_type d)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	mat1 = mat1 / d;
	return mat1;
}


/**
 * operator <<
 */
template<class t_mat>
std::ostream& operator<<(std::ostream& ostr, const t_mat& mat)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	const std::size_t ROWS = mat.size1();
	const std::size_t COLS = mat.size2();

	for(std::size_t row=0; row<ROWS; ++row)
	{
		for(std::size_t col=0; col<COLS; ++col)
		{
			ostr << mat(row, col);
			if(col < COLS-1)
				ostr << ", ";
		}

		if(row < ROWS-1)
			ostr << "; ";
	}

	return ostr;
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// mixed operators
// ----------------------------------------------------------------------------

/**
 * matrix-vector product
 */
template<class t_mat, class t_vec>
t_vec operator*(const t_mat& mat, const t_vec& vec)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
	&& m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	if constexpr(m::is_dyn_mat<t_mat>)
		assert((mat.size2() == vec.size()));
	else
		static_assert(mat.size2() == vec.size());


	t_vec vecRet(mat.size1());

	for(std::size_t row=0; row<mat.size1(); ++row)
	{
		vecRet[row] = 0;
		for(std::size_t col=0; col<mat.size2(); ++col)
			vecRet[row] += mat(row, col) * vec[col];
	}

	return vecRet;
}
// ----------------------------------------------------------------------------

}
#endif
