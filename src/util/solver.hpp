#ifndef solver_hpp
#define solver_hpp

#include "util/s_string.h"
#include "util/numeric.hpp"
#include "math.h"

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
   * @param x0 A <tt>double[N]</tt> input buffer. If NULL considers the zero vector. We may have x0 == x.
   * @return The estimation error <tt>| b - A x |</tt>
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

  /** Projects a point onto a manifold defined by implicit equation.
   * @param M The number of equations, the manifold is defined by M equations.
   * @param N The number of unknows.
   * @param c The constraint <tt>m</tt>-th equation value, \f$c_m({\bf x})\f$.
   * @param d The constraint <tt>m</tt>-th equation, <tt>n</tt>-th component derivative value, \f$\partial_{x_n}c_m({\bf x})\f$.
   * @param x A <tt>double[N]</tt> output buffer.
   * @param x0 A <tt>double[N]</tt> input buffer. If NULL considers the zero vector. We may have x0 == x.
   * @param epsilon The threshold under which two <tt>x[n]</tt> values are indistinguishable.
   * @param maxIterations The maximal number of iteration. The 0 value corresponds to unbounded interation.
   * @return The estimation error <tt>| c(x) |<sup>2</sup></tt>
   */
  static double projsolve(unsigned int M, unsigned int N, double c(const double *x, unsigned int m), double d(const double *x, unsigned int m, unsigned int n), double *x, const double *x0 = NULL, double epsilon = 1e-6, unsigned int maxIterations = 0);

  /** Returns a matrix or vector as a string to dump.
   * @param A The matrix buffer <tt>double[M * N]</tt> buffer storing <tt>A(i, j) == A[i + j * N]</tt>.
   * @param M The number of rows.
   * @param N The number of columns.
   * @param symmetric If the matrix is symmetric.
   * @param format The printf format.
   */
  static std::string asString(const double *A, unsigned int M, unsigned int N = 1, bool symmetric = false, String format = "%8.1g");
};

#endif
