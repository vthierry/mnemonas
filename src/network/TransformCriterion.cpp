#include "main.hpp"

network::TransformCriterion::TransformCriterion(RecurrentTransform& transform) : transform(transform), estimates(NULL), destimates(NULL)
{}
/// @cond INTERNAL
network::TransformCriterion::~TransformCriterion()
{
  delete[] estimates;
  delete[] destimates;
}
///@endcond
double network::TransformCriterion::rho() const
{
  unsigned int N = transform.getN(), T = (int) transform.getT();
  double r = 0;
  transform.reset(true);
  // - printf("W: %s\n", transform.asString().c_str());
  for(unsigned int t = 0; t < T; t++)
    for(int n = N - 1; 0 <= n; n--)
      r += rho(n, t);
  // - printf("%s %.1g %s", n == (int) N - 1 ? (t == 0 ? "[\n [" : " [" ): "" , transform.get(n, t), n == 0 ? (t == T - 1 ? "]\n]\n\n" : "]\n") : "");
  assume(r < 1e100, "numerical-error", "in network::TransformCriterion::forward_simulation huge cost = %g, the simulation is divergent", r);
  return r / (N * T);
}
double network::TransformCriterion::rho(unsigned int n, double t) const
{
  assume(false, "illegal-state", "in network::TransformCriterion::rho, this virtual method must be overridden");
  return NAN;
}
double network::TransformCriterion::drho(unsigned int n, double t) const
{
  assume(false, "illegal-state", "in network::TransformCriterion::rho, this virtual method must be overridden");
  return NAN;
}
double network::TransformCriterion::get(unsigned int n, double t) const
{
  return estimates == NULL ? NAN : estimates[n + transform.getN() * (int) t];
}
unsigned int network::TransformCriterion::getN0() const
{
  return estimates == NULL ? 0 : transform.getN();
}
void network::TransformCriterion::update()
{
  if(estimates == NULL) {
    estimates = new double[transform.getN() * ((int) transform.getT())];
    destimates = new double[transform.getN() * ((int) transform.getT())];
  }
  unsigned int N = transform.getN(), N0 = getN0(), T = transform.getT(), R = transform.getR();
  transform.reset(true);
  double r = 1;
  static const unsigned int K = 20;
  for(unsigned int k = 0; k < K && 1e-6 < r; k++) {
    for(unsigned int t = 0; t < T; t++)
      for(int n = N - 1; 0 <= n; n--) {
        double v = transform.get(n, t);
        estimates[n + t * N] = n < (int) N0 ? get(n, t) : v;
      }
    if(k > 0)
      for(int t_ = T - 1; 0 <= t_; t_--)
        for(unsigned int n_ = 0, nt_ = N * t_; n_ < N; n_++, nt_++)
          if(N0 <= n_)
            for(int t = t_; t_ <= t + (int) R && t < (int) T; t++)
              for(unsigned int n = 0; n < (t == t_ ? n_ : N); n++)
                estimates[n_ + t_ * N] += transform.getValueDerivative(n, t, n_, t_) * destimates[n + t * N];
    r = 0;
    for(unsigned int t = 0; t < T; t++)
      for(unsigned int n = 0; n < N; n++) {
        double e = destimates[n + t * N] = estimates[n + t * N] - transform.get(n, t);
        r += e * e;
      }
    r = sqrt(r / (N * T));
    for(unsigned int t = 0; t < T; t++)
      for(unsigned int n = 0; n < N; n++)
        transform.set(n, t, estimates[n + t * N]);
    // - printf("network::SupervisedCriterion::update() r[%d] = %g\n", k, r);
  }
}
