#include "mnemonas.hpp"

network::KernelExperimentalEstimator::KernelExperimentalEstimator(KernelTransform& transform, TransformCriterion& criterion) : KernelEstimator(transform, criterion), upsilon(0), grads(new double *[K_GRADS + K_SYLIN]), k_grads(new double[K_GRADS]), n_grads(new double[K_GRADS + K_SYLIN]), A(new double **[K_SYLIN]), b(new double **[K_SYLIN]), w1(new double[transform.getWeightCount()])
{
  for(unsigned int k = 0; k < K_GRADS + K_SYLIN; k++) {
    grads[k] = new double[transform.getWeightCount()];
    if(k < K_GRADS)
      k_grads[k] = pow(2, -(int) k);
  }
  for(unsigned int k = 0; k < K_SYLIN; k++) {
    A[k] = new double *[N];
    b[k] = new double *[N];
    for(unsigned int n = 0; n < N; n++) {
      unsigned int D = transform.getKernelDimension(n);
      A[k][n] = new double[D * (D + 1) / 2];
      b[k][n] = new double[D];
    }
  }
}
/// @cond INTERNAL
network::KernelExperimentalEstimator::~KernelExperimentalEstimator()
{
  delete[] k_grads;
  delete[] n_grads;
  for(unsigned int k = 0; k < K_GRADS + K_SYLIN; k++)
    delete[] grads[k];
  delete[] grads;
  for(unsigned int k = 0; k < K_SYLIN; k++) {
    for(unsigned int n = 0; n < N; n++) {
      delete[] A[k][n];
      delete[] b[k][n];
    }
    delete[] A[k];
    delete[] b[k];
  }
  delete[] A;
  delete[] b;
  delete[] w1;
}
///@endcond
#define clear_buffer(buffer, size) \
  for(unsigned int ijk = 0; ijk < size; ijk++) \
    buffer[ijk] = 0;
#define set_weights(w_nd) \
  for(unsigned int n = 0, nd = 0; n < N; n++) \
    for(unsigned int d = 1; d <= transform.getKernelDimension(n); d++, nd++) \
      transform.setWeight(n, d, w_nd);
#define dumpAdd(s) \
  if(dumpName != "") { \
    dumpString += s; if(dumpName == "stdout") \
      printf("%s", s.c_str()); \
  }
