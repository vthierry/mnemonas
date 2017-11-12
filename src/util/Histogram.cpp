#include "Histogram.hpp"

#include "assume.h"
#include "s_printf.h"
#include "s_save.h"
#include "gnuplot.h"

#include <cmath>
#include <cfloat>

std::map < std::string, unsigned int > Histogram::names = {
  { "count", 0 },
  { "mean", 1 },
  { "stdev", 2 },
  { "skew", 3 },
  { "kurt", 4 },
  { "min", 5 },
  { "max", 6 },
  { "gamma-degree", 7 },
  { "gamma-rate", 8 },
  { "uniform-entropy", 9 },
  { "gaussian-entropy", 10 },
  { "hsize", 19 },
  { "density", 20 },
  { "mode", 21 },
  { "quantile", 22 },
  { "entropy", 23 },
  { "uniform-divergence", 24 },
  { "gaussian-divergence", 25 },
  { "gamma-divergence", 26 },
  { "best-model", 27 },
};
/// @cond INTERNAL
Histogram::Histogram(const Histogram& histogram) : m0(histogram.m0), m1(histogram.m1), m2(histogram.m2), m3(histogram.m3), m4(histogram.m4), min(histogram.min), max(histogram.max), histo(NULL), hsize(histogram.hsize), hmin(histogram.hmin), hmax(histogram.hmax), hscale(histogram.hscale), hcount(histogram.hcount), cdensity(NULL), changed(true), values(new double[names.size()]), model(NULL)
{
  if(hsize > 0) {
    histo = new unsigned int[hsize];
    for(unsigned int i = 0; i < hsize; i++)
      histo[i] = histogram.histo[i];
    cdensity = new double[hsize];
  }
}
///@endcond
Histogram::Histogram(unsigned int samples, double hmin_, double hmax_) : histo(NULL), cdensity(NULL), changed(true), values(new double[nnames]), model(NULL)
{
  m0 = m1 = m2 = m3 = m4 = 0, max = -DBL_MAX, min = -max;
  if((hsize = samples) > 0) {
    assume(hmin_ < hmax_, "numerical-error", "in Histogram::reset, incoherent bounds [%g %g]", hmin_, hmax_);
    hmin = hmin_, hmax = hmax_, hscale = hsize / (hmax - hmin);
    histo = new unsigned int[samples];
    cdensity = new double[samples];
  }
  clear();
}
/// @cond INTERNAL
Histogram::~Histogram()
{
  delete[] values;
  delete[] histo;
  delete[] cdensity;
  delete model;
}
///@endcond
void Histogram::update() const
{
  if(changed) {
    for(unsigned int i = 0; i < nnames; values[i++] = NAN) ;
    delete model;
    model = NULL;
    for(unsigned int j = 0, i = 0, s = 0; j < hsize; j++) {
      for(; i < hsize && s < j * hcount / hsize; s += histo[i++]) ;
      cdensity[j] = hmin + (i + 0.5) / hscale;
    }
    changed = false;
  }
}
void Histogram::clear()
{
  m0 = m1 = m2 = m3 = m4 = 0, max = -DBL_MAX, min = -max;
  if(hsize > 0) {
    for(unsigned int i = 0; i < hsize; histo[i++] = 0) ;
    hcount = 0;
  }
  changed = true;
}
void Histogram::add(double value)
{
  assume(!std::isnan(value), "numerical-error", "in Histogram::add adding a NAN value");
  double value2 = value * value;
  m0++, m1 += value, m2 += value2, m3 += value * value2, m4 += value2 * value2;
  min = value < min ? value : min, max = value > max ? value : max;
  // Here h[i] = int(d(hmin + u / hscale), u = i .. i+1))
  if((hsize > 0) && (hmin <= value) && (value < hmax)) {
    histo[(unsigned int) (hscale * (value - hmin))]++;
    hcount++;
    changed = true;
  }
}
double Histogram::get(String value, double v) const
{
  assume(0 < m0, "numerical-error", "in Histogram::get requires the %s value, but no value has been added", value.c_str());
  unsigned int ivalue = names[value];
  assume(ivalue < 20 || (0 < hsize && 0 < hcount), "numerical-error", "in Histogram::get requires the %s value, but the histogram is %s", value.c_str(), 0 == hsize ? "undefined" : "empty");
  // Cache mechanism
  if(std::isnan(v)) {
    update();
    if(!std::isnan(values[ivalue]))
      return values[ivalue];
  }
  double result;
  switch(ivalue) {
  // Momenta based estimations
  case 0:   // count
    result = m0;
    break;
  case 1:   // mean
    result = m1 / m0;
    break;
  case 2:   // stdev
  {
    double v = m0 == 1 ? 0 :
               (m2 - m1 * m1 / m0) / (m0 - 1);
    result = v < DBL_EPSILON ? 0 : sqrt(v);
    break;
  }
  case 3:   // skew
  {
    double s = get("stdev");
    // n / ((n-1)(n-2)) >_i (x_i-m)^3/s^3 : http://fr.wikipedia.org/wiki/Asymétrie_(statistique)
    result = s == 0 || m0 <= 2 ? 0 :
             (m0 / (m0 - 1) / (m0 - 2)) *
             // (x-m)^3/s^3
             (m3 - 3 * m2 * m1 / m0 + 2 * pow(m1, 3) / (m0 * m0)) / pow(s, 3);
    break;
  }
  case 4:   // kurt
  {
    double s = get("stdev");
    // n (n+1) / ((n-1)(n-2)(n-3))  >_i (x_i-m)^4/s^4 - 3 * (n-1)^2/((n-2)(n-3)) : http://fr.wikipedia.org/wiki/Kurtosis
    result = s == 0 || m0 <= 3 ? 0 :
             -3 * (m0 - 1) * (m0 - 1) / (m0 - 2) / (m0 - 3) + m0 * (m0 + 1) / (m0 - 1) / (m0 - 2) / (m0 - 3) *
             // >_i (x_i-m)^4/s^4
             (m4 - 4 * m3 * m1 / m0 + 6 * m2 * m1 * m1 / m0 / m0 - 3 * pow(m1, 4) / pow(m0, 3)) / pow(s, 4);
    break;
  }
  case 5:   // min
    result = min;
    break;
  case 6:   // max
    result = max;
    break;
  case 7:   // gamma-degree
  {
    double m = get("mean"), s = get("stdev");
    double n = m * m, d = s * s;
    result = d <= 0 ? 0 : n < d ? 1 : n / d;
    break;
  }
  case 8:   // gamma-rate
  {
    double m = get("mean"), s = get("stdev");
    result = s <= 0 ? 0 : s * s / m;
    break;
  }
  case 9:   // uniform-entropy
  {
    double s = get("stdev");
    result = s <= 0 ? 0 : log(2 * sqrt(3) * s) / log(2);
    break;
  }
  case 10:   // gaussian-entropy
  {
    double s = get("stdev");
    result = s <= 0 ? 0 : 0.5 * log(2 * M_PI * exp(1) * s) / log(2);
    break;
  }
  case 19:
    result = hsize;
    break;
  // Histogram based estimations
  case 21:   // mode
  {
    unsigned int m = 0;
    for(unsigned int i = 0, hmode = 0; i < hsize; i++)
      if(histo[i] > hmode)
        hmode = histo[m = i];
    double x = m + 0.5;
    // Maximum interpolation
    if((0 < m) && (m < hsize - 1)) {
      double d1 = 0.5 * ((double) histo[m + 1] - histo[m - 1]), d2 = 2.0 * histo[m] - histo[m - 1] - histo[m + 1];
      if((0 < d2) && (fabs(d1) < d2))
        x += d1 / d2;
    }
    result = hmin + x / hscale;
    break;
  }
  case 22:   // quantile
  {
    unsigned int i = 0, n1 = 0, n2 = 0;
    for(; i < hsize && n2 < v * hcount; n1 = n2, n2 += histo[i++]) ;
    // n1 = histo[i-1] < v * hcount <= n2 = histo[i] // 1st order interpolation
    result = hmin + (i - 1 + (n2 > n1 ? (v * hcount - n1) / (n2 - n1) : 0)) / hscale;
    break;
  }
  case 20:   // density
    result = hmin <= v && v < hmax ? hscale * histo[(unsigned int) (hscale * (v - hmin))] / hcount : 0;
    break;
  case 23:   // entropy
  {
    double h = 0;
    for(unsigned int i = 0; i < hsize; i++) {
      double p = histo[i];
      if(p > 0)
        h -= p * log(p);
    }
    result = (h / hcount + log(hcount / hscale)) / log(2);
    break;
  }
  case 24:   // uniform divergence
    result = getDivergence(getDensityModel("uniform"), !std::isnan(v) && v < 0);
    break;
  case 25:   // gaussian divergence
    result = getDivergence(getDensityModel("gaussian"), !std::isnan(v) && v < 0);
    break;
  case 26:   // gamma divergence
    result = getDivergence(getDensityModel("gamma"), !std::isnan(v) && v < 0);
    break;
  case 27:    // best model
  {
    double gaussian = get("gaussian-divergence");
    double gamma = get("gamma-divergence");
    double uniform = get("uniform-divergence");
    result = 2 * gaussian < uniform && gaussian < gamma ? 1 :
             2 * gamma < uniform ? 2 :
             0;
    break;
  }
  default:
    assume(false, "illegal-argument", "in Histogram::get undefined value '%s'", value.c_str());
    result = NAN;
    break;
  }
  return values[ivalue] = result;
}
double Histogram::getDivergence(const random::Density& density, bool inverse) const
{
  assume(0 < hsize && 0 < hcount, "numerical-error", "in Histogram::get requires the %s value, but the histogram is %s", 0 == hsize ? "undefined" : "empty");
  double div = 0, s_p = 0, s_q = 0;
  for(unsigned int i = 0; i < hsize; i++) {
    double p = histo[i], q = density.p(hmin + (i + 0.5) / hscale);
    if((p > 0) && (q > 0)) {
      if(inverse)
        div += q * log(q / p), s_p += p, s_q += q;
      else
        div += p * log(p / q), s_p += p, s_q += q;
    }
  }
  return (inverse ? div / s_q - log(s_q / s_p) : div / s_p - log(s_p / s_q)) / log(2);
}
double Histogram::draw() const
{
  assume(0 < hsize, "numerical-error", "in Histogram::getDivergence requires an histogram to be defined");
  update();
  return cdensity[random::uniform(1, hsize - 1)];
}
const random::Density& Histogram::getDensityModel(String model_) const
{
  if(model_ == "gaussian") {
    class GaussianDensity: public random::Density {
      double m, s, k;
public:
      GaussianDensity(double mean, double stdev) : m(mean), s(stdev), k(s * sqrt(2.0 * M_PI)) {}
      double p(double x) const
      {
        if(0 < s) {
          double u = (x - m) / s;
          return exp(-0.5 * u * u) / k;
        } else
          return 1;
      }
    };
    delete model;
    model = new GaussianDensity(get ("mean"), get ("stdev"));
  } else if(model_ == "gamma") {
    class GammaDensity: public random::Density {
      double d, t, k;
public:
      GammaDensity(double degree, double rate) : d(degree), t(rate), k(t * tgamma(d)) {}
      double p(double x) const
      {
        return 0 < t ? pow(x / t, d - 1) * exp(-x / t) / k : 1;
      }
    };
    delete model;
    model = new GammaDensity(get ("gamma-degree"), get ("gamma-rate"));
  } else if(model_ == "uniform") {
    double m = get("mean"), s = get("stdev");
    class UniformDensity: public random::Density {
      double min, max;
public:
      UniformDensity(double min, double max) : min(min), max(max) {
        assume(min <= max, "numerical-error", "in Histogram::get::UniformDensity incoherent uniform bounds [%g %g]", min, max);
      }
      double p(double x) const
      {
        return min < max ? 1 / (max - min) : 1;
      }
    };
    delete model;
    model = new UniformDensity(m - sqrt (3) *s, m + sqrt (3) *s);
  } else if(model_ == "automatic")
    switch((unsigned int) get("best-model")) {
    case 1:
      return getDensityModel("gaussian");
    case 2:
      return getDensityModel("gamma");
    default:
      return getDensityModel("uniform");
    }
  else
    assume(false, "illegal-argument", "in Histogram::getModel undefined model '%s'", model_.c_str());
  return *model;
}
std::string Histogram::asString(String what_, bool oneline) const
{
  std::string json;
  if(m0 == 0) {
    json = "{ 'count' : 0}";
    return json;
  }
  std::string what = " " + what_ + " ", next1 = oneline ? "" : "\n", next2 = oneline ? "," : ",\n";
  json = "{";
  // Here defined a map with the keys in the index order
  std::map < unsigned int, std::string > keys;
  for(std::map < std::string, unsigned int > ::const_iterator i = names.begin(); i != names.end(); ++i)
    keys[i->second] = i->first;
  for(std::map < unsigned int, std::string > ::const_iterator i = keys.begin(); i != keys.end(); ++i)
    if(((what_ == "") || (what.find(" " + i->second + " ") != std::string::npos)) &&
       ((i->first < 20) || (0 < hsize)))
    {
      if((i->second != "density") && (i->second != "quantile") &&
         (i->second != "uniform-divergence") && (i->second != "gaussian-divergence") && (i->second != "gamma-divergence") && (i->second != "best-model"))
        json += s_printf(next1 + " '" + i->second + "' : %g", get(i->second));
      else if(i->second == "quantile")
        json += s_printf(next1 + " 'median' : %g", get(i->second, 0.5))
                + s_printf(next2 + " 'quartile-0.25' : %g", get(i->second, 0.25))
                + s_printf(next2 + " 'quartile-0.75' : %g", get(i->second, 0.75));
      else if((i->second == "uniform-divergence") || (i->second == "gaussian-divergence") || (i->second == "gamma-divergence"))
        json += s_printf(next1 + " '" + i->second + "' : %g", get(i->second))
                + s_printf(next2 + " 'inverse-" + i->second + "' : %g", get(i->second, -1));
      else if(i->second == "best-model") {
        unsigned int k = (unsigned int) get(i->second);
        json += s_printf(next1 + " '" + i->second + "' : '%s'",
                         k == 1 ? "gaussian" : k == 2 ? "gamma" : "uniform");
      }
      next1 = next2;
    }
  json += s_printf(oneline ? "}" : "\n}", hsize);
  return json;
}
void Histogram::plot(String file, String model_, bool show)
{
  assume(0 < hsize && 0 < hcount, "numerical-error", "in Histogram::plot the histogram is %s", 0 == hsize ? "undefined" : "empty");
  if(model_ != "")
    getDensityModel(model_);
  {
    std::string data;
    for(unsigned int i = 0; i < hsize; i++)
      data += (model_ == "" ?
               s_printf("%d %d\n", i, histo[i]) :
               s_printf("%d %d %d\n", i, histo[i], (int) (hcount / hscale * model->p(hmin + (i + 0.5) / hscale))));
    s_save(file + ".dat", data);
  }
  gnuplot(file, "set yrange [0:*] writeback\nplot \"" + file + ".dat\" using 1:2 with boxes linecolor \"black\" notitle" + (model_ == "" ? "\n" : ", \"" + file + ".dat\" using 1:3 with lines linecolor \"red\" linewidth 2 notitle\n"), show);
}
