/**
 * ODE test with Riccati equation
 * @author Tobias Weber
 * @date nov-18
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_68_0/libs/numeric/odeint/doc/html/boost_numeric_odeint/tutorial.html
 *  * https://github.com/boostorg/odeint/tree/develop/examples
 *  * https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
 *  * https://en.wikipedia.org/wiki/Euler_method
 *
 * g++ -std=c++17 -o ode2 ode2.cpp
 *
 * higher order ODE to system:
 *  a*f'' + b*f' + c*f = 0
 *  subst: g0 = f,  g1 = f';
 *  system: ( i) g0' [= f'] = g1
 *          (ii) g1' [= f'' = -b/a*f' - c/a*f] = -b/a*g1 - c/a*g0
 *               a*g1' + b*g1 + c*g0 = 0
 */

#include <iostream>

#include <boost/numeric/odeint.hpp>
namespace ode = boost::numeric::odeint;

using t_real = double;
using t_y = std::vector<t_real>;

constexpr t_real g = -9.81;
constexpr t_real c = 1.23;


template<class t_stepper>
void riccati(t_stepper&& stepper, t_y& y, t_real x_start, t_real x_end, t_real x_step)
{
	std::cout << "initial: y=" << y[0] << ", dy/dx=" << y[1] << std::endl;

	ode::integrate_adaptive(stepper,
	[](const t_y& y, t_y& y_diff, t_real x) -> void	// ODE system
	{
		y_diff[0] = y[1];		// dy/dx = y1
		y_diff[1] = c*y[1]*y[1] + g;	// dy1/dx = d^2y/dx^2 = c*y'^2 + g
	}, y, x_start, x_end, x_step,
	[](const t_y& y, t_real x) -> void	// optional: step-wise output
	{
		std::cout << "step: x=" << x << ", y=" << y[0] << ", dy/dx=" << y[1] << std::endl;
	});

	std::cout << "final: y=" << y[0] << ", dy/dx=" << y[1] << std::endl;
}


void riccati_tst()
{
	const t_real x_start = 0.;
	const t_real x_end = 5.;
	const t_real x_step = 0.01;
	const t_real eps_abs = 1e-8;
	const t_real eps_rel = 1e-6;

	// initial values
	t_y _y = { 10. /*y*/, -0.1 /*y'*/ };


	{
		std::cout << "\nRK4" << std::endl;
		t_y y = _y;
		riccati(ode::runge_kutta4<t_y>(), y, x_start, x_end, x_step);
	}

	{
		std::cout << "\nadaptive RK4 1" << std::endl;
		t_y y = _y;
		riccati(ode::make_controlled(eps_abs, eps_rel, ode::runge_kutta_cash_karp54<t_y>()),
			y, x_start, x_end, x_step);
	}

	{
		std::cout << "\nadaptive RK4 2" << std::endl;
		t_y y = _y;
		riccati(ode::make_controlled(eps_abs, eps_rel, ode::runge_kutta_fehlberg78<t_y>()),
			y, x_start, x_end, x_step);
	}

	{
		std::cout << "\nadaptive RK4 3" << std::endl;
		t_y y = _y;
		riccati(ode::make_controlled(eps_abs, eps_rel, ode::runge_kutta_dopri5<t_y>()),
			y, x_start, x_end, x_step);
	}

	{
		std::cout << "\nmanual RK4" << std::endl;
		t_y y = _y;


		auto ydiff_0 = [](t_real x, t_real y_0, t_real y_1) -> t_real
		{	// dy/dx = y1
			return y_1;
		};

		auto ydiff_1 = [](t_real x, t_real y_0, t_real y_1) -> t_real
		{	// dy1/dx = d^2y/dx^2 = c*y'^2 + g
			return c*y_1*y_1 + g;
		};

		auto k1234 = [&ydiff_0, &ydiff_1](t_real x_step, t_real x, const t_y& y)
			-> std::tuple<t_real,t_real,t_real,t_real, t_real,t_real,t_real,t_real>
		{
			t_real k1[] =
			{
				x_step * ydiff_0(x, y[0], y[1]),
				x_step * ydiff_1(x, y[0], y[1])
			};

			t_real k2[] =
			{
				x_step * ydiff_0(x+x_step*0.5, y[0]+k1[0]*0.5, y[1]+k1[1]*0.5),
				x_step * ydiff_1(x+x_step*0.5, y[0]+k1[0]*0.5, y[1]+k1[1]*0.5)
			};

			t_real k3[] =
			{
				x_step * ydiff_0(x+x_step*0.5, y[0]+k2[0]*0.5, y[1]+k2[1]*0.5),
				x_step * ydiff_1(x+x_step*0.5, y[0]+k2[0]*0.5, y[1]+k2[1]*0.5)
			};

			t_real k4[] =
			{
				x_step * ydiff_0(x+x_step, y[0]+k3[0], y[1]+k3[1]),
				x_step * ydiff_1(x+x_step, y[0]+k3[0], y[1]+k3[1])
			};

			return std::make_tuple(k1[0],k2[0],k3[0],k4[0], k1[1],k2[1],k3[1],k4[1]);
		};


		for(t_real x=x_start; x<x_end-x_step; x+=x_step)
		{
			t_real k1[2], k2[2], k3[2], k4[2];
			std::tie(k1[0],k2[0],k3[0],k4[0], k1[1],k2[1],k3[1],k4[1]) =
				k1234(x_step, x, y);
			y[0] += (k1[0] + 2.*k2[0] + 2.*k3[0] + k4[0]) / 6.;
			y[1] += (k1[1] + 2.*k2[1] + 2.*k3[1] + k4[1]) / 6.;
		}

		std::cout << "final: y=" << y[0] << ", dy/dx=" << y[1] << std::endl;
	}


	{
		std::cout << "\nmanual Euler" << std::endl;
		t_y y = _y;


		auto ydiff_0 = [](t_real x, t_real y_0, t_real y_1) -> t_real
		{	// dy/dx = y1
			return y_1;
		};

		auto ydiff_1 = [](t_real x, t_real y_0, t_real y_1) -> t_real
		{	// dy1/dx = d^2y/dx^2 = c*y'^2 + g
			return c*y_1*y_1 + g;
		};

		for(t_real x=x_start; x<x_end-x_step; x+=x_step)
		{
			y[0] += x_step*ydiff_0(x, y[0], y[1]);
			y[1] += x_step*ydiff_1(x, y[0], y[1]);
		}

		std::cout << "final: y=" << y[0] << ", dy/dx=" << y[1] << std::endl;
	}
}


int main()
{
	riccati_tst();

	return 0;
}
