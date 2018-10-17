#
# ode test
# @author tweber
# @date oct-18
# @license: see 'LICENSE.EUPL' file
#

import numpy as np


# ODE: y'(x) = a*y(x) + b*x + c*x^2
a = 1.23
b = -4.56
c = 7.89
def dydx(y, x):
	return a*y + b*x + c*x**2.


# analytical integral, for checking
int_c = 5.
def y(x):
	return int_c*np.exp(a*x) - c/a*x**2. - b/a*x - 2.*c/a**2.*x - b/a**2. - 2.*c/a**3.


def euler(x_start, x_end, x_step, y_start):
	x_range = x_end - x_start
	xs = np.linspace(x_start, x_end-x_step, int(x_range/x_step))
	y = y_start

	for x in xs:
		y += x_step*dydx(y, x)

	return y


def runge(x_start, x_end, x_step, y_start):
	def k1234(y, x, x_step):
		k1 = x_step * dydx(y, x)
		k2 = x_step * dydx(y+k1*0.5, x+x_step*0.5)
		k3 = x_step * dydx(y+k2*0.5, x+x_step*0.5)
		k4 = x_step * dydx(y+k3, x+x_step)
		return [k1, k2, k3, k4]

	x_range = x_end - x_start
	xs = np.linspace(x_start, x_end-x_step, int(x_range/x_step))
	y = y_start

	for x in xs:
		[k1, k2, k3, k4] = k1234(y, x, x_step)
		y += (k1 + 2.*k2 + 2.*k3 + k4) / 6.

	return y



x_start = 0.
x_end = 5.
x_step = 0.001
y_start = y(x_start)


y_euler = euler(x_start, x_end, x_step, y_start)
print("Euler: y = %.6g" % y_euler)

y_runge = runge(x_start, x_end, x_step, y_start)
print("Runge: y = %.6g" % y_runge)

print("Analytical: y = %.6g" % (y(x_end)-y(x_start)))
