///@cond INTERNAL

void solver_test()
{
  // solver::linsolve
  {
    double x[3];
    {
      static double A[] = { 5, 8, 13, 11, 18, 26 }, b[] = { 54, 88, 125 };
      solver::linsolve(3, 3, A, true, b, x);
      double err = fabs(x[0] - 1) + fabs(x[1] - 2) + fabs(x[2] - 3);
      assume(err < 1e-12, "illegal-state", "solver_test/linsolve 1/3 failed [1 2 3] != [%g %g %g] err = %g\n", x[0], x[1], x[2], err);
    }
    {
      static double A[] = { 1, 2, 4, 3, 5, 6 }, b[] = { 14, 25, 31 };
      solver::linsolve(3, 3, A, true, b, x);
      double err = fabs(x[0] - 1) + fabs(x[1] - 2) + fabs(x[2] - 3);
      assume(err < 1e-12, "illegal-state", "solver_test/linsolve 2/3 failed [1 2 3] != [%g %g %g] err = %g\n", x[0], x[1], x[2], err);
    }
    {
      static double A[] = { 0, 0, 1, 1, 0, 0 }, b[] = { 3, 1 }, x0[] = { 0, 2, 0 };
      {
        solver::linsolve(2, 3, A, false, b, x, x0);
        double err = fabs(x[0] - 1) + fabs(x[1] - 2) + fabs(x[2] - 3);
        assume(err < 1e-12, "illegal-state", "solver_test/linsolve 3/3 failed [1 2 3] != [%g %g %g] err = %g\n", x[0], x[1], x[2], err);
      }
      // Also tests with x == x0
      {
        solver::linsolve(2, 3, A, false, b, x0, x0);
        double err = fabs(x0[0] - 1) + fabs(x0[1] - 2) + fabs(x0[2] - 3);
        assume(err < 1e-12, "illegal-state", "solver_test/linsolve 3/3 failed [1 2 3] != [%g %g %g] err = %g\n", x0[0], x0[1], x0[2], err);
      }
    }
  }
  // solver::minimize 1D
  {
    class F {
public:
      static double f(double x)
      {
        return pow(x - 3.1416, 2);
      }
    }
    f;
    double x0 = solver::minimize(f.f, 0, 4);
    assume(fabs(x0 - 3.1416) < 1e-13, "illegal-state", "solver_test/minimize failed 3.1416 != %g\n");
  }
  // solver::minimize nD
  {
    class C {
public:
      static double f(const double *x)
      {
        return 0.5 * (C::sqr(x[0] - 1) + C::sqr(x[1] - 2) + C::sqr(x[2] - 3));
      }
      static double d_f(const double *x, unsigned int n)
      {
        return n == 0 ? x[0] - 1 : n == 1 ? x[1] - 2 : n == 2 ? x[2] - 3 : NAN;
      }
      static double sqr(double x)
      {
        return x * x;
      }
    }
    c;
    double x[3];
    double err = solver::minimize(3, c.f, c.d_f, x, NULL, 1e-16, 100);
    assume(err < 1e-13, "illegal-state", "solver_test/minimize [1 2 3] != [%g %g %g] err = %g\n", x[0], x[1], x[2], err);
  }
  // solver::minimize nD
  {
    class C {
public:
      static double f(const double *x)
      {
        return 0.5 * (fabs(x[0] - 1) + fabs(x[1] - 2) + fabs(x[2] - 3));
      }
      static double d_f(const double *x, unsigned int n)
      {
        return (n == 0 ? C::sg(x[0] - 1) : n == 1 ? C::sg(x[1] - 2) : n == 2 ? C::sg(x[2] - 3) : NAN) * (1 + rand() * 0.01);
      }
      static double sg(double x)
      {
        return x < 0 ? -1 : x > 0 ? 1 : 0;
      }
    }
    c;
    double x[3];
    double err = solver::minimize(3, c.f, c.d_f, x, NULL, 1e-16, 100);
    assume(err < 1e-6, "illegal-state", "solver_test/minimize [1 2 3] != [%g %g %g] err = %g\n", x[0], x[1], x[2], err);
  }
  // solver::project
  {
    class C {
public:
      static double c(const double *x, unsigned int m)
      {
        return m < 1 ? pow(x[0] * x[0] + x[1] * x[1] + x[2] * x[2], 4) - 1 : NAN;
      }
      static double d(const double *x, unsigned int m, unsigned int n)
      {
        return m < 1 && n < 3 ? 2 * x[n] * pow(x[0] * x[0] + x[1] * x[1] + x[2] * x[2], 3) : NAN;
      }
    }
    c;
    double x0[] = { 1, 1, 1 };
    double err = solver::projsolve(3, 1, c.c, c.d, x0, x0);
    assume(fabs(x0[0] - 1 / sqrt(3)) + fabs(x0[1] - 1 / sqrt(3)) + fabs(x0[2] - 1 / sqrt(3)) < 1e-13, "illegal-state", "solver_test/project [1 1 1]/3^1/2 != [%g %g %g] err = %g\n", x0[0], x0[1], x0[2], err);
  }
  // solver::getspectralradius
  {
    double M[] = { 1, 2, 2, 1 }, r = solver::getSpectralRadius(M, 2);
    assume(fabs(r - 3) < 1e-6, "illegal-state", "solver_test/getspectralradius r = %g != 3 err = %g\n", r, fabs(r - 3));
  }
}
///@endcond
