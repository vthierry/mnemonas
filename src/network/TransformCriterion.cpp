#include "main.hpp"

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
void network::TransformCriterion::update()
{}
double network::TransformCriterion::get(unsigned int n, double t) const
{
  return NAN;
}
unsigned int network::TransformCriterion::getN0() const
{
  return 0;
}
