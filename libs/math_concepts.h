/**
 * concepts and adapters for math classes
 * @author Tobias Weber
 * @date dec-17
 * @license: see 'LICENSE.EUPL' file
 */

#ifndef __MATH_CONCEPTS_H__
#define __MATH_CONCEPTS_H__

#include <cstddef>
#include <iterator>
#include <complex>
#include <concepts>
//#include <iostream>


namespace m {

// ----------------------------------------------------------------------------
// concepts
// ----------------------------------------------------------------------------
/**
 * requirements for a scalar type
 */
template<class T>
concept /*bool*/ is_scalar =
	std::is_floating_point_v<T> || std::is_integral_v<T> /*|| std::is_arithmetic_v<T>*/;


/**
 * requirements for a basic vector container like std::vector
 */
template<class T>
concept /*bool*/ is_basic_vec = requires(const T& a)
{
	typename T::value_type;		// must have a value_type

	a.size();			// must have a size() member function
	a.operator[](1);		// must have an operator[]
};


/**
 * requirements of a vector type with a dynamic size
 */
template<class T>
concept /*bool*/ is_dyn_vec = requires(const T& a)
{
	T(3); T{3};					// constructor with size
};


/**
 * requirements for a vector container
 */
template<class T>
concept /*bool*/ is_vec = requires(const T& a)
{
	a + a;						// operator+
	a - a;						// operator-
	a[0] * a;					// operator*
	a * a[0];
	a / a[0];					// operator/
} && is_basic_vec<T>;


/**
 * requirements for a basic quaternion container
 */
template<class T>
concept /*bool*/ is_basic_quat = requires(T a, typename T::value_type scalar)
{
	typename T::value_type;		// must have a value_type

	T(scalar, scalar, scalar, scalar);

	a.real();
	a.imag1();
	a.imag2();
	a.imag3();

	a.real(scalar);
	a.imag1(scalar);
	a.imag2(scalar);
	a.imag3(scalar);
};


/**
 * requirements for a quaternion container with operators
 */
template<class T>
concept /*bool*/ is_quat = requires(T a, typename T::value_type scalar)
{
	a + a;						// operator+
	a - a;						// operator-
	a * a;						// operator-
	scalar * a;					// operator*
	a * scalar;
	a / scalar;					// operator/
} && is_basic_quat<T>;


/**
 * requirements for a basic matrix container
 */
template<class T>
concept /*bool*/ is_basic_mat = requires(const T& a)
{
	typename T::value_type;		// must have a value_type

	a.size1();			// must have a size1() member function
	a.size2();			// must have a size2() member function
	a.operator()(1,1);		// must have an operator()
};


/**
 * requirements of a matrix type with a dynamic size
 */
template<class T>
concept /*bool*/ is_dyn_mat = requires(const T& a)
{
	T(3,3);	T{3,3};					// constructor with sizes
};


/**
 * requirements for a matrix container
 */
template<class T>
concept /*bool*/ is_mat = requires(const T& a)
{
	a + a;						// operator+
	a - a;						// operator-
	a(0,0) * a;					// operator*
	a * a(0,0);
	a / a(0,0);					// operator/
} && is_basic_mat<T>;


/**
 * requirements for a complex number
 */
template<class T>
concept /*bool*/ is_complex = requires(const T& a)
{
	typename T::value_type;		// must have a value_type

	std::conj(a);
	a.real();			// must have a real() member function
	a.imag();			// must have an imag() member function

	a + a;
	a - a;
	a * a;
	a / a;
};


/**
 * requirements for an iterable container
 */
template<class T>
concept /*bool*/ is_iterable = requires(const T& a)
{
	a.begin();
	a.end();

	{ a.begin() == a.end() } -> std::same_as<bool>;
	{ a.begin() != a.end() } -> std::same_as<bool>;
};
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// adapters
// ----------------------------------------------------------------------------
template<typename size_t, size_t N, typename T, template<size_t, size_t, class...> class t_mat_base>
class qvec_adapter : public t_mat_base<1, N, T>
{
public:
	// types
	using base_type = t_mat_base<1, N, T>;
	using size_type = size_t;
	using value_type = T;

