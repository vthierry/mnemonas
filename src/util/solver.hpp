#ifndef solver_hpp
#define solver_hpp

#include "numeric.hpp"
#include "math.h"
#include "s_string.h"

/** This factory encapsulates equation solvers.
 */
class solver: public numeric {
public:
  /** Solves a linear system of equations.
   * - It uses a SVD decomposition in order to calculates the pseudo-inverse of the system, i.e.
   * <center>\f$\min_{\bf x} |{\bf x} - {\bf x}_0| + \lambda \, ({\bf b} - {\bf A} \, {\bf x}) \f$</center>
   * - The computational cost is \f$O(\max(M,N)\,N^2)\f$.
   * @param M The number of equations.
   * @param N The number of unknows.
   * @param A Either
   * - a <tt>double[M*N]</tt> non-symmetric input buffer matrix, <tt>A(i, j) == A[i + j * N]</tt>.
   * - a <tt>double[N*(N+1)/2]</tt> diagonal and lower triangle input buffer of a symmetric matrix, <tt>A(i, j) == A[j + i*(i+1)/2], j <= i</tt>.
   * @param symmetric Specifies if the previous parameter is symmetric or not.
   * @param b A <tt>double[M]</tt> input buffer.
   * @param x A <tt>double[N]</tt> output buffer.
   * @param x0 A <tt>double[N]</tt> input buffer. If NULL considers the zero vector.
   * @return The estimation error | b - A x |
   */
  static double linsolve(unsigned int M, unsigned int N, const double *A, bool symmetric, const double *b, double *x, const double *x0 = NULL);

  /** Minimizes a 1D function in a given interval.
   * @param f The function to minimize.
   * @param xmin  The search lower bound.
   * @param xmax  The search upper bound.
   * @param xeps  The required precision. Default is <tt>xeps = 1e-6 (xmax - xmin)</tt>.
   * @param imax  Maximal number of iterations. Unbounded if 0.
   * @return The best minimal solution.
   */
  static double minimize(double f(double x), double xmin, double xmax, double xeps = NAN, unsigned int imax = 0);

  /** Returns a matrix or vector as a string to dump.
   * @param A The matrix buffer <tt>double[M * N]</tt> buffer storing <tt>A(i, j) == A[i + j * N]</tt>.
   * @param M The number of rows.
   * @param N The number of columns.
   * @param symmetric If the matrix is symmetric.
   * @param format The printf format.
   */
  static std::string asString(const double* A, unsigned int M, unsigned int N = 1, bool symmetric = false, String format = "%8.1g");
};

#endif
