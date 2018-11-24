#
# determinant test
# @date 24-nov-18
# @author tweber
# @license see 'LICENSE.EUPL' file
#

import numpy as np


def det(M):
	NUMROWS = M.shape[0]
	NUMCOLS = M.shape[1]

	if NUMROWS != NUMCOLS:
		print("Need square matrix!")
		return 0.

	if NUMROWS == 0:
		return 0.
	elif NUMROWS == 1:
		return M[0][0]

	d = 0.
	COL = 0
	for ROW in range(0, NUMROWS):
		sign = 1.
		if (ROW+COL) % 2 != 0:
			sign = -1.

		Msub = np.delete(np.delete(M, ROW, 0), COL, 1)
		d += sign*M[ROW, COL]*det(Msub)

	return d


# test and comparison with la.det()
import numpy.linalg as la

mat1 = np.array([[1,2],[3,4]])
mat2 = np.array([[1.5,2.2,3.1],[-4,5,-6],[7,-8,9]])
print(det(mat1))
print(la.det(mat1))

print(det(mat2))
print(la.det(mat2))
