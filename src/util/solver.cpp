#include <unistd.h>
#include "solver.hpp"

#include <cmath>
#include <cfloat>
#include "Density.hpp"

#include "assume.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include "s_string.h"
#include "s_printf.h"

double solver::linsolve(unsigned int M0, unsigned int N, const double *A, bool symmetric, const double *b, double *x, const double *x0)
{
  static const bool verbose = false;
  // Prepares the matrix and vectors
  double c = NAN;
  // Note: We must have M >= N for the SVD, thus complete with 0 rows
  unsigned int M = M0 < N ? N : M0;
  gsl_matrix *gsl_A = gsl_matrix_calloc(M, N);
  gsl_vector *gsl_b = gsl_vector_calloc(M);
  gsl_vector *gsl_x0 = NULL;
  {
    for(unsigned int j = 0; j < M0; j++)
      for(unsigned int i = 0; i < N; i++)
        gsl_matrix_set(gsl_A, j, i, symmetric ? A[i < j ? i + j * (j + 1) / 2 : j + i * (i + 1) / 2] : A[i + j * N]);
    for(unsigned int j = 0; j < M0; j++)
      gsl_vector_set(gsl_b, j, b[j]);
  }
  gsl_vector_view gsl_x = gsl_vector_view_array(x, N);
  // Shifts the unknown to x0
  if(x0 != NULL) {
    {
      // Here we solve (b - A x0) = A (x - x0) in order the pseudo inverse to minmize | x - x0 |
      gsl_vector_const_view gsl_x0 = gsl_vector_const_view_array(x0, N);
      // Ax0 = A  x0 - b
      gsl_blas_dgemv(CblasNoTrans, -1.0, gsl_A, &gsl_x0.vector, 1.0, gsl_b);
    }
    if(x0 == x) {
      gsl_x0 = gsl_vector_calloc(M);
      for(unsigned int i = 0; i < N; i++)
        gsl_vector_set(gsl_x0, i, x0[i]);
    }
  }
  // Solves the system in the least-square sense
  {
    // Performs the SVD and peudo-inverse
    {
      gsl_matrix *gsl_V = gsl_matrix_alloc(N, N);
      gsl_vector *gsl_s = gsl_vector_alloc(N);
      gsl_vector *gsl_w = gsl_vector_alloc(N);
      gsl_linalg_SV_decomp(gsl_A, gsl_V, gsl_s, gsl_w);
      // Thresholds singular values that are to small
      {
        double threshold = 1e-12 * gsl_vector_get(gsl_s, 0);
        for(unsigned int n = 1; n < N; n++)
          if(gsl_vector_get(gsl_s, n) < threshold)
            gsl_vector_set(gsl_s, n, 0);
      }
      gsl_linalg_SV_solve(gsl_A, gsl_V, gsl_s, gsl_b, &gsl_x.vector);
      if(verbose) {
        printf("svd = [");
        for(unsigned int n = 0; n < N; n++)
          printf(" %g", gsl_vector_get(gsl_s, n));
        printf(" ]\n");
      }
      gsl_matrix_free(gsl_V);
      gsl_vector_free(gsl_s);
      gsl_vector_free(gsl_w);
    }
    // Calculates the residual error
    {
      gsl_blas_dgemv(CblasNoTrans, -1.0, gsl_A, &gsl_x.vector, 1.0, gsl_b);
      c = sqrt(gsl_blas_dnrm2(gsl_b) / M0);
    }
    gsl_matrix_free(gsl_A);
    gsl_vector_free(gsl_b);
  }
  if(x0 != NULL) {
    if(gsl_x0 == NULL) {
      gsl_vector_const_view gsl_x0 = gsl_vector_const_view_array(x0, N);
      gsl_vector_add(&gsl_x.vector, &gsl_x0.vector);
    } else {
      gsl_vector_add(&gsl_x.vector, gsl_x0);
      gsl_vector_free(gsl_x0);
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
  // Code imported from https://people.sc.fsu.edu/~jburkardt/cpp_src/brent/brent.cpp because there is a bug in the GSL for the brent algorithm
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
    d = 0;
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
double solver::projsolve(unsigned int N, unsigned int M, double c(const double *x, unsigned int m), double d_c(const double *x, unsigned int m, unsigned int n), double *x, const double *x0_, double epsilon, unsigned int maxIterations)
{
  double *x0 = new double[N], *x1 = new double[N], *lambda = new double[M], *b = new double[M], *A = new double[(M * (M + 1)) / 2];
  // Initial point
  for(unsigned int n = 0; n < N; n++)
    x0[n] = x0_ == NULL ? 0 : x0_[n], x[n] = x0[n];
  double r0 = 0;
  for(unsigned int m = 0; m < M; m++)
    r0 += fabs(c(x, m));
  // Projection loop
  for(unsigned int k = 0; maxIterations == 0 || k < maxIterations; k++) {
    // Solves the lambda linear system
    {
      for(unsigned int m = 0; m < M; m++) {
        b[m] = c(x, m);
        for(unsigned int n = 0; n < N; n++)
          b[m] -= d_c(x, m, n) * (x[n] - x0[n]);
      }
      for(unsigned int mm_ = 0; mm_ < (M * (M + 1)) / 2; mm_++)
        A[mm_] = 0;
      for(unsigned int n = 0; n < N; n++)
        for(unsigned int m = 0, mm_ = 0; m < M; m++)
          for(unsigned int m_ = 0; m_ <= m; m_++, mm_++)
            A[mm_] += d_c(x, m, n) * d_c(x, m_, n);
      solver::linsolve(M, M, A, true, b, lambda);
      // printf(" A:\n %s b:\n %s lambda:\n %s", solver::asString(A, M, M, true).c_str(), solver::asString(b, M).c_str(), solver::asString(lambda, M).c_str());
    }
    // Line searchs the best projection
    {
      bool failed = true;
      for(double u_abs = 1; u_abs > epsilon; u_abs *= 0.5)
        for(int u_sg = -1; u_sg < 2; u_sg += 2) {
          double u = u_abs * u_sg;
          // Updates the estimates
          for(unsigned int n = 0; n < N; n++)
            for(unsigned int m = 0; m < M; m++)
              x1[n] = x[n] - u * (x[n] - x0[n] + d_c(x, m, n) * lambda[m]);
          // Recomputes the projection error
          {
            double r1 = 0;
            for(unsigned int m = 0; m < M; m++)
              r1 += fabs(c(x1, m));
            // printf("r[k=%d, u = %g] = %g < r0 = %g ok = %g\n", k, u, r1, r0,r0 - r1);
            // Updates the estimates
            if(r1 < r0) {
              r0 = r1;
              for(unsigned int n = 0; n < N; n++)
                x[n] = x1[n];
              failed = false;
              break;
            }
          }
        }
      if(failed)
        break;
    }
  }
  delete[] A;
  delete[] b;
  delete[] lambda;
  delete[] x1;
  delete[] x0;
  return r0;
}
double solver::getSpectralRadius(const double *A, unsigned int N, double epsilon, unsigned int maxIterations)
{
  double r0 = 0, *u0 = new double[N], r1 = 0, *u1 = new double[N];
  for(unsigned int n = 0; n < N; n++)
    u1[n] = Density::gaussian(), r1 += u1[n] * u1[n];
  r0 = sqrt(r1);
  for(unsigned int k = 0; k < maxIterations && epsilon < fabs(r0 - r1); k++) {
    r1 = r0;
    if(r1 > 0)
      for(unsigned int n = 0; n < N; n++)
        u1[n] /= r1;
    r0 = 0;
    for(unsigned int n = 0, nn = 0; n < N; n++) {
      u0[n] = 0;
      for(unsigned int n_ = 0; n_ < N; n_++, nn++)
        u0[n] += A[nn] * u1[n_];
      r0 += u0[n] * u0[n];
    }
    r0 = sqrt(r0);
#if 0
    {
      double e1 = 0;
      if(r0 > 0)
        for(unsigned int n = 0; n < N; n++)
          e1 += fabs(u0[n] / r0 - u1[n]);
      printf(">> k = %d r0 = %g |u0-u1| = %g\n", k, r0, e1);
    }
#endif
    double *u;
    u = u0, u0 = u1, u1 = u;
  }
  delete[] u0;
  delete[] u1;
  return r0;
}
std::string solver::asString(const double *A, unsigned int M, unsigned int N, bool symmetric, String format)
{
  std::string s;
  for(unsigned int j = 0; j < M; j++)
    for(unsigned int i = 0; i < N; i++)
      s += s_printf("%s" + format + "%s",
                    i == 0 ? "\t| " : " ",
                    symmetric ? A[i < j ? i + j * (j + 1) / 2 : j + i * (i + 1) / 2] : A[i + j * N],
                    i < N - 1 ? "" : " |\n");
  return s;
}
