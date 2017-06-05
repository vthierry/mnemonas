#include "solver.hpp"

#include <cmath>
#include <cfloat>

#include "assume.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

double solver::cholesky(unsigned int N, const double *A, const double *b, double *x)
{
  static const bool verbose = false;
  // Prepares the GLS matrix and vectors
  gsl_matrix *gsl_A = gsl_matrix_calloc(N, N);
  for(unsigned int i = 0, ij = 0; i < N; i++)
    for(unsigned int j = 0; j <= i; j++, ij++)
      gsl_matrix_set(gsl_A, i, j, A[ij]);
  gsl_vector_const_view gsl_b = gsl_vector_const_view_array(b, N);
  gsl_vector_view gsl_x = gsl_vector_view_array(x, N);
  // Performs the Cholesky decompositions
  gsl_error_handler_t *gsl_eh = *gsl_set_error_handler_off();
  int gsl_e1 = gsl_linalg_cholesky_decomp(gsl_A);
  gsl_set_error_handler(gsl_eh);
  if(gsl_e1 == 0)
    gsl_linalg_cholesky_solve(gsl_A, &gsl_b.vector, &gsl_x.vector);
  else {
    // Fallbacks on a SVD decomposition if it fails
    for(unsigned int i = 0; i < N; i++)
      for(unsigned int j = 0; j < N; j++)
        gsl_matrix_set(gsl_A, i, j, A[i < j ? i + j * (j + 1) / 2 : j + i * (i + 1) / 2]);
    gsl_matrix *gsl_V = gsl_matrix_alloc(N, N);
    gsl_vector *gsl_s = gsl_vector_alloc(N);
    gsl_vector *gsl_w = gsl_vector_alloc(N);
    gsl_linalg_SV_decomp(gsl_A, gsl_V, gsl_s, gsl_w);
    gsl_linalg_SV_solve(gsl_A, gsl_V, gsl_s, &gsl_b.vector, &gsl_x.vector);
    if(verbose) {
      printf("\nsvd = [");
      for(unsigned int n = 0; n < N; n++)
        printf(" %g", gsl_vector_get(gsl_s, n));
      printf(" ]");
    }
    gsl_matrix_free(gsl_V);
    gsl_vector_free(gsl_s);
    gsl_vector_free(gsl_w);
  }
  double c = 0;
  for(unsigned int i = 0; i < N; i++) {
    double e = b[i];
    for(unsigned int j = 0; j < N; j++)
      e -= A[i < j ? i + j * (j + 1) / 2 : j + i * (i + 1) / 2] * x[j];
    c += e * e;
  }
  c = sqrt(c / N);
  if(verbose) {
    printf("\n b = A x, error = %g :\n", c);
    for(unsigned int i = 0; i < N; i++) {
      printf("| %8.2g | = | ", b[i]);
      for(unsigned int j = 0; j < N; j++)
        if(j <= i)
          printf(" %8.2g", A[j + i * (i + 1) / 2]);
        else
          printf(" %8s", "");
      printf("| | %8.2g |\n", x[i]);
    }
  }
  return c;
}
double solver::minimize(double f(double x), double xmin, double xmax, double xeps, unsigned int imax)
{
  static const bool verbose = false;
  assume(xmin < xmax, "illegal argument", "in solver:minimize incorrect bounds [%f %f]\n", xmin, xmax);
  xeps = std::isnan(xeps) ? 1e-6 * (xmax - xmin) : 0.5 * xeps;
  double x0;
  // The code is imported from @todo citer la reference because there is a bug in the GSL for the brent algorithm
  {
    double c;
    double d;
    double e;
    double fu;
    double fv;
    double fw;
    double fx;
    double m;
    double p;
    double q;
    double r;
    double sa;
    double sb;
    double t2;
    double tol;
    double u;
    double v;
    double w;
    // C is the square of the inverse of the golden ratio.
    c = 0.5 * (3.0 - sqrt(5.0));
    // Initialization
    sa = xmin;
    sb = xmax;
    x0 = xmin + c * (xmax - xmin);
    w = x0;
    v = w;
    e = 0.0;
    fx = f(x0);
    fw = fx;
    fv = fw;
    for(unsigned int n = 0; imax == 0 || n < imax; n++) {
      if(verbose)
        printf("> solver/minimize [%g %g]\n", sa, sb);
      m = 0.5 * (sa + sb);
      tol = xeps / 2; // eps * fabs ( x0 ) + t;
      t2 = 2.0 * tol;
      // Check the stopping criterion.
      if(fabs(x0 - m) <= t2 - 0.5 * (sb - sa))
        break;
      // Fit a parabola.
      r = 0.0;
      q = r;
      p = q;
      if(tol < fabs(e)) {
        r = (x0 - w) * (fx - fv);
        q = (x0 - v) * (fx - fw);
        p = (x0 - v) * q - (x0 - w) * r;
        q = 2.0 * (q - r);
        if(0.0 < q)
          p = -p;
        q = fabs(q);
        r = e;
        e = d;
      }
      if((fabs(p) < fabs(0.5 * q * r)) &&
         (q * (sa - x0) < p) &&
         (p < q * (sb - x0)))
      {
        // Takes the parabolic interpolation step.
        d = p / q;
        u = x0 + d;
        // F must not be evaluated too close to A or B.
        if(((u - sa) < t2) || ((sb - u) < t2)) {
          if(x0 < m)
            d = tol;
          else
            d = -tol;
        }
      } else {
        // A golden-section step.
        if(x0 < m)
          e = sb - x0;
        else
          e = sa - x0;
        d = c * e;
      }
      // F must not be evaluated too close to X.
      if(tol <= fabs(d))
        u = x0 + d;
      else if(0.0 < d)
        u = x0 + tol;
      else
        u = x0 - tol;
      fu = f(u);
      // Updates A, B, V, W, and X.
      if(fu <= fx) {
        if(u < x0)
          sb = x0;
        else
          sa = x0;
        v = w;
        fv = fw;
        w = x0;
        fw = fx;
        x0 = u;
        fx = fu;
      }else {
        if(u < x0)
          sa = u;
        else
          sb = u;
        if((fu <= fw) || (w == x0)) {
          v = w;
          fv = fw;
          w = u;
          fw = fu;
        } else if((fu <= fv) || (v == x0) || (v == w)) {
          v = u;
          fv = fu;
        }
      }
    }
  }
  return x0;
}