void network::KernelExperimentalEstimator::backward_tuning()
{
  // Resets the buffers
  {
    clear_buffer(grads[0], transform.getWeightCount());
    for(unsigned int n = 0; n < N; n++) {
      unsigned int D = transform.getKernelDimension(n);
      for(unsigned int k = 0; k < K_SYLIN; k++) {
        clear_buffer(A[k][n], D * (D + 1) / 2);
        clear_buffer(b[k][n], D);
      }
    }
  }
  // Runs backward tuning
  KernelEstimator::run();
  criterion.update();
  // Weight adjustement 1st and 2nd order elements calculation
  for(int t_ = T - 1; 0 <= t_; t_--)
    for(unsigned int n_ = 0, nd = 0, nt_ = N * t_; n_ < N; n_++, nt_++) {
      // Calculates gradient
      {
        for(unsigned int d = 1; d <= transform.getKernelDimension(n_); d++, nd++)
          grads[0][nd] += transform.getKernelValue(n_, d, t_) * errors[nt_];
      }
      // Calculates 2nd order system
      if(transform.getKernelDimension(n_) <= DMAX)
        for(unsigned int k = 0; k < K_SYLIN; k++) {
          double err = getValue(n_, t_) - transform.getKernelValue(n_, 0, t_);
          double kappa_nt = k == 1 ? 1 / (1 + fabs(err)) : 1;
          double b_nt = k == 2 ? kappa_nt * err : kappa_nt * err - errors[nt_];
          for(unsigned int d = 0, dd_ = 0; d < transform.getKernelDimension(n_); d++) {
            double phi_ndt = transform.getKernelValue(n_, d + 1, t_);
            for(unsigned int d_ = 0; d_ <= d; d_++, dd_++)
              A[k][n_][dd_] += kappa_nt * phi_ndt * transform.getKernelValue(n_, d_ + 1, t_);
            b[k][n_][d] += phi_ndt * b_nt;
          }
        }
    }
  // Updates the 1st order gradients
  {
    for(unsigned int k = 1; k < K_GRADS; k++)
      for(unsigned int nd = 0; nd < transform.getWeightCount(); nd++)
        grads[k][nd] += k_grads[k] * (grads[0][nd] - grads[k][nd]);
  }
  // Solves the 2nd order equation
  {
    for(unsigned int k = 0; k < K_SYLIN; k++) {
      for(unsigned int n = 0, nd = 0; n < N; n++) {
        unsigned int D = transform.getKernelDimension(n);
        if((0 < D) && (D <= DMAX)) {
          solver::linsolve(D, D, A[k][n], true, b[k][n], w1 + nd, weights0 + nd);
          nd += D;
        } else
          for(unsigned int d = 0; d < D; d++, nd++)
            w1[nd] = weights0[nd];
      }
      for(unsigned int nd = 0; nd < transform.getWeightCount(); nd++)
        grads[K_GRADS + k][nd] = weights0[nd] - w1[nd];
    }
  }
  // Calculates some backward-tuning statistics
  if(dumpName != "") {
    {
      Histogram stat;
      for(unsigned int nt = 0; nt < N * T; stat.add(errors[nt++])) ;
      dumpAdd(s_printf("\n      'errors': %s, ", stat.asString("mean stdev min max", true).c_str()));
    }
    dumpAdd(s_printf("\n      'costs': %s,\n      'guard': { 'nu': '%.0f%%', 'omega': '%.0f%%'},\n",
                     costs.asString().c_str(),
                     100.0 * k_negligible / (N * T),
                     100.0 * k_saturated / (N * T)));
  }
}
double network::KernelExperimentalEstimator::set_second_order_weights(double upsilon)
{
  set_weights(weights0[nd] - upsilon * grads[solver_minimize_k][nd]);
  return criterion.rho();
}
double network::KernelExperimentalEstimator::solver_minimize_f(double x)
{
  return solver_minimize_e->set_second_order_weights(x);
}
void network::KernelExperimentalEstimator::run_init(double weights_epsilon)
{
  for(unsigned int k = 1; k < K_GRADS; k++)
    clear_buffer(grads[k], transform.getWeightCount());
  clear_buffer(n_grads, K_GRADS + K_SYLIN);
  solver_minimize_e = this;
  upsilon = weights_epsilon * 100;
  cost0 = cost = KernelEstimator::run();
}
bool network::KernelExperimentalEstimator::run_once(double weights_epsilon)
{
  backward_tuning();
  // Attempts to improve by a brent search the different next estimates
  {
    double upsilons[K_GRADS + K_SYLIN], costs[K_GRADS + K_SYLIN], c_min = cost;
    unsigned int k_min = -1;
    for(unsigned int k = 0; k < K_GRADS + K_SYLIN; k++) {
      double u = k < K_GRADS ? upsilon : 1;
      solver_minimize_k = k;
      for(bool loop = true; loop && weights_epsilon < u; loop = cost < costs[k], u = upsilons[k])
        upsilons[k] = solver::minimize(solver_minimize_f, 0, u, u * 1e-2), costs[k] = solver_minimize_f(upsilons[k]);
      if(costs[k] < c_min)
        c_min = costs[k_min = k];
    }
    // Calculates some scales
    if(dumpName != "") {
      dumpAdd(s_printf("      'grads': [\n"));
      if(k_min != (unsigned int) -1)
        n_grads[k_min]++;
      double n_grad0 = 0;
      for(unsigned int k = 0; k < K_GRADS + K_SYLIN; k++) {
        double n_grad = 0, c_grad = 0;
        for(unsigned int nd = 0; nd < transform.getWeightCount(); nd++)
          n_grad += grads[k][nd] * grads[k][nd], c_grad += grads[0][nd] * grads[k][nd];
        n_grad = sqrt(n_grad), n_grad0 = k == 0 ? n_grad : n_grad0, c_grad /= (n_grad * n_grad0);
        if((k == k_min) || (k >= K_GRADS))
          dumpAdd(s_printf("        {%s 'n_grad': %6.1g, 'c_grad': %6.1g, 'cost': %7.2g, 'dcost': %7.1g, 'dw': %7.2g, 'da': %7.2g, 'min': %s}%s\n", (k < K_GRADS ? s_printf("'1st-order': %4d,", k).c_str() : s_printf("'2n-order': %4d,", k - K_GRADS).c_str()), n_grad, c_grad, costs[k], cost - costs[k], upsilons[k], upsilons[k], k == k_min ? "true " : "false", k < K_GRADS ? "," : ""));
      }
      dumpAdd(s_printf("      ],\n"));
    }
    // Updates the best estimate
    if(c_min < cost) {
      for(unsigned int n = 0, nd = 0; n < N; n++)
        for(unsigned int d = 1; d <= transform.getKernelDimension(n); d++, nd++)
          transform.setWeight(n, d, weights0[nd] - upsilons[k_min] * grads[k_min][nd]);
      cost = c_min;
      if(k_min < K_GRADS)
        upsilon = 2 * (upsilons[k_min] > weights_epsilon ? upsilons[k_min] : weights_epsilon);
      return true;
    }
  }
  // Optimisation fails
  set_weights(weights0[nd]);
  return false;
}
double network::KernelExperimentalEstimator::run(double criterion_epsilon, double weights_epsilon, unsigned int maxIterations, String dumpDirectory, String dumpHeader)
{
  run_init(weights_epsilon);
  // dumpString management
  {
    dumpName = dumpDirectory, dumpString = "", costs.clear();
    dumpAdd(s_printf("{" + (dumpHeader == "" ? "" : "\n  'estimator': " + dumpHeader + ",\n") + "  'trace': [\n    {  't':   0, 'order': 0, 'cost': %7.2g},\n", cost));
  }
  for(unsigned int nIterations = 1, loop = true; cost0 * criterion_epsilon < cost && (maxIterations == 0 || nIterations <= maxIterations) && loop; nIterations++) {
    dumpAdd(s_printf("    {  't': %3d, ", nIterations));
    loop = run_once(weights_epsilon);
    dumpAdd(s_printf("    },\n"));
#if 0
    if(dumpName != "") {
      BufferedInput activity(transform);
      activity.save(s_printf(dumpDirectory + "/activity/at-iteration-%d", nIterations), "gnuimg");
    }
#endif
  }
  costs.add(cost);
  // dumpString management
  {
    dumpAdd(s_printf("    {'done': true}\n  ],\n"));
    {
      double n_grad = 0;
      for(unsigned int k = 0; k < K_GRADS + K_SYLIN; k++)
        n_grad += n_grads[k];
      dumpAdd(s_printf("  'grads': [\n"));
      for(unsigned int k = 0; k < K_GRADS + K_SYLIN; k++)
        dumpAdd(s_printf("    %s: '%.1f%%'%s", (k < K_GRADS ? s_printf("'1st-order-%d'", k).c_str() : s_printf("'2nd-order-%d'", k - K_GRADS).c_str()), 100 * n_grads[k] / n_grad, k < K_GRADS ? ",\n" : "\n"));
      dumpAdd(s_printf(" ],\n"));
    }
    dumpAdd(s_printf("  'costs': %s\n}\n", costs.asString().c_str()));
  }
  // dumpString and dumpCosts management
  {
    if(dumpName != "") {
      costs.show(dumpDirectory + "/costs", false);
      s_save(dumpDirectory + "/logs.txt", dumpString);
#if 0
      system(("mencoder mf://" + dumpDirectory + "/activity/*.png -mf w=640:h=480:fps=2:type=png -ovc copy -oac copy -o " + dumpDirectory + "/activity.avi >/dev/null").c_str());
      system(("/bin/rm -rf " + dumpDirectory + "/activity").c_str());
#endif
    }
  }
  return cost;
}
network::KernelExperimentalEstimator *network::KernelExperimentalEstimator::solver_minimize_e = NULL;
