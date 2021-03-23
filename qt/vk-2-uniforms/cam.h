/**
 * camera
 * @author Tobias Weber
 * @date mar-2021
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __CAM_H__
#define __CAM_H__

#include "../../libs/math_algos.h"


template<class t_mat, class t_vec, class t_real = typename t_mat::value_type>
requires m::is_mat<t_mat> && m::is_vec<t_vec>
class Camera
{
public:
	using value_type = t_real;
	using size_type = decltype(t_mat{}.size1());

	Camera() = default;
	~Camera() = default;


	const t_mat& GetMatrix() const
	{
		return m_mat;
	}


	const t_mat& GetMatrixInv() const
	{
		return m_mat_inv;
	}


	void SetTranslation(t_real x, t_real y, t_real z)
	{
		m_matTrans(0,3) = x;
		m_matTrans(1,3) = y;
		m_matTrans(2,3) = z;
	}


	void SetRotation(t_real x, t_real y, t_real z)
	{
		m_matRot = m::rotation<t_mat, t_vec>(m::create<t_vec>({1.,0.,0.,0.}), x, 0);
		m_matRot *= m::rotation<t_mat, t_vec>(m::create<t_vec>({0.,1.,0.,0.}), y, 0);
		m_matRot *= m::rotation<t_mat, t_vec>(m::create<t_vec>({0.,0.,1.,0.}), z, 0);
	}


	void Translate(size_type axisidx, t_real delta)
	{
		//m_matTrans(axisidx, 3) += delta;
		const t_vec& axis = m::row<t_mat, t_vec>(m_mat, axisidx);

		for(size_type i=0; i<3; ++i)
			m_matTrans(i, 3) += axis[i] * delta;
	}


	void Rotate(size_type axisidx, t_real delta)
	{
		t_vec axis = m::row<t_mat, t_vec>(m_mat, axisidx);

		t_mat rot = m::rotation<t_mat, t_vec>(axis, delta, 0);
		m_matRot *= rot;
	}


	void Update()
	{
		m_matRot = m::orthonorm<t_mat, t_vec>(m_matRot);
		m_mat = m_matRot * m_matTrans;

		std::tie(m_mat_inv, std::ignore) = m::inv<t_mat, t_vec>(m_mat);
	}


private:
	t_mat m_mat = m::unit<t_mat>(4);
	t_mat m_mat_inv = m::unit<t_mat>(4);

	t_mat m_matTrans = m::unit<t_mat>(4);
	t_mat m_matRot = m::unit<t_mat>(4);
};

#endif
