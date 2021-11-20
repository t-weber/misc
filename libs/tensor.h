/**
 * tensor class
 * @author Tobias Weber (ident: 486fbae61c79af61ae9217361601098d7dd367380692f116995b0b81ab3b3407)
 * @date November 2021
 * @license: see 'LICENSE.EUPL' file
 */

#ifndef __TENSOR_H__
#define __TENSOR_H__

#include <array>
#include <tuple>
#include <type_traits>
#include <iostream>


// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------
/**
 * multiply all function arguments
 */
template<class t_ret, class... t_args>
constexpr t_ret mult_args(const t_args&&... args)
{
	return ( args * ... );
}


/**
 * get the first function argument
 */
template<std::size_t i, class t_arg1>
constexpr auto&& get_arg_i(t_arg1&& arg)
{
	return arg;
}


/**
 * get the i-th function argument
 */
template<std::size_t i, class t_arg1, class... t_args>
constexpr auto&& get_arg_i(t_arg1&& arg, t_args&&... args)
{
	if constexpr(i==0)
		return arg;
	return get_arg_i<i-1>(args...);
}


/**
 * get the i-th function argument
 */
template<std::size_t i, class... t_args>
constexpr auto&& get_arg_i(t_args&&... args)
{
	return get_arg_i<i-1>(args...);
}


/**
 * set all elements of the container to zero
 */
template<class t_cont>
void set_zero(t_cont& cont)
{
	using t_size = decltype(cont.size());
	using t_elem = std::decay_t<decltype(cont[0])>;

	for(t_size i=0; i<cont.size(); ++i)
		cont[i] = t_elem{};
}


/**
 * pick the elements with the given indices from a tuple
 */
template<std::size_t seq_offs, std::size_t ...seq>
constexpr auto pick_from_tuple(const auto& tup, const std::index_sequence<seq...>&)
{
	return std::make_tuple(std::get<seq + seq_offs>(tup)...);
}


/**
 * get an index to a multi-dimensional array with the given sizes
 */
