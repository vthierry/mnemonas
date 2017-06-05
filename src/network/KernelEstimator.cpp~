#include "main.hpp"
#include "inc/s_printf.h"
#include "inc/s_save.h"

network::KernelEstimator::KernelEstimator(KernelTransform& transform) : transform(transform), N(transform.getN()), T((int) transform.getT()), R(transform.getR()), upsilon(0), epsilon(1), errors(new double[N * T]), kappas(new double[N * T]), betas_max(new double[N * T]), grad(new double[transform.getWeightCount()]), n_grad(0), w0(new double[transform.getWeightCount()]), w1(new double[transform.getWeightCount()])
{
  A = new double *[N];
  b = new double *[N];
  for(unsigned int n = 0; n < N; n++) {
    unsigned int D = transform.getKernelDimension(n);
    A[n] = new double[D * (D + 1) / 2];
    b[n] = new double[D];
  }
}
network::KernelEstimator::~KernelEstimator()
{
  delete[] errors;
  delete[] kappas;
  delete[] betas_max;
  delete[] grad;
  for(unsigned int n = 0; n < N; n++) {
    delete[] A[n];
    delete[] b[n];
  }
  delete[] A;
  delete[] b;
  delete[] w0;
  delete[] w1;
}
#define clear_buffer(buffer, size) for(unsigned int k = 0; k < size; buffer[k++] = 0) ;
#define set_weights(w_nd) \
  for(unsigned int n = 0, nd = 0; n < N; n++) \
    for(unsigned int d = 1; d <= transform.getKernelDimension(n); d++, nd++) \
      transform.setWeight(n, d, w_nd);
#define dumpAdd(s) \
  if(dumpName != "") { \
    dumpString += s; if(dumpName == "stdout") \
      printf("%s", s.c_str()); \
  }
