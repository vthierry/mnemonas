#include "CurveFit.hpp"

#include <cmath>
#include "s_printf.h"
#include "s_save.h"
#include "gnuplot.h"

/// @cond INTERNAL
CurveFit::CurveFit(const CurveFit& fit)
{
  for(unsigned int g = 0; g < G; g++) {
    for(unsigned int d = 0; d < 3; d++)
      P[g][d] = fit.P[g][d] ;
    for(unsigned int m = 0; m < 3; m++)
      bias[g][m] = fit.bias[g][m], gain[g][m] = fit.gain[g][m], decay[g][m] = fit.decay[g][m], error[g][m] = fit.error[g][m];
  }
  c0 = fit.c0, c1 = fit.c1;
  count = fit.count, igamma = fit.igamma, imode = fit.imode;
  values = fit.values;
  updated = fit.updated;
}
CurveFit::CurveFit()
{
  clear();
}
constexpr double CurveFit::gammas[G];
///@endcond
void CurveFit::clear()
{
  for(unsigned int g = 0; g < G; g++) {
    lgammas[g] = log(gammas[g]) / log(gammas[G - 1]) * (1 - gammas[g]), tgammas[g] = gammas[g];
    for(unsigned int d = 0; d < 3; d++)
      P[g][d] = 0;
    for(unsigned int m = 0; m < 3; m++)
      bias[g][m] = gain[g][m] = decay[g][m] = error[g][m] = 0;
  }
  c0 = c1 = 0;
  count = 0, igamma = 0, imode = 0;
  values.clear();
  updated = false;
}
void CurveFit::add(double c)
{
  for(unsigned int g = 0; g < G; g++) {
    double p0 = gammas[g] * (P[g][0] - c1) + c1;
    tgammas[g] *= gammas[g];
    P[g][2] = P[g][1], P[g][1]= P[g][0], P[g][0] = p0;
  }
  c1 = c0, c0 = c, count++;
  values.push_back(c);
  updated = false;
}
void CurveFit::update()
{
#define sqr(x) ((x)*(x))
  if(updated) return; updated = true;
  for(unsigned int g = 0; g < G; g++) {
    // Estimates the constant value model parameters
    bias[g][0] = P[g][0] / (1 - tgammas[g]);
    error[g][0] = lgammas[g] * sqr(c0 - bias[g][0]) + gammas[g] * error[g][0];
    
    // Estimates the linear value model parameters
    double 
      M00 = (1 - tgammas[g]), 
      M01 = (1 - gammas[g] * tgammas[g]), 
      M10 = ((2 - 3 * gammas[g]) * tgammas[g] - 1) / (1 - gammas[g]),
      M11 = (gammas[g] * (4 - 5 * gammas[g]) * tgammas[g] - (2 - gammas[g])) / (1 - gammas[g]),
      d = M00 * M11 - M01 * M10;
    if (d != 0) {
      gain[g][1] = (P[g][1] * M00 - P[g][0] * M01) / d;
      bias[g][1] = (P[g][0] * M11 - P[g][1] * M10) / d + gain[g][1];
      error[g][1] = lgammas[g] * sqr(c0 - bias[g][1]) + gammas[g] * error[g][1];
    }
    // Estimates the exponential value model parameters
    if (P[g][0] != P[g][1]) {
      double cdecay0 = (P[g][1] - P[g][2]) / (P[g][0] - P[g][1]), cdecay = exp(-1/3141.6), cdecayT = pow(cdecay, count);
      /*
	../..
      */
      double k_m = (1 - gammas[g]) * cdecay / (1 - gammas[g] * cdecay);
      M01 = k_m * (1 - tgammas[g] * cdecayT), M10 = k_m * cdecay * (1 - gammas[g] * tgammas[g] * cdecay * cdecayT), d = M00 * M11 - M01 * M10;
      if (d != 0) {
	decay[g][2] = -1 / log(cdecay);
	gain[g][2] = (P[g][1] * M00 - P[g][0] * M01) / d;
	bias[g][2] = (P[g][0] * M11 - P[g][1] * M10) / d + gain[g][1];
	error[g][2] = lgammas[g] * sqr(c0 - (bias[g][2] + gain[g][2])) + gammas[g] * error[g][2];
	printf(" >> g = %g bias = %g gain = %g decay = %g -> %g error = %g\n", gammas[g], bias[g][2], gain[g][2], 1 / log(cdecay0), decay[g][2], error[g][2]);
      }

    }
  }

  // Estimates the best model
  double emin = 1e100;
  igamma = 0, imode = 0;
  for(int g = G - 1; 0 <= g; g--)
    for(unsigned int m = 0; m < 3; m++)
      if(!std::isnan(error[g][m]) && (0 < error[g][m]) && (error[g][m] < emin))
	emin = error[igamma = g][imode = m];
}
std::string CurveFit::asString() const
{
  std::string s;
#if 0
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
