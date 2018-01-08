#include "main.hpp"

network::KernelEstimator::KernelEstimator(KernelTransform& transform, TransformCriterion& criterion) : transform(transform), N(transform.getN()), T((int) transform.getT()), R(transform.getR()), criterion(criterion), weights0(new double[transform.getWeightCount()]), values0(new double[N * T]), errors(new double[N * T]), drho_mean(0), k_negligible(0), k_saturated(0)
{
  once = true;
}
/// @cond INTERNAL
network::KernelEstimator::~KernelEstimator()
{
  delete[] errors;
  delete[] values0;
  delete[] weights0;
}
///@endcond

double network::KernelEstimator::run()
{
  // Manages iteration time
  if(once)
    once = false, now(false, true);
  else
    iteration_time.add(now(false, true));
  // Calculates the cost, the rho() function simulates the network
  costs.add(cost = criterion.rho());
  simulation_time.add(now(false, true));
  // Saves previous values and weights
  for(unsigned int t = 0, nt = 0; t < T; t++)
    for(unsigned int n = 0; n < N; n++, nt++)
      values0[nt] = transform.get(n, t);
  for(unsigned int n = 0, nd = 0; n < N; n++)
    for(unsigned int d = 1; d <= transform.getKernelDimension(n); d++, nd++)
      weights0[nd] = transform.getWeight(n, d);
  // Initializes the backward guard variables
  double m0 = 0, s0 = 0, m1 = 0, s1 = 0;
  k_negligible = k_saturated = 0;
  // Calculates the backward tuning errors
  for(int t_ = T - 1; 0 <= t_; t_--)
    for(unsigned int n_ = 0, nt_ = N * t_; n_ < N; n_++, nt_++) {
      double e0 = criterion.drho(n_, t_), e1 = 0;
      if(e0 != 0)
        m0 += fabs(e0), s0++;
      for(int t = t_; t_ <= t + (int) R && t < (int) T; t++)
        for(unsigned int n = 0; n < (t == t_ ? n_ : N); n++)
          if(transform.isConnected(n, n_))
            e1 += transform.getValueDerivative(n, t, n_, t_) * errors[n + N * t];
      // Implements the backward guard, with nu = 1e-6, omega = 1e6 and a simple saturation.
      {
        if(0 < e1) {
          if(e1 < drho_mean * 1e-6) {
            e1 = drho_mean * (1e-6 + e1 * e1 * 2.5e5);
            k_negligible++;
          } else if(e1 > drho_mean * 1e6) {
            e1 = drho_mean * 1e6;
            k_saturated++;
          }
        } else if(e1 < 0) {
          if(drho_mean * -1e-6 < e1) {
            e1 = drho_mean * (-1e-6 + e1 * e1 * -2.5e5);
            k_negligible++;
          } else if(drho_mean * -1e6 > e1) {
            e1 = drho_mean * -1e6;
            k_saturated++;
          }
        }
      }
      if(e1 != 0)
        m1 += fabs(e1), s1++;
      errors[nt_] = e0 + e1;
      // - printf("%s %.1g %s", n_ == 0 ? (t_ == (int) T - 1 ? "[\n [" : " [" ): "" , errors[nt_], n_ == N - 1 ? (t_ == 0 ? "]\n]\n\n" : "]\n") : "");
    }
  output_errors.add(drho_mean = s0 > 0 ? m0 / s0 : 0);
  backward_errors.add(s1 > 0 ? m1 / s1 : 0);
  double NT = N * T;
  ratio_negligible.add(k_negligible / NT);
  ratio_saturated.add(k_saturated / NT);
  printf(">tuning time = %g\n", now(false, true));
  tuning_time.add(now(false, true));
  return cost;
}
double network::KernelEstimator::getValue(unsigned int n, double t) const
{
  if (n < criterion.getN0()) {
    double value = criterion.get(n, t);
    if (!isnan(value))
      return value;
  } 
  return values0[n + ((int) t) * N];
}
const CurveFit& network::KernelEstimator::getFit(String what) const
{
  return what == "output-error" ? output_errors : what == "backward-error" ? backward_errors :
         what == "ratio-negligible" ? ratio_negligible : what == "ratio-saturated" ? ratio_saturated : costs;
}
void network::KernelEstimator::updateReadOut(unsigned int N0)
{
  if (N0 == 0) N0 = criterion.getN0();
  cost0 = criterion.rho();
  // Reinjects the desired values in the last simulated values
  for(unsigned int t = 0; t < T; t++)
    for(unsigned int n = 0; n < N0; n++)
      values0[n + ((int) t) * N] = criterion.get(n, t);
  // Builds and solves the linear system of equations for each unit
  w0 = new double[transform.getWeightCount()], w1 = new double[transform.getWeightCount()];
  for(unsigned int n = 0, nd = 0; n < N0; n++) {
    unsigned int D = transform.getKernelDimension(n);
    for(unsigned int d = 0; d < D; d++) 
      w0[nd + d] = transform.getWeight(n, d);
    double *b = new double [D], *A = new double [(D * (D + 1))/2];
    for(unsigned int t = 0; t < T; t++) {
      double b_nt = getValue(n, t) - transform.getKernelValue(n, 0, t);
      for(unsigned int d = 0, dd_ = 0; d < D; d++) {
	double phi_ndt = transform.getKernelValue(n, d + 1, t);
	for(unsigned int d_ = 0; d_ <= d; d_++, dd_++)
	  A[dd_] += phi_ndt * transform.getKernelValue(n, d_ + 1, t);
	b[d] += phi_ndt * b_nt;
      }
      nd += D;
    }
    solver::linsolve(D, D, A, true, b, w1 + nd, w0 + nd);
    delete[] A;
    delete[] b;
  }
  // Line searchs in the 2nd order estimation
  solver_minimize_e = this, c_f = 0;
  double u = solver::minimize(solver_minimize_e_f, -10, 10, 1e-1);
  printf("run_once { 'u': %6.4f, 'c': %d, 'cost': %6.2g, 'ok': %d, 'd_cost' : %g, 'delta_cost' : %g }\n", u, c_f, cost1, cost1 < cost0, cost0 - cost, (cost0 - cost1) / cost0);    
  delete[] w1;
  delete[] w0;
}
double network::KernelEstimator::solver_minimize_f(double u)
{
  c_f++;
  for(unsigned int n = 0, nd = 0; n < N; n++)  
    for(unsigned int d = 0; d < transform.getKernelDimension(n); d++ ,nd++)
      transform.setWeight(n, d + 1, w0[d] + u * (w1[d] - w0[d]));
  // - printf("\t{ '#': %d, 'u': %6.4f }\n", c_f, u);
  return cost1 = criterion.rho();
}
double network::KernelEstimator::solver_minimize_e_f(double u)
{
  return solver_minimize_e->solver_minimize_f(u);
}
network::KernelEstimator *network::KernelEstimator::solver_minimize_e = NULL;