double network::KernelEstimator::forward_simulation()
{
  double r = 0;
  transform.reset(true);
  for(unsigned int t = 0; t < T; t++)
    for(int n = N - 1; 0 <= n; n--)
      r += rho(n, t, transform.get(n, t));
  return r / (N * T);
}
double network::KernelEstimator::backward_sigmoid(double u, double b) const
{
  return b < 1e-3 ? 0 : epsilon < 5e-3 ? u : (u < 0 ? -1 : 1) * (1 + b * (1 - epsilon) / epsilon) * (1 - exp(-epsilon / b * abs(u)));
}
void network::KernelEstimator::backward_tuning()
{
  // Resets buffer
  {
    clear_buffer(grad, transform.getWeightCount());
    for(unsigned int n = 0; n < N; n++) {
      unsigned int D = transform.getKernelDimension(n);
      clear_buffer(A[n], D * (D + 1) / 2);
      clear_buffer(b[n], D);
    }
  }
  for(int t_ = T - 1; 0 <= t_; t_--) {
    // Backward error
    for(int n_ = N - 1; 0 <= n_; n_--) {
      unsigned int nt_ = n_ + N * t_;
      double e0 = drho(n_, t_, transform.get(n_, t_)), e1 = 0, e2 = 0, beta_max = 0;
      for(int t = t_; t_ <= t + (int) R && t < (int) T; t++)
        for(unsigned int n = t == t_ ? n_ + 1 : 0; n < N; n++) {
          unsigned int nt = n + N * t;
          double beta = transform.getKernelDerivative(n, 0, t, n_, t_);
          for(unsigned int d = 1; d <= transform.getKernelDimension(n); d++)
            beta += transform.getWeight(n, d) * transform.getKernelDerivative(n, d, t, n_, t_);
          beta_max = beta_max < fabs(beta) ? fabs(beta) : beta_max;
          e1 += beta * errors[nt];
          e2 += beta * kappas[nt];
        }
      betas_max[nt_] = beta_max;
      errors[nt_] = e0 + e1;
      kappas[nt_] = e0 + backward_sigmoid(e2, beta_max);
    }
    for(unsigned int n_ = 0, nd = 0, nt_ = N * t_; n_ < N; n_++, nt_++) {
      // Calculates gradient
      {
        for(unsigned int d = 1; d <= transform.getKernelDimension(n_); d++, nd++)
          grad[nd] += transform.getKernelValue(n_, d, t_) * errors[nt_];
      }
      // Calculates 2nd order system
      {
        double kappa_nt = 1;
        double b_nt = kappa_nt * (transform.get(n_, t_) - transform.getKernelValue(n_, 0, t_)) - errors[nt_];
        for(unsigned int d = 0, dd_ = 0; d < transform.getKernelDimension(n_); d++) {
          double phi_ndt = transform.getKernelValue(n_, d + 1, t_);
          for(unsigned int d_ = 0; d_ <= d; d_++, dd_++)
            A[n_][dd_] += kappa_nt * phi_ndt * transform.getKernelValue(n_, d_ + 1, t_);
          b[n_][d] += phi_ndt * b_nt;
        }
      }
    }
  }
  // Solves the 2nd order equation
  for(unsigned int n = 0, nd = 0; n < N; n++) {
    unsigned int D = transform.getKernelDimension(n);
    if(D > 0) {
      solver::cholesky(D, A[n], b[n], w1 + nd);
      nd += D;
    }
  }
  // Calculates some data for algorithme evaluation purpose
  if(dumpName != "") {
    {
      n_grad = 0;
      for(unsigned int nd = 0; nd < transform.getWeightCount(); nd++)
        n_grad += grad[nd] * grad[nd];
      n_grad = sqrt(n_grad);
    }
    // Calculates the cos(angle) between 1st order and 2order descent
    {
      double c_gdw = 0, n_dw = 0, k = pow(transform.getWeightCount(), -0.5);
      for(unsigned int n = 0, nd = 0; n < N; n++)
        for(unsigned int d = 1; d <= transform.getKernelDimension(n); d++, nd++) {
          double dw = w1[nd] - transform.getWeight(n, d);
          c_gdw -= grad[nd] * dw, n_dw += dw * dw;
        }
      n_dw = sqrt(n_dw);
      c_gdw /= n_grad * n_dw;
      dumpAdd(s_printf("'ngrad1' : %7.3g, 'ngrad2' : %7.3g, 'cgrad12' : %6.2g, ", k * n_grad, k * n_dw, c_gdw));
    }
    // Calculates some backward-tuning statistics
    if(false) {
      Histogram stat;
      for(unsigned int nt = 0; nt < N * T; stat.add(betas_max[nt++])) ;
      dumpAdd(s_printf("'beta max statistics' : %s, ", stat.asString("mean stdev min max", true).c_str()));
      stat.clear();
      for(unsigned int nt = 0; nt < N * T; stat.add(errors[nt++])) ;
      dumpAdd(s_printf("'error statistics' : %s, ", stat.asString("mean stdev min max", true).c_str()));
      stat.clear();
      for(unsigned int nt = 0; nt < N * T; stat.add(kappas[nt++])) ;
      dumpAdd(s_printf("'kappa statistics' : %s, ", stat.asString("mean stdev min max", true).c_str()));
    }
    dumpAdd(s_printf("'costs' : %s ", dumpCosts.asString().c_str()));
  }
}
double network::KernelEstimator::set_second_order_weights(double apsilon)
{
  set_weights(w0[nd] + apsilon * (w1[nd] - w0[nd]));
  return forward_simulation();
}
double network::KernelEstimator::solver_minimize_f(double x)
{
  return solver_minimize_e->set_second_order_weights(x);
}
unsigned int network::KernelEstimator::run_once(double weights_epsilon)
{
  cost = forward_simulation();
  backward_tuning();
  // Saves weights;
  for(unsigned int n = 0, nd = 0; n < N; n++)
    for(unsigned int d = 1; d <= transform.getKernelDimension(n); d++, nd++)
      w0[nd] = transform.getWeight(n, d);
#if 1
  // Attempts to improve at the 2nd order by a brent search in the 2nd order interval
  {
    solver_minimize_e = this;
    double apsilon = solver::minimize(solver_minimize_f, 0, 1, 5e-2), c = solver_minimize_f(apsilon);
    if(c < cost) {
      dumpAdd(s_printf("'order' : 2, 'cost' : %7.2g, 'dcost' : %7.2g, 'dw' : %7.2g},\n", c, cost - c, apsilon));
      cost = c;
      return 2;
    }
  }
#elif 1
  // Attempts to improve at the 2nd order finding the 1st acceotable solution
  {
    for(double apsilon = 1; weights_epsilon < apsilon; apsilon /= 2) {
      set_weights(w0[nd] + apsilon * (w1[nd] - w0[nd]));
      double c = forward_simulation();
      if(c < cost) {
        dumpAdd(s_printf("'order' : 2, 'cost' : %7.2g, 'dcost' : %7.2g, 'dw' : %7.2g},\n", c, cost - c, apsilon));
        cost = c;
        return 2;
      }
    }
  }
#endif
  // Attempts to improve at the 1st order using adaptive gradient
  {
    static const double w_epsilon_init = 1e-3;
    if(upsilon < weights_epsilon)
      upsilon = w_epsilon_init;
    while(weights_epsilon < upsilon) {
      set_weights(w0[nd] - upsilon * grad[nd]);
      double c = forward_simulation();
      if(c < cost) {
        dumpAdd(s_printf("'order' : 1, 'cost' : %7.2g, 'dcost' : %7.2g, 'dw' : %7.2g},\n", c, cost - c, upsilon * n_grad));
        cost = c;
        upsilon *= 2;
        return 1;
      } else
        upsilon /= 3;
    }
  }
  // Optimisation fails
  set_weights(w0[nd]);
  return 0;
}
double network::KernelEstimator::run(double criterion_epsilon, double weights_epsilon, unsigned int maxIterations, String dumpDirectory, String dumpHeader)
{
  cost = forward_simulation();
  // dumpString management
  {
    dumpName = dumpDirectory, dumpString = "", dumpCosts.clear();
    dumpAdd(s_printf("{" + (dumpHeader == "" ? "" : "\n 'estimator' : " + dumpHeader + ",\n") + " 'trace' : [\n {  't' :   0, 'order' : 0, 'cost' : %7.2g},\n", cost));
  }
  for(unsigned int nIterations = 1, order = 3; criterion_epsilon < cost && (maxIterations == 0 || nIterations <= maxIterations) && 0 < order; nIterations++) {
    {
      dumpAdd(s_printf(" {  't' : %3d: ", nIterations));
      dumpCosts.add(cost);
    }
    order = run_once(weights_epsilon);
  }
  // dumpString and dumpCosts management
  {
    dumpAdd(s_printf("  {'done' : true}\n ],\n 'costs' : %s\n}\n", dumpCosts.asString().c_str()));
    if(dumpName != "") {
      system(("mkdir -p " + dumpDirectory).c_str());
      dumpCosts.show(dumpDirectory + "/costs", false);
      s_save(dumpDirectory + "/logs.txt", dumpString);
    }
  }
  return cost;
}
network::KernelEstimator *network::KernelEstimator::solver_minimize_e = NULL;
