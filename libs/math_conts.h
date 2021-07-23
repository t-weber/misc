/**
 * containers and operators for use with math algorithms
 * @author Tobias Weber
 * @date jan-18
 * @license: see 'LICENSE.EUPL' file
 *
 * @see general references for algorithms:
 *	- (Bronstein08): I. N. Bronstein et al., ISBN: 978-3-8171-2017-8 (2008) [in its html version "Desktop Bronstein"].
 */

#ifndef __MATH_CONTS_H__
#define __MATH_CONTS_H__

#include <boost/algorithm/string.hpp>
#include <cassert>
#include <vector>
#include <array>
#include <iostream>
#include <iomanip>
#include "math_concepts.h"


// separator tokens
#define COLSEP ';'
#define ROWSEP '|'


// math operators
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
	using t_size = decltype(t_vec{}.size());
	t_vec vec = m::create<t_vec>(vec1.size());

	for(t_size i=0; i<vec1.size(); ++i)
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
	using t_size = decltype(t_vec{}.size());

	if constexpr(m::is_dyn_vec<t_vec>)
		assert((vec1.size() == vec2.size()));
	else
		static_assert(vec1.size() == vec2.size());

	t_vec vec = m::create<t_vec>(vec1.size());

	for(t_size i=0; i<vec1.size(); ++i)
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
	using t_size = decltype(t_vec{}.size());
	t_vec vec = m::create<t_vec>(vec1.size());

	for(t_size i=0; i<vec1.size(); ++i)
		vec[i] = vec1[i] * d;

	return vec;
}


/**
 * vector * vector
 */
template<class t_vec>
typename t_vec::value_type operator*(const t_vec& vec1, const t_vec& vec2)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	return inner<t_vec>(vec1, vec2);
}


/**
 * scalar * vector
 */
template<class t_vec>
t_vec operator*(typename t_vec::value_type d, const t_vec& vec)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
	//&& !m::is_basic_mat<typename t_vec::value_type>	// hack!
{
	return vec * d;
}

/**
 * vector / scalar
 */
template<class t_vec>
t_vec operator/(const t_vec& vec1, typename t_vec::value_type d)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
/*
	// doesn't work for integer value types, because 1/d is always 0 for d>1
	using T = typename t_vec::value_type;
	return vec1 * (T(1)/d);
*/

	using t_size = decltype(t_vec{}.size());
	t_vec vec = m::create<t_vec>(vec1.size());

	for(t_size i=0; i<vec1.size(); ++i)
		vec[i] = vec1[i] / d;

	return vec;
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
	using t_size = decltype(t_vec{}.size());
	const t_size N = vec.size();

	for(t_size i=0; i<N; ++i)
	{
		ostr << vec[i];
		if(i < N-1)
			ostr << COLSEP << " ";
	}

	return ostr;
}


/**
 * operator >>
 */