template<class t_tup_dims, class t_tup_sizes>
constexpr std::size_t get_idx(const t_tup_dims& dims, const t_tup_sizes& sizes)
{
	static_assert(std::tuple_size<t_tup_dims>() == std::tuple_size<t_tup_sizes>(),
		"Wrong number of dimensions.");

	constexpr const std::size_t N = std::tuple_size<t_tup_sizes>();
	if constexpr(N == 0)
		return 0;
	else if constexpr(N == 1)
		return std::get<0>(dims);
	//else if constexpr(N == 2)
	//	return std::get<0>(dims)*std::get<1>(sizes) + std::get<1>(dims);
	else if constexpr(N > 1)
	{
		// remove first element of the sizes tuple
		auto sizes_1 = pick_from_tuple<1>(sizes, std::make_index_sequence<N-1>());
		// add a "1" to the tuple
		auto sizes_2 = std::tuple_cat(sizes_1, std::make_tuple(1));

		auto sizes_without_first = pick_from_tuple<1>(sizes_2, std::make_index_sequence<N-1>());
		auto dims_without_first = pick_from_tuple<1>(dims, std::make_index_sequence<N-1>());

		std::size_t idx =
			std::get<0>(dims) * std::get<0>(sizes_2) +
			get_idx(dims_without_first, sizes_without_first);

		return idx;
	}

	return 0;
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
/**
 * tensor with static size
 */
template<class t_scalar, std::size_t ...SIZES>
class Tensor
{
public:
	using t_size = std::size_t;
	using t_cont = std::array<t_scalar, mult_args<t_size>(SIZES...)>;


public:
	constexpr Tensor(bool zero = true) noexcept
	{
		if(zero)
			set_zero<t_cont>(m_elems);
	}


	~Tensor() = default;


	/**
	 * copy constructor
	 */
	constexpr Tensor(const Tensor<t_scalar, SIZES...>& other) noexcept
	{
		operator=(other);
	}


	/**
	 * move constructor
	 */
	constexpr Tensor(Tensor<t_scalar, SIZES...>&& other) noexcept
	{
		operator=(std::forward<Tensor<t_scalar, SIZES...>&&>(other));
	}


	/**
	 * assignment
	 */
	constexpr const Tensor& operator=(const Tensor<t_scalar, SIZES...>& other) noexcept
	{
		m_elems = other.m_elems;
		return *this;
	}


	/**
	 * movement
	 */
	constexpr const Tensor& operator=(Tensor<t_scalar, SIZES...>&& other) noexcept
	{
		m_elems = std::forward<t_cont&&>(other.m_elems);
		return *this;
	}


	/**
	 * get the total number of elements
	 */
	constexpr t_size size() const noexcept
	{
		return m_elems.size();
	}


	/**
	 * get the number of elements at a given position
	 */
	template<t_size i>
	constexpr t_size size() const
	{
		return get_arg_i<i>(SIZES...);
	}


	/**
	 * linear element access
	 */
	constexpr t_scalar& operator[](t_size i)
	{
		return m_elems[i];
	}


	/**
	 * linear element access
	 */
	constexpr const t_scalar& operator[](t_size i) const
	{
		return m_elems[i];
	}


	/**
	 * element access
	 */
	template<class ...t_dims>
	constexpr t_scalar& operator()(const t_dims&&... dims)
	{
		return operator[](get_idx(
			std::forward_as_tuple(dims...),
			std::forward_as_tuple(SIZES...)));
	}


	/**
	 * element access
	 */
	template<class ...t_dims>
	constexpr const t_scalar& operator()(const t_dims&&... dims) const
	{
		return operator[](get_idx(
			std::forward_as_tuple(dims...),
			std::forward_as_tuple(SIZES...)));
	}



	// ------------------------------------------------------------------------
	// operators

	/**
	 * unary +
	 */
	friend constexpr const Tensor<t_scalar, SIZES...>& operator+(
		const Tensor<t_scalar, SIZES...>& t) noexcept
	{
		return t;
	}


	/**
	 * unary -
	 */
	friend constexpr Tensor<t_scalar, SIZES...> operator-(
		const Tensor<t_scalar, SIZES...>& t) noexcept
	{
		using t_size = decltype(t.size());
		Tensor<t_scalar, SIZES...> t2;

		for(t_size i=0; i<t.size(); ++i)
			t2[i] = -t[i];

		return t2;
	}


	/**
	 * binary +
	 */
	friend constexpr Tensor<t_scalar, SIZES...> operator+(
		const Tensor<t_scalar, SIZES...>& t1, const Tensor<t_scalar, SIZES...>& t2) noexcept
	{
		using t_size = decltype(t1.size());
		Tensor<t_scalar, SIZES...> tret;

		for(t_size i=0; i<t1.size(); ++i)
			tret[i] = t1[i] + t2[i];

		return tret;
	}


	/**
	 * binary -
	 */
	friend constexpr Tensor<t_scalar, SIZES...> operator-(
		const Tensor<t_scalar, SIZES...>& t1, const Tensor<t_scalar, SIZES...>& t2) noexcept
	{
		using t_size = decltype(t1.size());
		Tensor<t_scalar, SIZES...> tret;

		for(t_size i=0; i<t1.size(); ++i)
			tret[i] = t1[i] - t2[i];

		return tret;
	}


	/**
	 * scalar multiplication
	 */
	friend constexpr Tensor<t_scalar, SIZES...> operator*(
		const Tensor<t_scalar, SIZES...>& t1, const t_scalar& s) noexcept
	{
		using t_size = decltype(t1.size());
		Tensor<t_scalar, SIZES...> tret;

		for(t_size i=0; i<t1.size(); ++i)
			tret[i] = t1[i] * s;

		return tret;
	}


	/**
	 * scalar multiplication
	 */
	friend constexpr Tensor<t_scalar, SIZES...> operator*(
		const t_scalar& s, const Tensor<t_scalar, SIZES...>& t1) noexcept
	{
		return operator*(t1, s);
	}


	/**
	 * scalar division
	 */
	friend constexpr Tensor<t_scalar, SIZES...> operator/(
		const Tensor<t_scalar, SIZES...>& t1, const t_scalar& s) noexcept
	{
		return operator*(t1, t_scalar(1)/s);
	}


	/**
	 * addition
	 */
	constexpr Tensor<t_scalar, SIZES...>& operator+=(const Tensor<t_scalar, SIZES...>& t) noexcept
	{
		for(t_size i=0; i<size(); ++i)
			operator[](i) += t[i];

		return *this;
	}


	/**
	 * subtraction
	 */
	constexpr Tensor<t_scalar, SIZES...>& operator-=(const Tensor<t_scalar, SIZES...>& t) noexcept
	{
		for(t_size i=0; i<size(); ++i)
			operator[](i) -= t[i];

		return *this;
	}


	/**
	 * scalar multiplication
	 */
	constexpr Tensor<t_scalar, SIZES...>& operator*=(const t_scalar& s) noexcept
	{
		for(t_size i=0; i<size(); ++i)
			operator[](i) *= s;

		return *this;
	}


	/**
	 * scalar division
	 */
	constexpr Tensor<t_scalar, SIZES...>& operator/=(const t_scalar& s) noexcept
	{
		return operator*=(t_scalar(1)/s);
	}
	// ------------------------------------------------------------------------


private:
	// tensor elements
	t_cont m_elems{};
};
// ----------------------------------------------------------------------------


#endif
