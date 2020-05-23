/**
 * parser test -- runtime library
 * @author Tobias Weber
 * @date 17-apr-20
 * @license: see 'LICENSE.GPL' file
 */

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <limits>


using t_real = double;
using t_int = std::int64_t;

t_real g_eps = std::numeric_limits<t_real>::epsilon();


/**
 * tests equality of floating point numbers
 */
extern "C" bool ext_equals(t_real x, t_real y, t_real eps)
{
	t_real diff = x-y;
	if(diff < 0.)
		diff = -diff;
	return diff <= eps;
}


/**
 * removes a given row and column of a square matrix
 */
extern "C" void ext_submat(const t_real* M, t_int N, t_real* M_new, t_int iremove, t_int jremove)
{
	t_int row_new = 0;
	for(t_int row=0; row<N; ++row)
	{
		if(row == iremove)
			continue;

		t_int col_new = 0;
		for(t_int col=0; col<N; ++col)
		{
			if(col == jremove)
				continue;

			M_new[row_new*(N-1) + col_new] = M[row*N + col];
			++col_new;
		}
		++row_new;
	}

	/*std::printf("mat: ");
	for(t_int i=0; i<N*N; ++i) std::printf("%g ", M[i]);
	std::printf("\n");

	std::printf("submat(%ld, %ld): ", iremove, jremove);
	for(t_int i=0; i<(N-1)*(N-1); ++i) std::printf("%g ", M_new[i]);
	std::printf("\n");*/
}


/**
 * calculates the determinant
 */
extern "C" t_real ext_determinant(const t_real* M, t_int N)
{
	// special cases
	if(N == 0)
		return 0;
	else if(N == 1)
		return M[0];
	else if(N == 2)
		return M[0*N+0]*M[1*N+1] - M[0*N+1]*M[1*N+0];


	// get row with maximum number of zeros
	t_int row = 0;
	t_int maxNumZeros = 0;
	for(t_int curRow=0; curRow<N; ++curRow)
	{
		t_int numZeros = 0;
		for(t_int curCol=0; curCol<N; ++curCol)
		{
			if(ext_equals(M[curRow*N + curCol], 0, g_eps))
				++numZeros;
		}

		if(numZeros > maxNumZeros)
		{
			row = curRow;
			maxNumZeros = numZeros;
		}
	}
	//std::printf("det: expanding on row %ld\n", row);


	// recursively expand determiant along a row
	t_real fullDet = t_real(0);

	t_real *submat = reinterpret_cast<t_real*>(std::malloc(sizeof(t_real)*(N-1)*(N-1)));
	for(t_int col=0; col<N; ++col)
	{
		const t_real elem = M[row*N + col];
		if(ext_equals(elem, 0, g_eps))
			continue;

		ext_submat(M, N, submat, row, col);
		const t_real sgn = ((row+col) % 2) == 0 ? t_real(1) : t_real(-1);
		fullDet += elem * ext_determinant(submat, N-1) * sgn;
	}
	std::free(submat);

	return fullDet;
}



/**
 * inverted matrix
 */
extern "C" t_int ext_inverse(const t_real* M, t_real* I, t_int N)
{
	t_real fullDet = ext_determinant(M, N);

	// fail if determinant is zero
	if(ext_equals(fullDet, 0., g_eps))
		return 0;

	t_real *submat = reinterpret_cast<t_real*>(std::malloc(sizeof(t_real)*(N-1)*(N-1)));
	for(t_int i=0; i<N; ++i)
	{
		for(t_int j=0; j<N; ++j)
		{
			ext_submat(M, N, submat, i, j);
			const t_real sgn = ((i+j) % 2) == 0 ? t_real(1) : t_real(-1);
			I[j*N + i] = ext_determinant(submat, N-1) * sgn / fullDet;
		}
	}
	std::free(submat);

	return 1;
}


/**
 * matrix-matrix product: RES^i_j = M1^i_k M2^k_j
 */
extern "C" void ext_mult(const t_real* M1, const t_real* M2, t_real *RES, t_int I, t_int J, t_int K)
{
	for(t_int i=0; i<I; ++i)
	{
		for(t_int j=0; j<J; ++j)
		{
			RES[i*J + j] = t_real(0);

			for(t_int k=0; k<K; ++k)
				RES[i*J + j] += M1[i*K + k]*M2[k*J + j];
		}
	}
}


/**
 * matrix power
 */
extern "C" t_int ext_power(const t_real* M, t_real* P, t_int N, t_int POW)
{
	t_int POW_pos = POW<0 ? -POW : POW;
	t_int status = 1;

	// temporary matrices
	t_real *Mtmp = reinterpret_cast<t_real*>(std::malloc(sizeof(t_real)*N*N));
	t_real *Mtmp2 = reinterpret_cast<t_real*>(std::malloc(sizeof(t_real)*N*N));

	// Mtmp = M
	for(t_int i=0; i<N; ++i)
		for(t_int j=0; j<N; ++j)
			Mtmp[i*N + j] = M[i*N + j];

	// matrix power
	for(t_int i=0; i<POW_pos-1; ++i)
	{
		ext_mult(Mtmp, M, Mtmp2, N, N, N);

		// Mtmp = Mtmp2
		for(t_int i=0; i<N; ++i)
			for(t_int j=0; j<N; ++j)
				Mtmp[i*N + j] = Mtmp2[i*N + j];
	}

	// invert
	if(POW < 0)
		status = ext_inverse(Mtmp, Mtmp2, N);

	// P = Mtmp2
	for(t_int i=0; i<N; ++i)
		for(t_int j=0; j<N; ++j)
			P[i*N + j] = Mtmp2[i*N + j];

	std::free(Mtmp);
	std::free(Mtmp2);
	return status;
}



/**
 * transposed matrix
 */
extern "C" void ext_transpose(const t_real* M, t_real* T, t_int rows, t_int cols)
{
	for(t_int i=0; i<rows; ++i)
		for(t_int j=0; j<cols; ++j)
			T[j*rows + i] = M[i*cols + j];
}