template<class t_vec>
std::istream& operator>>(std::istream& istr, t_vec& vec)
requires m::is_basic_vec<t_vec> && m::is_dyn_vec<t_vec>
{
	vec.clear();

	std::string str;
	std::getline(istr, str);

	std::vector<std::string> vecstr;
	boost::split(vecstr, str, [](auto c)->bool { return c==COLSEP; }, boost::token_compress_on);

	for(auto& tok : vecstr)
	{
		boost::trim(tok);
		std::istringstream istr(tok);

		typename t_vec::value_type c{};
		istr >> c;
		vec.emplace_back(std::move(c));
	}

	return istr;
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
	using t_size = decltype(t_mat{}.size1());
	t_mat mat = m::create<t_mat>(mat1.size1(), mat1.size2());

	for(t_size i=0; i<mat1.size1(); ++i)
		for(t_size j=0; j<mat1.size2(); ++j)
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
	using t_size = decltype(t_mat{}.size1());

	if constexpr(m::is_dyn_mat<t_mat>)
		assert((mat1.size1() == mat2.size1() && mat1.size2() == mat2.size2()));
	else
		static_assert(mat1.size1() == mat2.size1() && mat1.size2() == mat2.size2());

	t_mat mat = m::create<t_mat>(mat1.size1(), mat1.size2());

	for(t_size i=0; i<mat1.size1(); ++i)
		for(t_size j=0; j<mat1.size2(); ++j)
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
	using t_size = decltype(t_mat{}.size1());
	t_mat mat = m::create<t_mat>(mat1.size1(), mat1.size2());

	for(t_size i=0; i<mat1.size1(); ++i)
		for(t_size j=0; j<mat1.size2(); ++j)
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
t_mat operator/(const t_mat& mat1, typename t_mat::value_type d)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
/*
	// doesn't work for integer value types, because 1/d is always 0 for d>1
	using T = typename t_mat::value_type;
	return mat1 * (T(1)/d);
*/

	using t_size = decltype(t_mat{}.size1());
	t_mat mat = m::create<t_mat>(mat1.size1(), mat1.size2());

	for(t_size i=0; i<mat1.size1(); ++i)
		for(t_size j=0; j<mat1.size2(); ++j)
			mat(i,j) = mat1(i,j) / d;

	return mat;
}


/**
 * matrix-matrix product
 */
