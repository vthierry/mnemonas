#include "ExponentialDecayFit.hpp"

#include <cmath>
#include "s_printf.h"
#include "s_save.h"
#include "gnuplot.h"

ExponentialDecayFit::ExponentialDecayFit(const ExponentialDecayFit& fit) : gamma(fit.gamma), n(fit.n), c1(fit.c1), T0(fit.T0), T1(fit.T1), T2(fit.T2), L0(fit.L0), L1(fit.L1), cmin(fit.cmin), bias(fit.bias)
{
  values = fit.values;
}
ExponentialDecayFit::ExponentialDecayFit(unsigned int window) : gamma(window == 0 ? 1 : pow(0.1, 1.0 / window))
{
  clear();
}
void ExponentialDecayFit::clear()
{
  c1 = NAN, n = T0 = T1 = T2 = L0 = L1 = 0, tau = NAN, cmin = NAN, bias = NAN;
  values.clear();
}
void ExponentialDecayFit::add(double c)
{
  if(std::isnan(c1) || (c != c1)) {
    if(!std::isnan(c1)) {
      double l = log(fabs(c1 - c));
      T2 = gamma * (T0 + 2 * T1 + T2);
      T1 = gamma * (T0 + T1);
      T0 = 1 + gamma * T0;
      L1 = gamma * (L0 + L1);
      L0 = l + gamma * L0;
      double tau_n = T0 * T2 - T1 * T1, tau_d = T0 * L1 - T1 * L0;
      if(tau_d > 0)
        tau = tau_n / tau_d;
    }
    values.push_back(c);
    cmin = std::isnan(cmin) || c < cmin ? c : cmin;
    bias = NAN;
    n++;
    c1 = c;
  }
}
double ExponentialDecayFit::getDecay() const
{
  return tau;
}
double ExponentialDecayFit::getBias() const
{
  double tau = getDecay();
  if(std::isnan(bias) && !std::isnan(tau)) {
    double k = 1 / (exp(1 / tau) - 1), g_t = 1, b_n_t = 0, b_d_t = 0;
    for(int i = values.size() - 1; 0 < i; i--) {
      double b = values[i] - k * (values[i - 1] - values[i]);
      if((0 < b) && (b < cmin))
        b_n_t += g_t * b, b_d_t += g_t;
      g_t *= gamma;
    }
    bias = b_d_t == 0 ? 0 : b_n_t / b_d_t;
  }
  return bias;
}
double ExponentialDecayFit::getEstimationTime(double c) const
{
  double b = getBias(), tau = getDecay();
  return std::isnan(b) || std::isnan(tau) ? NAN :
         b < c && b < c1 && c < c1 ? tau *log((c1 - b) / (c - b)) : 0;
}
std::string ExponentialDecayFit::asString() const
{
  return s_printf("{ 'count' : %.0f, 'gamma' : %.3g, 'decay' : %g, 'bias' : %g, 'minimal-value' : %g }", getCount(), gamma, getDecay(), getBias(), cmin);
}
void ExponentialDecayFit::show(String file, bool show) const
{
  {
    std::string data;
    for(unsigned int i = 0; i < values.size(); i++)
      data += s_printf("%d %g\n", i, values[i]);
    s_save(file + ".dat", data);
  }
  std::string s;
  {
    double tau = getDecay(), b = getBias();
    if(!(std::isnan(tau) || std::isnan(b))) {
      double a, g_t = 1, a_n_t = 0, a_d_t = 0;
      for(int i = values.size() - 1; 0 <= i; i--)
        if(values[i] > b)
          a_n_t += g_t * (log(values[i] - b) + i / tau), a_d_t += g_t, g_t *= gamma;
      a = exp(a_n_t / a_d_t);
      s = s_printf("c(x) = %g*exp(-x/%g)+%g\n", a, tau, b);
    }
  }
  gnuplot(file, "set logscale y\nset format y '%.1e'\n" + s + "plot \"" + file + ".dat\" using 1:2 with lines linecolor \"black\" notitle" + (s == "" ? "" : ", c(x) with lines linecolor \"red\" notitle"), show);
}
