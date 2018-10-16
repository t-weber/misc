/**
 * ODE test
 * @author Tobias Weber
 * @date oct-18
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_68_0/libs/numeric/odeint/doc/html/boost_numeric_odeint/tutorial.html
 *  * https://github.com/boostorg/odeint/tree/develop/examples
 *  * https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
 *  * https://en.wikipedia.org/wiki/Euler_method
 *
 * g++ -std=c++17 -o ode ode.cpp
 *
 * higher order ODE to system:
 *  a*f'' + b*f' + c*f = 0
 *  subst: g0 = f,  g1 = f';
 *  system: ( i) g0' [= f'] = g1
 *          (ii) g1' [= f'' = -b/a*f' - c/a*f] = -b/a*g1 - c/a*g0
 */

#include <iostream>

#include <boost/numeric/odeint.hpp>
namespace ode = boost::numeric::odeint;

using t_real = double;
using t_phase = std::vector<t_real>;

constexpr t_real g = -9.81;


template<class t_stepper>
void falling(t_stepper&& stepper, t_phase& qp, t_real t_start, t_real t_end, t_real t_step)
{
	std::cout << "initial: q=" << qp[0] << ", p=" << qp[1] << std::endl;

	ode::integrate_adaptive(stepper,
	[](const t_phase& qp, t_phase& qp_diff, t_real t) -> void	// ODE system
	{
		qp_diff[0] = qp[1];	// dq/dt = p;
		qp_diff[1] = g;		// dp/dt = d^2q/dt^2 = g;
	}, qp, t_start, t_end, t_step,
	[](const t_phase& qp, t_real t) -> void				// optional: step-wise output
	{
		std::cout << "step: t=" << t << ", q=" << qp[0] << ", p=" << qp[1] << std::endl;
	});

	std::cout << "final: q=" << qp[0] << ", p=" << qp[1] << std::endl;
}


int main()
{
	const t_real t_start = 0.;
	const t_real t_end = 5.;
	const t_real t_step = 0.01;
	const t_real eps_abs = 1e-8;
	const t_real eps_rel = 1e-6;

	t_phase qp = { 0. /*position q*/, 0. /*momentum p*/ };


	{
		std::cout << "\nRK4" << std::endl;
		qp[0] = qp[1] = 0;
		falling(ode::runge_kutta4<t_phase>(), qp, t_start, t_end, t_step);
	}

	{
		std::cout << "\nadaptive RK4 1" << std::endl;
		qp[0] = qp[1] = 0;
		falling(ode::make_controlled(eps_abs, eps_rel, ode::runge_kutta_cash_karp54<t_phase>()),
			qp, t_start, t_end, t_step);
	}

	{
		std::cout << "\nadaptive RK4 2" << std::endl;
		qp[0] = qp[1] = 0;
		falling(ode::make_controlled(eps_abs, eps_rel, ode::runge_kutta_fehlberg78<t_phase>()),
			qp, t_start, t_end, t_step);
	}

	{
		std::cout << "\nadaptive RK4 3" << std::endl;
		qp[0] = qp[1] = 0;
		falling(ode::make_controlled(eps_abs, eps_rel, ode::runge_kutta_dopri5<t_phase>()),
			qp, t_start, t_end, t_step);
	}


	{
		std::cout << "\nmanual RK4" << std::endl;
		qp[0] = qp[1] = 0;

		// dq/dt = g*t
		auto qdiff = [](t_real t, t_real q) -> t_real
		{
			return g*t;
		};

		auto k1234 = [&qdiff](t_real t_step, t_real t, t_real q)
			-> std::tuple<t_real, t_real, t_real, t_real>
		{
			t_real k1 = t_step * qdiff(t, q);
			t_real k2 = t_step * qdiff(t+t_step*0.5, q+k1*0.5);
			t_real k3 = t_step * qdiff(t+t_step*0.5, q+k2*0.5);
			t_real k4 = t_step * qdiff(t+t_step, q+k3);

			return std::make_tuple(k1, k2, k3, k4);
		};


		t_real q = qp[0];
		for(t_real t=t_start; t<t_end-t_step; t+=t_step)
		{
			auto [k1, k2, k3, k4] = k1234(t_step, t, q);
			q += (k1 + 2.*k2 + 2.*k3 + k4) / 6.;
		}

		std::cout << "final: q=" << q << std::endl;
	}

	{
		std::cout << "\nmanual Euler" << std::endl;
		qp[0] = qp[1] = 0;

		// dq/dt = g*t
		auto qdiff = [](t_real t, t_real q) -> t_real
		{
			return g*t;
		};

		t_real q = qp[0];
		for(t_real t=t_start; t<t_end-t_step; t+=t_step)
			q += t_step*qdiff(t, q);

		std::cout << "final: q=" << q << std::endl;
	}

	{
		std::cout << "\nanalytical" << std::endl;
		t_real p = g * (t_end-t_start);
		t_real q = g/2. * std::pow(t_end-t_start, 2.);
		std::cout << "final: q=" << q << ", p=" << p << std::endl;
	}

	return 0;
}
