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
 *
 * kinetic energy:     T = 0.5*m*v^2 = 0.5*m*l^2*(dphi/dt)^2
 * potential energy:   U = m*g*z = m*g*l*(1-cos phi)
 * Lagrangian:         L = T - U
 *
 * equation of motion: d/dt[dL/d[dphi/dt]] = dL/dphi
 *   ( @see https://en.wikipedia.org/wiki/Lagrangian_mechanics )
 *                     d/dt[m*l^2*dphi/dt] = -m*g*l*sin phi
 *                     d^2/dt^2 phi = -g/l * sin phi
 *                     d^2/dt^2 phi = -g/l * phi for small angles
 * closed solution:    phi(t) = amp * sin(sqrt(g/l) * t + phase)
 *
 * to ODE system:      phi'' + g/l*phi = 0
 * subst:              g_0 := phi,  g_1 := phi';
 * system:             ( i) g_0' [= phi'] = g_1
 *                     (ii) g_1' [= phi'' = -g/l*phi] = -g/l*g_0
 */
template<class t_vec, class t_real = typename t_vec::value_type>
requires m::is_vec<t_vec>
class Pendulum
{
public:
	Pendulum(t_real l=1.) : m_l{l}
	{
		m_cur_state[0] = m_init_phase;
		m_cur_state[1] = 0.;
	}


	~Pendulum() = default;


	void SetLength(t_real l) { m_l = l; }
	t_real GetLength() const { return m_l; }

	void SetGrav(t_real l) { m_g = l; }
	t_real GetGrav() const { return m_g; }

	void SetAmp(t_real a) { m_amp = a; }
	t_real GetAmp() const { return m_amp; }

	void SetInitialPhase(t_real ph) { m_init_phase = ph; }
	t_real GetInitialPhase() const { return m_init_phase; }


	/**
	 * get phase at absolute time t
	 */
	t_real GetPhiAbs(t_real t) const
	{
		return m_amp * std::sin(std::sqrt(m_g/m_l)*t + m_init_phase);
	}


	/**
	 * get cartesian position of pendulum
	 */
	t_vec GetPos(t_real phi) const
	{
		//t_real phi = GetPhiAbs(t, m_amp, m_init_phase);
		t_real x = m_l * std::sin(phi);
		t_real y = 0.;
		t_real z = m_l * (1 - std::cos(phi));

		return m::create<t_vec>({x, y, z});
	}


	/**
	 * euler step of the differential equation
	 */
	t_real StepPhiEuler(t_real dt)
	{
		auto g_diff_0 = [/*this*/]([[maybe_unused]] t_real t, t_real g_0, t_real g_1) -> t_real
		{
			// phi'
			return 0*g_0 + 1*g_1;
		};

		auto g_diff_1 = [this]([[maybe_unused]] t_real t, t_real g_0, t_real g_1) -> t_real
		{
			// phi''
			return -m_g/m_l*g_0 + 0*g_1;
		};

		m_cur_t += dt;
		m_cur_state[0] += dt*g_diff_0(m_cur_t, m_cur_state[0], m_cur_state[1]);
		m_cur_state[1] += dt*g_diff_1(m_cur_t, m_cur_state[0], m_cur_state[1]);

		return m_cur_state[0];
	}


private:
	t_real m_l{1};
	t_real m_g{9.81};

	t_real m_amp{m::pi<t_real>*0.5};
	t_real m_init_phase{m::pi<t_real>*0.5};

	t_real m_cur_t{0};
	t_real m_cur_state[2];	// phi and phi'
};


#endif