	// constructors
	using base_type::base_type;
	qvec_adapter(const base_type& vec) : base_type{vec} {}
	qvec_adapter(size_t n=N) : base_type{}, m_n{n} {}

	// copying
	//qvec_adapter(const qvec_adapter<size_t, N, T, t_mat_base>& other)
	//	: t_mat_base<1, N, T>(other), m_n{other.m_n} {}

	constexpr size_t size() const { return m_n<N ? m_n : N; }

	T& operator[](size_t i) { return base_type::operator()(i,0); }
	const T operator[](size_t i) const { return base_type::operator()(i,0); }

private:
	size_t m_n{N};
};


template<typename size_t, size_t ROWS, size_t COLS, typename T, template<size_t, size_t, class...> class t_mat_base>
class qmat_adapter : public t_mat_base<COLS, ROWS, T>
{
public:
	// types
	using base_type = t_mat_base<COLS, ROWS, T>;
	using size_type = size_t;
	using value_type = T;

	// constructors
	using base_type::base_type;
	qmat_adapter(const base_type& mat) : base_type{mat} {}
	qmat_adapter(size_t rows=ROWS, size_t cols=COLS) : base_type{}, m_rows{rows}, m_cols{cols} {}

	// copying
	//qmat_adapter(const qmat_adapter<size_t, ROWS, COLS, T, t_mat_base>& other)
	//	: t_mat_base<COLS, ROWS, T>(other), m_rows{other.m_rows}, m_cols{other.m_cols} {}

	constexpr size_t size1() const { return m_rows<ROWS ? m_rows : ROWS; }
	constexpr size_t size2() const { return m_cols<COLS ? m_cols : COLS; }

private:
	size_t m_rows{ROWS}, m_cols{COLS};
};


template<typename size_t, size_t N, typename T, class t_vec_base>
class qvecN_adapter : public t_vec_base
{
public:
	// types
	using base_type = t_vec_base;
	using size_type = size_t;
	using value_type = T;

	// constructors
	using base_type::base_type;
	qvecN_adapter(const base_type& vec) : base_type{vec} {}
	qvecN_adapter(size_t n=N) : base_type{}, m_n{n} {}

	// copying
	//qvecN_adapter(const qvecN_adapter<size_t, N, T, t_vec_base>& other)
	//	: t_vec_base(other), m_n{other.m_n} {}

	constexpr size_t size() const { return m_n<N ? m_n : N; }

	T& operator[](size_t i) { return static_cast<base_type&>(*this)[i]; }
	const T operator[](size_t i) const { return static_cast<const base_type&>(*this)[i]; }

private:
	size_t m_n{N};
};


template<typename size_t, size_t ROWS, size_t COLS, typename T, class t_mat_base>
class qmatNN_adapter : public t_mat_base
{
public:
	// types
	using base_type = t_mat_base;
	using size_type = size_t;
	using value_type = T;

	// constructors
	using base_type::base_type;
	qmatNN_adapter(const base_type& mat) : base_type{mat} {}
	qmatNN_adapter(size_t rows=ROWS, size_t cols=COLS) : base_type{}, m_rows{rows}, m_cols{cols} {}

	// copying
	//qmatNN_adapter(const qmatNN_adapter<size_t, ROWS, COLS, T, t_mat_base>& other)
	//	: t_mat_base(other), m_rows{other.m_rows}, m_cols{other.m_cols} {}

	constexpr size_t size1() const { return m_rows<ROWS ? m_rows : ROWS; }
	constexpr size_t size2() const { return m_cols<COLS ? m_cols : COLS; }

private:
	size_t m_rows{ROWS}, m_cols{COLS};
};
// ----------------------------------------------------------------------------

}
#endif
