/**
 * container-agnostic math algorithms
 * @author Tobias Weber
 * @date 9-dec-17
 * @license: see 'LICENSE' file
 */

#ifndef __MATH_ALGOS_H__
#define __MATH_ALGOS_H__

#include <cstddef>
#include <cmath>


// ----------------------------------------------------------------------------
// concepts
// ----------------------------------------------------------------------------
/**
 * requirements for a basic vector container like std::vector
 */
template<class T>
concept bool is_basic_vec = requires(const T& a)
{
	typename T::value_type;		// must have a value_type

	a.size();					// must have a size() member function
	a.operator[](1);			// must have an operator[]
};

/**
 * requirements of a vector type with a dynamic size
 */
template<class T>
concept bool is_dyn_vec = requires(const T& a)
{
	T(3);						// constructor
};

/**
 * requirements for a vector container
 */
template<class T>
concept bool is_vec = requires(const T& a)
{
	a+a;						// operator+
	a-a;						// operator-
	a[0]*a;						// operator*
	a*a[0];
	a/a[0];						// operator/
} && is_basic_vec<T>;


/**
 * requirements of a matrix type with a dynamic size
 */
template<class T>
concept bool is_dyn_mat = requires(const T& a)
{
	T(3,3);						// constructor
};

/**
 * requirements for a matrix container
 */
template<class T>
concept bool is_mat = requires(const T& a)
{
	typename T::value_type;		// must have a value_type

	a.size1();					// must have a size1() member function
	a.size2();					// must have a size2() member function
	a.operator()(1,1);			// must have an operator()

	a+a;						// operator+
	a-a;						// operator-
	a(0,0)*a;					// operator*
	a*a(0,0);
	a/a(0,0);					// operator/
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

	size_t size() const { return N; }

	T& operator[](size_t i) { return base_type::operator()(i,0); }
	const T& operator[](size_t i) const { return base_type::operator()(i,0); }
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

	size_t size1() const { return COLS; }
	size_t size2() const { return ROWS; }
};
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// n-dim algos
// ----------------------------------------------------------------------------
/**
 * unit matrix
 */
template<class t_mat>
t_mat unity(std::size_t N)
requires is_mat<t_mat>
{
	t_mat mat;
	if constexpr(is_dyn_mat<t_mat>)
		mat = t_mat(N,N);

	for(std::size_t i=0; i<N; ++i)
		for(std::size_t j=0; j<N; ++j)
			mat(i,j) = (i==j ? 1 : 0);

	return mat;
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

	for(std::size_t i=0; i<N1; ++i)
		for(std::size_t j=0; j<N2; ++j)
			mat(i,j) = 0;

	return mat;
}


/**
 * zero vector
 */
template<class t_vec>
t_vec zero(std::size_t N)
requires is_basic_vec<t_vec>
{
	t_vec vec;
	if constexpr(is_dyn_vec<t_vec>)
		vec = t_vec(N);

	for(std::size_t i=0; i<N; ++i)
		vec[i] = 0;

	return vec;
}



/**
 * inner product
 */
template<class t_vec>
typename t_vec::value_type inner_prod(const t_vec& vec1, const t_vec& vec2)
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
	return std::sqrt(inner_prod<t_vec>(vec, vec));
}


/**
 * outer product
 */
template<class t_mat, class t_vec>
t_mat outer_prod(const t_vec& vec1, const t_vec& vec2)
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
 */
template<class t_mat, class t_vec>
t_mat projector(const t_vec& vec, bool bIsNormalised=1)
requires is_vec<t_vec> && is_mat<t_mat>
{
	if(bIsNormalised)
	{
		return outer_prod<t_mat, t_vec>(vec, vec);
	}
	else
	{
		const auto len = norm<t_vec>(vec);
		t_vec _vec = vec/len;
		return outer_prod<t_mat, t_vec>(_vec, _vec);
	}
}


/**
 * project vector vec onto another vector vecProj
 */
template<class t_vec>
t_vec project(const t_vec& vec, const t_vec& vecProj, bool bIsNormalised=1)
requires is_vec<t_vec>
{
	if(bIsNormalised)
	{
		return inner_prod<t_vec>(vec, vecProj) * vecProj;
	}
	else
	{
		const auto len = norm<t_vec>(vecProj);
		t_vec _vecProj = vecProj / len;
		return inner_prod<t_vec>(vec, _vecProj) * _vecProj;
	}
}


/**
 * matrix to project onto plane perpendicular to vector: P = 1-|v><v|
 * from: 1 = sum_i |v_i><v_i| = |x><x| + |y><y| + |z><z|
 */
template<class t_mat, class t_vec>
t_mat ortho_projector(const t_vec& vec, bool bIsNormalised=1)
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
t_mat ortho_mirror_op(const t_vec& vec, bool bIsNormalised=1)
requires is_vec<t_vec> && is_mat<t_mat>
{
	using T = typename t_vec::value_type;
	const std::size_t iSize = vec.size();

	return unity<t_mat>(iSize) -
		T(2)*projector<t_mat, t_vec>(vec, bIsNormalised);
}


/**
 * project vector vec onto plane through the origin and perpendicular to vector vecNorm
 */
template<class t_vec>
t_vec ortho_project(const t_vec& vec, const t_vec& vecNorm, bool bIsNormalised=1)
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


// ----------------------------------------------------------------------------





// ----------------------------------------------------------------------------
// 3-dim algos
// ----------------------------------------------------------------------------

/**
 * cross product matrix
 */
template<class t_mat, class t_vec>
t_mat skewsymmetric(const t_vec& vec)
requires is_basic_vec<t_vec> && is_mat<t_mat>
{
	t_mat mat;
	if constexpr(is_dyn_mat<t_mat>)
		mat = t_mat(3,3);

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
	// project along rotation axis
	t_mat matProj1 = projector<t_mat, t_vec>(axis, bIsNormalised);

	// project along axis 2 in plane perpendiculat to rotation axis
	t_mat matProj2 = ortho_projector<t_mat, t_vec>(axis, bIsNormalised) * std::cos(angle);

	// project along axis 3 in plane perpendiculat to rotation axis and axis 2
	typename t_vec::value_type len = 1;
	if(!bIsNormalised)
		len = norm<t_vec>(axis);
	t_mat matProj3 = skewsymmetric<t_mat, t_vec>(axis/len) * std::sin(angle);

	return matProj1 + matProj2 + matProj3;
}

// ----------------------------------------------------------------------------



#endif
