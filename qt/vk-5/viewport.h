/**
 * viewport
 * @author Tobias Weber
 * @date mar-2021
 * @license see 'LICENSE.GPL' file
 */

#ifndef __VIEWPORT_H__
#define __VIEWPORT_H__

#include "../../libs/math_algos.h"


template<class t_mat, class t_vec, class t_real = typename t_mat::value_type>
requires m::is_mat<t_mat> && m::is_vec<t_vec>
class Viewport
{
public:
	using value_type = t_real;
	using size_type = decltype(t_mat{}.size1());

	Viewport() = default;
	~Viewport() = default;


	const t_mat& GetMatrix() const
	{
		return m_mat;
	}


	const t_mat& GetMatrixInv() const
	{
		return m_mat_inv;
	}


	void SetScreenSize(std::uint32_t w, std::uint32_t h)
	{
		m_screen[0] = w;
		m_screen[1] = h;
	}


	std::uint32_t GetScreenWidth() const
	{
		return m_screen[0];
	}


	std::uint32_t GetScreenHeight() const
	{
		return m_screen[1];
	}


	void Update()
	{
		m_mat = m::hom_viewport<t_mat>(m_screen[0], m_screen[1], 0., 1.);
		std::tie(m_mat_inv, std::ignore) = m::inv<t_mat, t_vec>(m_mat);
	}


private:
	t_mat m_mat = m::unit<t_mat>(4);
	t_mat m_mat_inv = m::unit<t_mat>(4);

	std::uint32_t m_screen[2] = { 800, 600 };
};

#endif
