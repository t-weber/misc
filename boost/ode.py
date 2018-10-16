#
# ode test
# @author tweber
# @date oct-18
# @license: see 'LICENSE.EUPL' file
#

import numpy as np


# ODE: y' = 1*y + 2*x
def dydx(y, x):
	return 1.*y + 2.*x


# analytical integral, for checking
def y(x):
	c = 5.
	return c*np.exp(x) - 2.*x - 2.


def euler(x_start, x_end, x_step, y_start):
	xs = np.linspace(x_start, x_end-x_step, int(np.round((x_end-x_start)/x_step)))
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

	xs = np.linspace(x_start, x_end-x_step, int(np.round((x_end-x_start)/x_step)))
	y = y_start

	for x in xs:
		[k1, k2, k3, k4] = k1234(y, x, x_step)
		y += (k1 + 2.*k2 + 2.*k3 + k4) / 6.

	return y



x_start = 0.
x_end = 7.
x_step = 0.001
y_start = 3.


y_euler = euler(x_start, x_end, x_step, y_start)
print("Euler: y = %.6g" % y_euler)

y_runge = runge(x_start, x_end, x_step, y_start)
print("Runge: y = %.6g" % y_runge)

print("Analytical: y = %.6g" % (y(x_end)-y(x_start)))
