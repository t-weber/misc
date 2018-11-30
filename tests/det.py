#
# determinant test
# @date 24-nov-18
# @author tweber
# @license see 'LICENSE.EUPL' file
#

import numpy as np


g_eps = 1e-6


def det(M):
	NUMROWS = M.shape[0]
	NUMCOLS = M.shape[1]

	if NUMROWS != NUMCOLS:
		print("Need square matrix!")
		return 0.


	# recursion end conditions
	if NUMROWS == 0:
		return 0.
	elif NUMROWS == 1:
		return M[0][0]


	# find the column with the most zeros
	COL = 0
	maxZeros = 0

	for _COL in range(0, NUMCOLS):
		numZeros = 0

		for _ROW in range(0, NUMROWS):
			if M[_ROW, _COL] == 0.:
				numZeros += 1

		if numZeros > maxZeros:
			COL = _COL
			maxZeros = numZeros

	#print("Using column %d." % COL)


	# calculate determinant
	d = 0.
	for ROW in range(0, NUMROWS):
		elem = M[ROW, COL]
		if np.abs(elem) <= g_eps:
			continue

		sign = 1.
		if (ROW+COL) % 2 != 0:
			sign = -1.

		newidx_1 = [i for i in range(0, NUMROWS) if i!=ROW]
		newidx_2 = [j for j in range(0, NUMCOLS) if j!=COL]
		Msub = M[newidx_1, :][:, newidx_2]
		d += sign * elem * det(Msub)

	return d



# test and comparison with la.det()
import numpy.linalg as la

mat1 = np.array([[1,2],[3,4]])
mat2 = np.array([[1.5,2.2,0.],[-4,5,-6],[7,-8,9]])
print("det=%g, check=%g" % (det(mat1), la.det(mat1)))
print("det=%g, check=%g" % (det(mat2), la.det(mat2)))
