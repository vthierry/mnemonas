#include "CurveFit.hpp"

#include <cmath>
#include "s_printf.h"
#include "s_save.h"
#include "gnuplot.h"

/// @cond INTERNAL
CurveFit::CurveFit(const CurveFit& fit)
{
  c0 = fit.c0, c1 = fit.c1;
  for(unsigned int g = 0; g < G; g++) {
    T0[g] = fit.T0[g], T1[g] = fit.T1[g], T2[g] = fit.T2[g], C0[g] = fit.C0[g], C1[g] = fit.C1[g], D0[g] = fit.D0[g], CD0[g] = fit.CD0[g], DD0[g] = fit.DD0[g];
    for(unsigned int m = 0; m < 3; m++)
      bias[g][m] = fit.bias[g][m], gain[g][m] = fit.gain[g][m], decay[g][m] = fit.decay[g][m], error[g][m] = fit.error[g][m];
  }
  count = fit.count, igamma = fit.igamma, imode = fit.imode;
  values = fit.values;
}
CurveFit::CurveFit()
{
  clear();
}
constexpr double CurveFit::gammas[G];
///@endcond
void CurveFit::clear()
{
  c0 = c1 = NAN;
  for(unsigned int g = 0; g < G; g++) {
    lgammas[g] = log(gammas[g])/log(gammas[G-1]) * (1 - gammas[g]);
    T0[g] = T1[g] = T2[g] = C0[g] = C1[g] = D0[g] = CD0[g] = DD0[g] = 0;
    for(unsigned int m = 0; m < 3; m++) {
      bias[g][m] = gain[g][m] = decay[g][m] = error[g][m] = 0;
    }
  }
  count = 0, igamma = 0, imode = 0;
  values.clear();
}
void CurveFit::add(double c)
{
#define sqr(x) ((x)*(x))
  for(unsigned int g = 0; g < G; g++) {
    // Calculates the estimation momenta
    T2[g] = gammas[g] * (T0[g] + 2 * T1[g] + T2[g]);
    T1[g] = gammas[g] * (T0[g] + T1[g]);
    T0[g] = 1 + gammas[g] * T0[g];
    if (count > 0) {    
      C1[g] = gammas[g] * (C0[g] + C1[g]);
      C0[g] = c0 + gammas[g] * C0[g];
      if (count > 1) {
	D0[g] = c1 + gammas[g] * D0[g];
	CD0[g] = c0 * c1 + gammas[g] * CD0[g];
	DD0[g] = c1 * c1 + gammas[g] * DD0[g];
      }
    }
    // Estimates the constant value model parameters
    bias[g][0] = C0[g] / T0[g];
    error[g][0] = lgammas[g] * sqr(c - bias[g][0]) + gammas[g] * error[g][0];
    // Estimates the linear value model parameters
    double DT = T1[g] * T1[g] - T0[g] * T2[g];
    if (DT != 0) {
      gain[g][1] = (C1[g] * T0[g] - C0[g] * T1[g]) / DT;
      bias[g][1] = (T1[g] * C1[g] - T2[g] * C0[g]) / DT + gain[g][1];
      error[g][1] = lgammas[g] * sqr(c - bias[g][1]) + gammas[g] * error[g][1];
    }
    // Estimates the exponential value model parameters
    if (count > 1) {
      double DD = D0[g] * D0[g] - DD0[g] * T0[g];
      if (DD != 0) {
	double a = (C0[g] * D0[g] - CD0[g] * T0[g]) / DD;
	double b = (D0[g] * CD0[g] - DD0[g] * C0[g]) / DD;
	if (a != 1)
	  bias[g][2] = b / (1 - a);
	if (a > 0)
	  decay[g][2] = -1.0 / log(a);
	gain[g][2] = (c0 - bias[g][2])  * a;
	error[g][2] = lgammas[g] * sqr(c - (bias[g][2] + gain[g][2])) + gammas[g] * error[g][2];
      }
    }
    // Estimates the best model
    double emin = 1e100;
    igamma = 0, imode = 0;
    for(int g = G - 1; 0 <= g; g--)
      for(unsigned int m = 0; m < 3; m++)
	if (!std::isnan(error[g][m]) && 0 < error[g][m] && error[g][m] < emin)
	  emin = error[igamma = g][imode = m];
  }
  count++, c1 = c0, c0 = c;
  values.push_back(c);
}
std::string CurveFit::asString() const
{
#if 1
  std::string s;
  for(unsigned int g = 0; g < G; g++)
    for(unsigned int m = 0; m < 3; m++) 
      s += s_printf("g = %4.2g m = %d, bias = %g, gain = %g, decay = %g, error = %.2g\n", 
		    gammas[g], m, bias[g][m], gain[g][m], decay[g][m], error[g][m]);
#endif
  return s_printf("{ 'count' : %d, 'decay' : %g, 'gain' : %g, 'bias' : %g, 'error' : %g, 'gamma' : '%g', 'mode' : '%c'}\n%s", getCount(), getDecay(), getGain(), getBias(), getError(), getGamma(), getMode(), s.c_str());
}
void CurveFit::show(String file, bool show) const
{
  double cmin = 1e10;
  {
    std::string data;
    for(unsigned int i = 0; i < values.size(); i++) {
      data += s_printf("%d %g\n", i, values[i]);
      cmin = values[i] < cmin ? values[i] : cmin;
    }
    s_save(file + ".dat", data);
  }
  {
    std::string model =
      imode == 2 ? s_printf("c(x) = %g * exp(-(x - %d) / %g) + %g", getGain(), count, getDecay(), getBias()) :
      imode == 1 ? s_printf("c(x) = %g * (x - %d) + %g", getGain(), count, getBias()) :
      s_printf("c(x) = %g", getBias());
    gnuplot(file, (std::string) (0 < cmin && imode == 2 ? "set logscale y" : "") + "\nset format y '%.1e'\n" + model + "\nplot \"" + file + ".dat\" using 1:2 with lines linecolor \"black\" notitle, c(x) with lines linecolor \"red\" notitle", show);
  }
}
const Histogram CurveFit::getHistogram() const
{
  Histogram histogram;
  for(unsigned int i = 0; i < values.size(); i++)
    histogram.add(values[i]);
  return histogram;
}
