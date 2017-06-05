#ifndef solver_hpp
#define solver_hpp

#include "math.h"

/** This factory encapsulates equation solvers.
 */
class solver {
public:
  /** Solves a linear system of equations for a symmetric matrix.
   * - It uses the Cholesky decomposition and fall backs to a SVD decomposition if the matrix is not definite postive.
   * @param N The problem dimension.
   * @param A A <tt>double[N*(N+1)/2]</tt> diagonal and lower triangle input buffer of a symmetric matrix, <tt>A(i, j) == A[j + i*(i+1)/2], j <= i</tt>.
   * @param b A <tt>double[N]</tt> input buffer.
   * @param x A <tt>double[N]</tt> output buffer.
   * @return The estimation error | b - A x |
   */
  static double cholesky(unsigned int N, const double *A, const double *b, double *x);

  /** Minimizes a 1D function in a given interval.
   * @param f The function to minimize.
   * @param xmin  The search lower bound.
   * @param xmax  The search upper bound.
   * @param xeps  The required precision. Default is <tt>xeps = 1e-6 (xmax - xmin)</tt>.
   * @param imax  Maximal number of iterations. Unbounded if 0.
   * @return The best minimal solution.
   */
  static double minimize(double f(double x), double xmin, double xmax, double xeps = NAN, unsigned int imax = 0);
};

#endif
