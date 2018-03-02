#include "CurveFit.hpp"

#include <cmath>
#include "solver.hpp"
#include "s_printf.h"
#include "s_save.h"
#include "gnuplot.h"
#define sqr(x) ((x) * (x))

/// @cond INTERNAL
CurveFit::CurveFit(const CurveFit& fit)
{
  for(unsigned int g = 0; g < G; g++) {
    for(unsigned int d = 0; d < 3; d++)
      P[g][d] = fit.P[g][d];
    for(unsigned int m = 0; m < 3; m++)
      bias[g][m] = fit.bias[g][m], gain[g][m] = fit.gain[g][m], decay[g][m] = fit.decay[g][m], error[g][m] = fit.error[g][m];
  }
  c0 = fit.c0, count = fit.count, igamma = fit.igamma, imode = fit.imode;
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
  for(unsigned int g = 0; g < G; g++) {
    lgammas[g] = log(gammas[g]) / log(gammas[G - 1]) * (1 - gammas[g]), tgammas[g] = 1;
    for(unsigned int d = 0; d < 3; d++)
      P[g][d] = 0;
    for(unsigned int m = 0; m < 3; m++)
      bias[g][m] = gain[g][m] = decay[g][m] = error[g][m] = 0;
  }
  c0 = 0;
  count = 0, igamma = 0, imode = 0;
  values.clear();
}
void CurveFit::add(double c)
{
  for(unsigned int g = 0; g < G; g++) {
    double p0 = gammas[g] * (P[g][0] - c0) + c0;
    P[g][2] = P[g][1], P[g][1] = P[g][0], P[g][0] = p0;
  }
  for(unsigned int g = 0; g < G; g++) {
    // Estimates the constant value model parameters
    if(count > 0) {
      bias[g][0] = P[g][0] / (1 - tgammas[g]);
      error[g][0] = lgammas[g] * sqr(c - bias[g][0]) + gammas[g] * error[g][0];
      // Estimates the linear value model parameters
      if(count > 1) {
        double
          M00 = (1 - tgammas[g]),
          M01 = (1 - tgammas[g] / gammas[g]),
          M10 = tgammas[g] * count - (1 - tgammas[g]) / (1 - gammas[g]),
          M11 = (tgammas[g] * count + (gammas[g] * (gammas[g] - 2) + tgammas[g]) / (1 - gammas[g])) / gammas[g],
          d = M00 * M11 - M01 * M10;
        if(d != 0) {
          gain[g][1] = (P[g][1] * M00 - P[g][0] * M01) / d;
          bias[g][1] = (P[g][0] * M11 - P[g][1] * M10) / d;
          error[g][1] = lgammas[g] * sqr(c - bias[g][1]) + gammas[g] * error[g][1];
        }
        // Estimates the exponential value model parameters
        if(count > 2) {
          // z = (1-g)*((g*p2-p1)-(g*p3-p2)) * c^2*cT + ((g*p3-p2)*gT+g^2*(p2-p3)) * c^2 + ((p1-g^2*p3)*gT-g^2*(p1-p3)) * c + g*(g*p2-p1)*gT+g^2*(p1-p2)
          double
          // g * p3 - p2
            gp3p2 = gammas[g] * P[g][2] - P[g][1],
          // g * p2 - p1
            gp2p1 = gammas[g] * P[g][1] - P[g][0],
          // g^2
            g2 = sqr(gammas[g]);
          // z = (((data[3] * pow(g / c, count) + data[2]) * c + data[1]) * c + data[0]) / (1 - c)
          double data[] = {
            // g * (g * p2 - p1) * gT + g^2 * (p1 - p2)
            gammas[g] * gp2p1 * tgammas[g] + g2 * (P[g][0] - P[g][1]),
            // (p1 - g^2 * p3) * gT - g^2 * (p1 - p3)
            (P[g][0] - g2 * P[g][2]) * tgammas[g] - g2 * (P[g][0] - P[g][2]),
            // (g * p3 - p2) * gT + g^2 * (p2 - p3)
            gp3p2 * tgammas[g] + g2 * (P[g][1] - P[g][2]),
            // (1 - g) * ((g * p2 - p1) - (g * p3 - p2))
            (1 - gammas[g]) * (gp2p1 - gp3p2),
            // g
            gammas[g]
          };
          solver_minimize_this = this;
          solver_minimize_this_data = data;
          double cdecay = solver::minimize(solver_minimize_this_f, 0, 10, 1e-12);
          if(cdecay > gammas[g]) {
            double
              cT = pow(gammas[g] / cdecay, count),
              M10 = (1 - gammas[g]) * (1 - cT) / (cdecay - gammas[g]),
              M11 = (1 - gammas[g]) * (1 / cdecay - cT / gammas[g]) / (cdecay - gammas[g]),
              d = M00 * M11 - M01 * M10;
            if(d != 0) {
              decay[g][2] = -1 / log(cdecay);
              gain[g][2] = (P[g][1] * M00 - P[g][0] * M01) / d;
              bias[g][2] = (P[g][0] * M11 - P[g][1] * M10) / d;
              error[g][2] = lgammas[g] * sqr(c - (bias[g][2] + gain[g][2])) + gammas[g] * error[g][2];
            }
          }
        }
      }
    }
    tgammas[g] *= gammas[g];
  }
  // Estimates the best model
  {
    double emin = 1e100;
    igamma = 0, imode = 0;
    for(int g = G - 1; 0 <= g; g--)
      for(unsigned int m = 0; m < 3; m++)
        if((m < count) && ((m < 2) || (0 < error[g][m])) && (error[g][m] < emin))
          emin = error[igamma = g][imode = m];
  }
  values.push_back(c0 = c), count++;
}
double CurveFit::solver_minimize_f(double c)
{
  // z = (((data[3] * cT + data[2]) * c + data[1]) * c + data[0]) / (1 - c)
  return fabs((((solver_minimize_this_data[3] * pow(solver_minimize_this_data[4] / c, count) + solver_minimize_this_data[2]) * c + solver_minimize_this_data[1]) * c + solver_minimize_this_data[0]) / (1 - c));
}
double CurveFit::solver_minimize_this_f(double c)
{
  return solver_minimize_this->solver_minimize_f(c);
}
double *CurveFit::solver_minimize_this_data = NULL;
CurveFit *CurveFit::solver_minimize_this = NULL;
std::string CurveFit::asString() const
{
  std::string s;
#if 1
  for(unsigned int m = 0; m < 3; m++)
    for(unsigned int g = 0; g < G; g++)
      s += s_printf("%sg = %4.2g m = %d, bias = %g, gain = %g, decay = %g, error = %.2g\n",
                    (g == 0 ? "\n" : ""), gammas[g], m, bias[g][m], gain[g][m], decay[g][m], error[g][m]);
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
