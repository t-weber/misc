/**
 * camera
 * @author Tobias Weber
 * @date mar-2021
 * @license see 'LICENSE.GPL' file
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
		return m_trafo;
	}


	const t_mat& GetMatrixInv() const
	{
		return m_trafo_inv;
	}


	const t_mat& GetPerspectiveMatrix() const
	{
		return m_persp;
	}


	const t_mat& GetPerspectiveMatrixInv() const
	{
		return m_persp_inv;
	}


	bool GetUsePerspectiveProj() const
	{
		return m_use_prespective_proj;
	}


	void SetUsePerspectiveProj(bool b)
	{
		m_use_prespective_proj = b;
	}


	t_real GetFOV() const
	{
		return m_fov;
	}


	void SetFOV(t_real fov)
	{
		m_fov = fov;
	}


	void SetTranslation(t_real x, t_real y, t_real z)
	{
		m_trafoTrans(0,3) = x;
		m_trafoTrans(1,3) = y;
		m_trafoTrans(2,3) = z;
	}


	void SetRotation(t_real x, t_real y, t_real z)
	{
		m_trafoRot = m::rotation<t_mat, t_vec>(m::create<t_vec>({1.,0.,0.,0.}), x, 0);
		m_trafoRot *= m::rotation<t_mat, t_vec>(m::create<t_vec>({0.,1.,0.,0.}), y, 0);
		m_trafoRot *= m::rotation<t_mat, t_vec>(m::create<t_vec>({0.,0.,1.,0.}), z, 0);
	}


	void Translate(size_type axisidx, t_real delta)
	{
		//m_trafoTrans(axisidx, 3) += delta;
		const t_vec& axis = m::row<t_mat, t_vec>(m_trafo, axisidx);

		for(size_type i=0; i<3; ++i)
			m_trafoTrans(i, 3) += axis[i] * delta;
	}


	void Rotate(size_type axisidx, t_real delta)
	{
		t_vec axis = m::row<t_mat, t_vec>(m_trafo, axisidx);
		axis[3] = 0;

		t_mat rot = m::rotation<t_mat, t_vec>(axis, delta, 0);
		m_trafoRot *= rot;
	}


	void Update()
	{
		m_trafoRot = m::orthonorm<t_mat, t_vec>(m_trafoRot);
		m_trafo = m_trafoRot * m_trafoTrans;

		std::tie(m_trafo_inv, std::ignore) = m::inv<t_mat, t_vec>(m_trafo);
	}


	void UpdatePerspective(t_real screen_ratio = 3./4.)
	{
		// perspective projection
		if(m_use_prespective_proj)
		{
			m_persp = m::hom_perspective<t_mat>(
				0.01, 100., m_fov, screen_ratio,
				false, m_z01, m_inv_y);
		}

		// orthogonal projection
		else
		{
			m_persp = m::hom_parallel_sym<t_mat>(
				0.01, 100., 8., 8.,
				false, m_z01, m_inv_y);
		}

		std::tie(m_persp_inv, std::ignore) = m::inv<t_mat, t_vec>(m_persp);

		//using namespace m_ops;
		//std::cout << "projection matrix: " << m_persp << "." << std::endl;
		//std::cout << "inverted projection matrix: " << m_persp_inv << "." << std::endl;
	}


	t_vec GetPosition() const
	{
		return m::create<t_vec>({ -m_trafoTrans(0,3), -m_trafoTrans(1,3), -m_trafoTrans(2,3) });
	}


	t_vec ToScreenCoords(const t_vec& vec4, const t_mat& viewport, bool *visible = nullptr) const
	{
		auto [ persp, vec ] = m::hom_to_screen_coords<t_mat, t_vec>(
			vec4, GetMatrix(), GetPerspectiveMatrix(), viewport, m_inv_y);

		// position not visible -> return a point outside the viewport
		if(persp[2] > t_real(1))
		{
			if(visible) *visible = false;

			vec[0] = t_real(-2.) * viewport(0,0);
			vec[1] = t_real(-2.) * viewport(1,1);
			return vec;
		}

		if(visible) *visible = true;
		return vec;
	}


	bool InFrustum(const t_vec& vec) const
	{
		t_vec vec_cam = m_trafo * vec;
		t_vec vec_persp = m_persp * vec_cam;
		if(vec_persp.size() > 3)
			vec_persp /= vec_persp[3];

		const t_real ranges[3][2]
		{
			{ t_real(-1), t_real(+1) },	// x range
			{ t_real(-1), t_real(+1) },	// y range
			{ m_z01 ? t_real(0) : t_real(-1) , t_real(+1) },	// z range
		};

		// test ranges
		for(int i=0; i<3; ++i)
		{
			bool inside = (vec_persp[0]>=ranges[i][0] && vec_persp[0]<=ranges[i][1]);
			if(!inside)
				return false;
		}

		return true;
	}


private:
	t_mat m_trafo = m::unit<t_mat>(4);
	t_mat m_trafo_inv = m::unit<t_mat>(4);

	t_mat m_trafoTrans = m::unit<t_mat>(4);
	t_mat m_trafoRot = m::unit<t_mat>(4);

	t_mat m_persp{m::unit<t_mat>(4)};
	t_mat m_persp_inv{m::unit<t_mat>(4)};

	bool m_use_prespective_proj{true};
	bool m_z01{true};	// false for gl, true for vk
	bool m_inv_y{true};	// false for gl, true for vk

	t_real m_fov{m::pi<t_real> * t_real(0.5)};
};

#endif
