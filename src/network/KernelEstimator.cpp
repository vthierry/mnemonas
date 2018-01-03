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
  double value = criterion.get(n, t);
  return isnan(value) ? values0[n + ((int) t) * N] : value;
}
const CurveFit& network::KernelEstimator::getFit(String what) const
{
  return what == "output-error" ? output_errors : what == "backward-error" ? backward_errors :
         what == "ratio-negligible" ? ratio_negligible : what == "ratio-saturated" ? ratio_saturated : costs;
}
