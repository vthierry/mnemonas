#include "CurveFit.hpp"

#include <cmath>
#include "s_printf.h"
#include "s_save.h"
#include "gnuplot.h"

/// @cond INTERNAL
CurveFit::CurveFit(const CurveFit& fit) : gamma(fit.gamma), c1(fit.c1), l1(fit.l1), T0(fit.T0), T1(fit.T1), T2(fit.T2), C0(fit.C0), C1(fit.C1), L0(fit.L0), L1(fit.L1), cmin(fit.cmin), cmax(fit.cmax), updated(false)
{
  values = fit.values;
}
///@endcond
CurveFit::CurveFit(unsigned int window) : gamma(window == 0 ? 1 : pow(0.1, 1.0 / window))
{
  clear();
}
void CurveFit::clear()
{
  values.clear();
  c1 = l1 = cmin = cmax = bias = gain = decay = error = NAN, T0 = T1 = T2 = C0 = C1 = L0 = L1 = 0;
  updated = false;
}
void CurveFit::add(double c)
{
  T2 = gamma * (T0 + 2 * T1 + T2);
  T1 = gamma * (T0 + T1);
  T0 = 1 + gamma * T0;
  C1 = gamma * (C0 + C1);
  C0 = c + gamma * C0;
  L1 = gamma * (L0 + L1);
  double d = std::isnan(c1) ? 0 : fabs(c1 - c), l = d > 0 ? log(d) : std::isnan(l1) ? 0 : l1;
  L0 = l + gamma * L0;
  cmin = std::isnan(cmin) || c < cmin ? c : cmin;
  cmax = std::isnan(cmax) || c > cmax ? c : cmax;
  values.push_back(c);
  c1 = c;
  l1 = l;
  updated = false;
}
void CurveFit::update() const
{
  if(updated)
    return;
  updated = true;
  // Estimates the constant value model parameters
  {
    // Offset calculation
    bias = T0 > 0 ? C0 / T0 : 0, gain = decay = NAN;
    // Error estimation
    error = 0;
    for(unsigned int i = 0; i < values.size(); i++)
      error = fabs(values[i] - bias) + gamma * error;
  }
  // Estimates the linear value model parameters
  double T0211 = T0 * T2 - T1 * T1;
  {
    if(T0211 > 0) {
      // Gain and offset calculation
      double nu = (C0 * T1 - C1 * T0) / T0211, beta = (T2 * C0 - T1 * C1) / T0211;
      // Error estimation
      double err = 0;
      for(unsigned int i = 0; i < values.size(); i++) {
        double t = 1 - (int) (values.size() - i);
        err = fabs(values[i] - (beta + nu * t)) + gamma * err;
      }
      if(err < error / 2)
        error = err, bias = beta, gain = nu;
    }
  }
  // Calculates the exponential model parameters
  {
    // Decay calculation
    double tau_d = T0 * L1 - T1 * L0, tau = tau_d != 0 ? T0211 / tau_d : NAN;
    if(!std::isnan(tau) && (T0211 > 0)) {
      // Gain and Bias calculation
      double E1 = 0, E2 = 0, CE1 = 0;
      for(unsigned int i = 0; i < values.size(); i++) {
        double t = i, e = exp(-t / tau), c = values[i];
        E1 = e + gamma * E1;
        E2 = e * e + gamma * E2;
        CE1 = c * e + gamma * CE1;
      }
      double E0211 = T0 * E2 - E1 * E1;
      if(E0211 > 0) {
        double nu = (CE1 * T0 - C0 * E1) / E0211, beta = (E2 * C0 - E1 * CE1) / E0211;
        // Error estimation
        double err = 0;
        for(unsigned int i = 0; i < values.size(); i++) {
          double t = i;
          err = fabs(values[i] - (beta + nu * exp(-t / tau))) + gamma * err;
        }
        if(err < error / (std::isnan(gain) ? 3 : 1.5))
          error = err, bias = beta, gain = nu, decay = tau;
      }
    }
  }
}
std::string CurveFit::asString() const
{
  return s_printf("{ 'count' : %d, 'gamma' : %.3g, 'decay' : %g, 'gain' : %g, 'bias' : %g, 'min' : %g, 'max' : %g, 'model' : '%s', 'error' : %g }", getCount(), gamma, getDecay(), getGain(), getBias(), cmin, cmax, !std::isnan(decay) ? "exponential" : !std::isnan(gain) ? "affine" : "constant", gain, bias);
}
void CurveFit::show(String file, bool show) const
{
  {
    std::string data;
    for(unsigned int i = 0; i < values.size(); i++)
      data += s_printf("%d %g\n", i, values[i]);
    s_save(file + ".dat", data);
  }
  {
    std::string model =
      !std::isnan(decay) ? s_printf("c(x) = %g * exp(-x / %g) + %g", gain, decay, bias) :
      !std::isnan(gain) ? s_printf("c(x) = %g * x + %g", gain, bias) :
      s_printf("c(x) = %g", bias);
    gnuplot(file, (std::string) (std::isnan(getDecay()) ? "" : "set logscale y") + "\nset format y '%.1e'\n" + model + "\nplot \"" + file + ".dat\" using 1:2 with lines linecolor \"black\" notitle, c(x) with lines linecolor \"red\" notitle", show);
  }
}
const Histogram CurveFit::getHistogram() const
{
  Histogram histogram;
  for(unsigned int i = 0; i < values.size(); i++)
    histogram.add(values[i]);
  return histogram;
}
