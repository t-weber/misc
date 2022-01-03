/**
 * simple pendulum
 * @author Tobias Weber
 * @date jan-2022
 * @license see 'LICENSE.GPL' file
 */

#ifndef __PENDULUM_H__
#define __PENDULUM_H__

#include "../../libs/math_algos.h"


/**
 * simple pendulum of length l
 * kinetic energy:     T = 0.5*m*v^2 = 0.5*m*l^2*(dphi/dt)^2
 * potential energy:   U = m*g*z = m*g*l*(1-cos phi)
 * Lagrangian:         L = T - U
 * equation of motion: d/dt[dL/d[dphi/dt]] = dL/dphi
 *                     d/dt[m*l^2*dphi/dt] = -m*g*l*sin phi
 *                     d^2/dt^2 phi = -g/l * sin phi
 *                     d^2/dt^2 phi = -g/l * phi for small angles
 *                     phi(t) = amp * sin(sqrt(g/l) * t + phase)
 *
 * @see https://en.wikipedia.org/wiki/Lagrangian_mechanics
 */
template<class t_vec, class t_real = typename t_vec::value_type>
requires m::is_vec<t_vec>
class Pendulum
{
public:
	Pendulum(t_real l=1.) : m_l{l}
	{}


	~Pendulum() = default;


	void SetLength(t_real l) { m_l = l; }
	t_real GetLength() const { return m_l; }


	void SetGrav(t_real l) { m_g = l; }
	t_real GetGrav() const { return m_g; }


	t_real GetPhi(t_real t, t_real amp=1., t_real phase=0.) const
	{
		return amp * std::sin(std::sqrt(m_g/m_l)*t + phase);
	}


	t_vec GetPos(t_real t, t_real amp=1., t_real phase=0.) const
	{
		t_real phi = GetPhi(t, amp, phase);
		t_real x = m_l * std::sin(phi);
		t_real y = 0.;
		t_real z = m_l * (1 - std::cos(phi));

		return m::create<t_vec>({x, y, z});
	}


private:
	t_real m_l{1};
	t_real m_g{9.81};
};


#endif
