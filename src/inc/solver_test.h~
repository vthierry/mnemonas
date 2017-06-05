void solver_test()
{
  double x[3];
  {
    static double A[] = { 5, 8, 13, 11, 18, 26 }, b[] = { 54, 88, 125 };
    solver::cholesky(3, A, b, x);
    double err = fabs(x[0] - 1) + fabs(x[1] - 2) + fabs(x[2] - 3);
    assume(err < 1e-14, "illegal-state", "solver_test/cholesky 1/2 failed [1 2 3] != [%g %g %g] err = %g\n", x[0], x[1], x[2], err);
  }
  {
    static double A[] = { 1, 2, 4, 3, 5, 6 }, b[] = { 14, 25, 31 };
    solver::cholesky(3, A, b, x);
    double err = fabs(x[0] - 1) + fabs(x[1] - 2) + fabs(x[2] - 3);
    assume(err < 1e-13, "illegal-state", "solver_test/cholesky 2/2 failed [1 2 3] != [%g %g %g] err = %g\n", x[0], x[1], x[2], err);
  }
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
}
