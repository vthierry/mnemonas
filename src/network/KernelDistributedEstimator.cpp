#include "main.hpp"

network::KernelDistributedEstimator::KernelDistributedEstimator(KernelTransform& transform, TransformCriterion& criterion) : KernelEstimator(transform, criterion)
{
  unsigned int D = 0;
  for(unsigned int n = 0; n < N; n++) {
    unsigned int d = transform.getKernelDimension(n);
    D = D < d ? d : D;
  }
  A = new double[D * D];
  b = new double[D];
  w0 = new double[D];
  w1 = new double[D];
}
/// @cond INTERNAL
network::KernelDistributedEstimator::~KernelDistributedEstimator()
{
  delete[] w1;
  delete[] w0;
  delete[] b;
  delete[] A;
}
///@endcond
double network::KernelDistributedEstimator::run(double criterion_epsilon, unsigned int maxIterations, unsigned int batch_duration, unsigned int batch_count, bool with_update)
{
  double cost0 = criterion.rho(), cost_oo = cost0 * criterion_epsilon;
  cost = cost0;
  for(unsigned int i = 0; (maxIterations == 0 || i < maxIterations) && cost_oo < cost; i++)
    run_once(batch_duration, batch_count, with_update);
  return cost;
}
void network::KernelDistributedEstimator::run_once(unsigned int batch_duration, unsigned int batch_count, bool with_update)
{
  // Performs backward tuning and calculates the 2nd order system
  double cost0 = KernelEstimator::run();
  for(unsigned int k = 0; k < batch_count; k++) {
    // Randomly selects a unit with weight to adjust
    unsigned int n, D;
    {
      assume(transform.getWeightCount() > 0, "illegal-argument", "in network::KernelDistributedEstimator::run_once : no weight to adjust");
      do
        n = random::uniform(0, N), D = transform.getKernelDimension(n);
      while(D == 0);
    }
    // Randomly selects an epoch
    unsigned int S = batch_duration < D ? D : batch_duration, t0 = random::uniform(0, T - S);
    if(with_update)
      criterion.update();
    for(unsigned int s = 0, sr = 0; s < S; s++) {
      b[s] = getValue(n, t0 + s) - getError(n, t0 + s) - transform.getKernelValue(n, 0, t0 + s);
      for(unsigned int r = 0; r < D; r++, sr++)
        A[sr] = transform.getKernelValue(n, r + 1, t0 + s);
    }
    for(unsigned int d = 0; d < D; d++)
      w0[d] = transform.getWeight(n, d + 1);
    // Solves the 2nd order system and performs line search around the solution
    solver::linsolve(S, D, A, false, b, w1, w0);
    solver_minimize_e = this, n_f = n, D_f = D, c_f = 0;
    double u = solver::minimize(solver_minimize_e_f, -10, 10, 1e-1);
    line_search_values.add(u);
    line_search_counts.add(c_f);
    printf("run_once { 'n': %d, 'u': %6.4f, 'c': %d, 'cost': %6.2g, 'ok': %d, 'd_cost' : %g, 'delta_cost' : %g }\n", n, u, c_f, cost, cost < cost0, cost0 - cost, (cost0 - cost) / cost0);
    // Restore initial weights if no improvement
    if(cost < cost0)
      cost0 = cost;
    else {
      for(unsigned int d = 0; d < D; d++)
        transform.setWeight(n, d + 1, w0[d]);
      cost = cost0;
    }
  }
}
double network::KernelDistributedEstimator::solver_minimize_f(double u)
{
  c_f++;
  for(unsigned int d = 0; d < D_f; d++)
    transform.setWeight(n_f, d + 1, w0[d] + u * (w1[d] - w0[d]));
  // - printf("\t{ '#': %d, 'u': %6.4f } %s\n", c_f, u, transform.asString().c_str());
  return cost = criterion.rho();
}
double network::KernelDistributedEstimator::solver_minimize_e_f(double u)
{
  return solver_minimize_e->solver_minimize_f(u);
}
const CurveFit& network::KernelDistributedEstimator::getFit(String what) const
{
  return what == "line-search-value" ? line_search_values : what == "line-search-count" ? line_search_counts : network::KernelEstimator::getFit(what);
}
network::KernelDistributedEstimator *network::KernelDistributedEstimator::solver_minimize_e = NULL;