template<class t_mat>
t_mat operator*(const t_mat& mat1, const t_mat& mat2)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	using t_size = decltype(t_mat{}.size1());

	if constexpr(m::is_dyn_mat<t_mat>)
		assert((mat1.size2() == mat2.size1()));
	else
		static_assert(mat1.size2() == mat2.size1());

	t_mat matRet = m::create<t_mat>(mat1.size1(), mat2.size2());

	for(t_size row=0; row<matRet.size1(); ++row)
	{
		for(t_size col=0; col<matRet.size2(); ++col)
		{
			matRet(row, col) = 0;
			for(t_size i=0; i<mat1.size2(); ++i)
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
requires m::is_basic_mat<t_mat> //&& m::is_dyn_mat<t_mat>
{
	using t_size = decltype(t_mat{}.size1());

	const t_size ROWS = mat.size1();
	const t_size COLS = mat.size2();

	for(t_size row=0; row<ROWS; ++row)
	{
		for(t_size col=0; col<COLS; ++col)
		{
			ostr << mat(row, col);
			if(col < COLS-1)
				ostr << COLSEP << " ";
		}

		if(row < ROWS-1)
			ostr << ROWSEP << " ";
	}

	return ostr;
}


/**
 * prints matrix in nicely formatted form
 */
template<class t_mat>
std::ostream& niceprint(std::ostream& ostr, const t_mat& mat)
requires m::is_basic_mat<t_mat> && m::is_dyn_mat<t_mat>
{
	using t_size = decltype(t_mat{}.size1());

	const t_size ROWS = mat.size1();
	const t_size COLS = mat.size2();

	for(t_size i=0; i<ROWS; ++i)
	{
		ostr << "(";
		for(t_size j=0; j<COLS; ++j)
			ostr << std::setw(ostr.precision()*1.5) << std::right << mat(i,j);
		ostr << ")";

		if(i < ROWS-1)
			ostr << "\n";
	}

	return ostr;
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// quaternion operators
// @see https://en.wikipedia.org/wiki/Quaternion
// @see https://www.boost.org/doc/libs/1_76_0/libs/math/doc/quaternion/TQE.pdf
// ----------------------------------------------------------------------------

/**
 * unary +
 */
template<class t_quat>
const t_quat& operator+(const t_quat& quat)
requires m::is_basic_quat<t_quat>
{
	return quat;
}


/**
 * unary -
 */
template<class t_quat>
t_quat operator-(const t_quat& quat)
requires m::is_basic_quat<t_quat>
{
	return t_quat
	{
		-quat.real(),
		-quat.imag1(),
		-quat.imag2(),
		-quat.imag3()
	};
}


/**
 * binary +
 * @see https://en.wikipedia.org/wiki/Quaternion#Scalar_and_vector_parts
 */
template<class t_quat>
t_quat operator+(const t_quat& quat1, const t_quat& quat2)
requires m::is_basic_quat<t_quat>
{
	return t_quat
	{
		quat1.real() + quat2.real(),
		quat1.imag1() + quat2.imag1(),
		quat1.imag2() + quat2.imag2(),
		quat1.imag3() + quat2.imag3(),
	};
}


/**
 * binary -
 */
template<class t_quat>
t_quat operator-(const t_quat& quat1, const t_quat& quat2)
requires m::is_basic_quat<t_quat>
{
	return t_quat
	{
		quat1.real() - quat2.real(),
		quat1.imag1() - quat2.imag1(),
		quat1.imag2() - quat2.imag2(),
		quat1.imag3() - quat2.imag3(),
	};
}


/**
 * quat * quat
 * @see https://en.wikipedia.org/wiki/Quaternion#Scalar_and_vector_parts
 */
template<class t_quat>
t_quat operator*(const t_quat& quat1, const t_quat& quat2)
requires m::is_basic_quat<t_quat>
{
	return m::mult<t_quat>(quat1, quat2);
}


/**
 * quat / quat
 * @see (Bronstein08), chapter 4, equation (4.168)
 */
template<class t_quat>
t_quat operator/(const t_quat& quat1, const t_quat& quat2)
requires m::is_basic_quat<t_quat>
{
	return m::div<t_quat>(quat1, quat2);
}


/**
 * quat * scalar
 */
template<class t_quat>
t_quat operator*(const t_quat& quat, typename t_quat::value_type scalar)
requires m::is_basic_quat<t_quat>
{
	return quat * t_quat{ scalar, 0, 0, 0 };
}


/**
 * scalar * quat
 */
template<class t_quat>
t_quat operator*(typename t_quat::value_type scalar, const t_quat& quat)
requires m::is_basic_quat<t_quat>
{
	return t_quat{ scalar, 0, 0, 0 } * quat;
}


/**
 * quat / scalar
 */
template<class t_quat>
t_quat operator/(const t_quat& quat, typename t_quat::value_type scalar)
requires m::is_basic_quat<t_quat>
{
	using T = typename t_quat::value_type;
	return quat * (T(1)/scalar);
}


/**
 * quat += quat
 */
template<class t_quat>
t_quat& operator+=(t_quat& quat1, const t_quat& quat2)
requires m::is_basic_quat<t_quat>
{
	quat1 = quat1 + quat2;
	return quat1;
}

/**
 * quat -= quat
 */
template<class t_quat>
t_quat& operator-=(t_quat& quat1, const t_quat& quat2)
requires m::is_basic_quat<t_quat>
{
	quat1 = quat1 - quat2;
	return quat1;
}


/**
 * quat *= scalar
 */
template<class t_quat>
t_quat& operator*=(t_quat& quat, typename t_quat::value_type scalar)
requires m::is_basic_quat<t_quat>
{
	quat = quat * scalar;
	return quat;
}

/**
 * quat /= scalar
 */
template<class t_quat>
t_quat& operator/=(t_quat& quat, typename t_quat::value_type scalar)
requires m::is_basic_quat<t_quat>
{
	quat = quat / scalar;
	return quat;
}


/**
 * operator <<
 */
template<class t_quat>
std::ostream& operator<<(std::ostream& ostr, const t_quat& quat)
requires m::is_basic_quat<t_quat>
{
	ostr << quat.real() << " + ";
	ostr << quat.imag1() << "i" << " + ";
	ostr << quat.imag2() << "j" << " + ";
	ostr << quat.imag3() << "k";

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
	using t_size = decltype(t_mat{}.size1());

	if constexpr(m::is_dyn_mat<t_mat>)
		assert((mat.size2() == t_size(vec.size())));
	else
		static_assert(mat.size2() == t_size(vec.size()));


	t_vec vecRet = m::create<t_vec>(mat.size1());

	for(t_size row=0; row<mat.size1(); ++row)
	{
		vecRet[row] = typename t_vec::value_type{/*0*/};
		for(t_size col=0; col<mat.size2(); ++col)
		{
			auto elem = mat(row, col) * vec[col];
			vecRet[row] += elem;
		}
	}

	return vecRet;
}
// ----------------------------------------------------------------------------

}


// maths
namespace m {

/**
 * ----------------------------------------------------------------------------
 * vector container
 * ----------------------------------------------------------------------------
 */
template<class T = double, template<class...> class t_cont = std::vector>
requires is_basic_vec<t_cont<T>> && is_dyn_vec<t_cont<T>>
class vec : public t_cont<T>
{
public:
	using value_type = T;
	using container_type = t_cont<T>;

	vec() = default;
	vec(std::size_t SIZE) : t_cont<T>(SIZE) {}
	~vec() = default;

	const value_type& operator()(std::size_t i) const { return this->operator[](i); }
	value_type& operator()(std::size_t i) { return this->operator[](i); }

	using t_cont<T>::operator[];

	friend vec operator+(const vec& vec1, const vec& vec2) { return m_ops::operator+(vec1, vec2); }
	friend vec operator-(const vec& vec1, const vec& vec2) { return m_ops::operator-(vec1, vec2); }
	friend const vec& operator+(const vec& vec1) { return vec1; }
	friend vec operator-(const vec& vec1) { return m_ops::operator-(vec1); }

	friend value_type operator*(const vec& vec1, const vec& vec2) { return m_ops::operator*<vec>(vec1, vec2); }
	friend vec operator*(value_type d, const vec& vec1) { return m_ops::operator*(d, vec1); }
	friend vec operator*(const vec& vec1, value_type d) { return m_ops::operator*(vec1, d); }
	friend vec operator/(const vec& vec1, value_type d) { return m_ops::operator/(vec1, d); }

	vec& operator*=(const vec& vec2) { return m_ops::operator*=(*this, vec2); }
	vec& operator+=(const vec& vec2) { return m_ops::operator+=(*this, vec2); }
	vec& operator-=(const vec& vec2) { return m_ops::operator-=(*this, vec2); }
	vec& operator*=(value_type d) { return m_ops::operator*=(*this, d); }
	vec& operator/=(value_type d) { return m_ops::operator/=(*this, d); }

private:
};


/**
 * ----------------------------------------------------------------------------
 * matrix container
 * ----------------------------------------------------------------------------
 */
template<class T = double, template<class...> class t_cont = std::vector>
requires is_basic_vec<t_cont<T>> && is_dyn_vec<t_cont<T>>
class mat
{
public:
	using value_type = T;
	using container_type = t_cont<T>;

	mat() = default;
	mat(std::size_t ROWS, std::size_t COLS) : m_data(ROWS*COLS), m_rowsize{ROWS}, m_colsize{COLS} {}
	~mat() = default;

	std::size_t size1() const { return m_rowsize; }
	std::size_t size2() const { return m_colsize; }
	const T& operator()(std::size_t row, std::size_t col) const { return m_data[row*m_colsize + col]; }
	T& operator()(std::size_t row, std::size_t col) { return m_data[row*m_colsize + col]; }

	friend mat operator+(const mat& mat1, const mat& mat2) { return m_ops::operator+(mat1, mat2); }
	friend mat operator-(const mat& mat1, const mat& mat2) { return m_ops::operator-(mat1, mat2); }
	friend const mat& operator+(const mat& mat1) { return mat1; }
	friend mat operator-(const mat& mat1) { return m_ops::operator-(mat1); }

	friend mat operator*(const mat& mat1, const mat& mat2) { return m_ops::operator*(mat1, mat2); }
	friend mat operator*(const mat& mat1, value_type d) { return m_ops::operator*(mat1, d); }
	friend mat operator*(value_type d, const mat& mat1) { return m_ops::operator*(d, mat1); }
	friend mat operator/(const mat& mat1, value_type d) { return m_ops::operator/(mat1, d); }

	template<class t_vec> requires is_basic_vec<t_cont<T>> && is_dyn_vec<t_cont<T>>
	friend t_vec operator*(const mat& mat1, const t_vec& vec2) { return m_ops::operator*(mat1, vec2); }

	mat& operator*=(const mat& mat2) { return m_ops::operator*=(*this, mat2); }
	mat& operator+=(const mat& mat2) { return m_ops::operator+=(*this, mat2); }
	mat& operator-=(const mat& mat2) { return m_ops::operator-=(*this, mat2); }
	mat& operator*=(value_type d) { return m_ops::operator*=(*this, d); }
	mat& operator/=(value_type d) { return m_ops::operator/=(*this, d); }

private:
	container_type m_data{};
	std::size_t m_rowsize{}, m_colsize{};
};


/**
 * ----------------------------------------------------------------------------
 * quaternion container
 * @see https://en.wikipedia.org/wiki/Quaternion
 * @see https://www.boost.org/doc/libs/1_76_0/libs/math/doc/quaternion/TQE.pdf
 * ----------------------------------------------------------------------------
 */
template<class T = double, template<class, std::size_t> class t_cont = std::array>
requires is_basic_vec<t_cont<T, 4>>
class quat
{
public:
	using value_type = T;
	using container_type = t_cont<T, 4>;

	quat(value_type r=0, value_type i1=0, value_type i2=0, value_type i3=0)
		: m_data{{r, i1, i2, i3}} {}
	~quat() = default;

	value_type real() const { return m_data[0]; }
	value_type imag1() const { return m_data[1]; }
	value_type imag2() const { return m_data[2]; }
	value_type imag3() const { return m_data[3]; }

	template<class t_vec> requires is_vec<t_vec>
	t_vec imag() const
	{
		return m::create<t_vec>({ imag1(), imag2(), imag3() });
	}

	void real(value_type val) { m_data[0] = val; }
	void imag1(value_type val) { m_data[1] = val; }
	void imag2(value_type val) { m_data[2] = val; }
	void imag3(value_type val) { m_data[3] = val; }

	template<class t_vec> requires is_vec<t_vec>
	void imag(const t_vec& vec)
	{
		imag1(vec[0]);
		imag2(vec[1]);
		imag3(vec[2]);
	}


	friend quat operator+(const quat& quat1, const quat& quat2) { return m_ops::operator+(quat1, quat2); }
	friend quat operator-(const quat& quat1, const quat& quat2) { return m_ops::operator-(quat1, quat2); }
	friend const quat& operator+(const quat& quat) { return quat; }
	friend quat operator-(const quat& quat) { return m_ops::operator-(quat); }

	friend quat operator*(const quat& quat1, const quat& quat2) { return m_ops::operator*<quat>(quat1, quat2); }
	friend quat operator/(const quat& quat1, const quat& quat2) { return m_ops::operator/<quat>(quat1, quat2); }

	friend quat operator*(value_type d, const quat& quat) { return m_ops::operator*(d, quat); }
	friend quat operator*(const quat& quat, value_type d) { return m_ops::operator*(quat, d); }
	friend quat operator/(const quat& quat, value_type d) { return m_ops::operator/(quat, d); }

	quat& operator*=(const quat& quat2) { return m_ops::operator*=(*this, quat2); }
	quat& operator+=(const quat& quat2) { return m_ops::operator+=(*this, quat2); }
	quat& operator-=(const quat& quat2) { return m_ops::operator-=(*this, quat2); }
	quat& operator*=(value_type d) { return m_ops::operator*=(*this, d); }
	quat& operator/=(value_type d) { return m_ops::operator/=(*this, d); }

private:
	container_type m_data{};
};
// ----------------------------------------------------------------------------

}


#endif
