#include <cmath>
#include <map>

#include "random.hpp"

/** Defines an 1D statistics on a set of scalar values. */
class Histogram: public random::Density  {
  static std::map < std::string, unsigned int > names;
  static const unsigned int nnames = 32;
  double m0, m1, m2, m3, m4, min, max;
  unsigned int *histo, hsize;
  double hmin, hmax, hscale, hcount;
  mutable double *cdensity;
  void update() const;
  mutable bool changed;
  mutable double *values;
  mutable random::Density *model;
public:
  Histogram(const Histogram &histogram);

  /** Defines an histogram for a given observation.
   * @param samples The histogram size (if 0 there is no histogram, only momenta calculation).
   * @param min The minimal expected value.
   * @param max The maximal expected value.
   */
  Histogram(unsigned int samples = 0, double min = NAN, double max = NAN);
  ~Histogram();
  /** Clears all added values. */
  void clear();
  /** Adds a value to the statistics. */
  void add(double value);

  /** Gets a statistical value.
   * @param value :
   * As a function of the sample momenta:
   * - "count" : the number of observed values.
   * - "min"   : the observed minimal value.
   * - "max"   : the observed maximal value.
   * - "mean"  : the mean value.
   * - "stdev" : the standard-deviation \f$\sigma\f$. (2nd order).
   *   - Returns an unbiased estimator of the theoretical standard-deviation <tt>E[(x-m)^2]^(1/2)</tt> (Any standard-deviation lower than DBL_EPSILON (1e-15) is set to 0).
   * - "skew"  : the skewness (3rd order).
   *   -  Returns an estimator of the theoretical skewness <a href="http://en.wikipedia.org/wiki/Skewness"> skewness</a> <tt>E[(x - m)^3 / stdev^3]</tt>, given by <tt>n / ((n-1)(n-2)) >_i (x_i-m)^3/s^3</tt> (Skewness is zero for symmetric distributions).
   * - "kurt"  : the kurtosis (4th order).
   *   - An estimator of the theoretical kurtosis <a href="http://en.wikipedia.org/wiki/Kurtosis">kurtosis</a> <tt>E[(x - m)^4 / stdev^4] - 3</tt>, given by <tt>n (n+1) / ((n-1)(n-2)(n-3))  >_i (x_i-m)^4/s^4 - 3 * (n-1)^2/((n-2)(n-3))</tt> (Kurtosis is equal to 0 for the Gaussian distribution).
   * - "gamma-degree" : the degree of a Gamma distribution, with the same mean and variance as the empirical distribution.
   *   - Returns the degree <tt>d >= 1</tt> or <tt>0</tt> if undefined, assuming that the probability distribution has the form <tt>p(t) = (t / tau)^(d - 1) exp(-t / tau) / (tau d!)</tt> e.g., <tt>d = 1</tt> for a Poisson distribution.
   * - "gamma-rate"   : the rate of a Gamma distribution, with the same mean and variance as the empirical distribution.
   *   - Returns the rate <tt>tau >= 1</tt> or <tt>0</tt> if undefined, assuming a Gamma distribution as above.
   * - "uniform-entropy" : the entropy of a uniform distribution with the same mean and variance.
   * - "gaussian-entropy" : the entropy of a Gaussian distribution with the same mean and variance.
   * As a function of the histogram:
   * - "hsize" : the histogram sampling size.
   * - "density" : the observed probability for value <tt>v</tt>. It returns the value of <tt>Histogram::p()</tt>.
   * - "mode" : the mode value, i.e. the most frequent value.
   * - "quantile" : The median corresponds to the quantile for v = 0.5. The quartiles to the quantiles for v = 0.25 and 0.75.
   * - "entropy" : The distribution entropy, in bits, i.e. using \f$\log_2\f$.
   *   - We use the approximation: \f$ h \sim - \sum_{i} p_i \log p_i + log \epsilon \f$, where \f$p_i\f$ is the probability of the \f$i\f$-th box of size \f$\epsilon\f$.
   * - "uniform-divergence" : The Kullback-Leibler divergence between the empirical distribution and a uniform distribution having the same mean and variance. If <tt>v < 0</tt> computes the inverse divergence.
   * - "gaussian-divergence" : The Kullback-Leibler divergence between the empirical distribution and a Gaussian distribution having the same mean and variance. If <tt>v < 0</tt> computes the inverse divergence.
   * - "gamma-divergence" : The Kullback-Leibler divergence between the empirical distribution and a Gamma distribution having the same mean and variance. If <tt>v < 0</tt> computes the inverse divergence.
   * - "best-model" Estimates the best model comparing divergences:
   *   - 1 : "gaussian",
   *   - 2 : "gamma",
   *   - 0 : "uniform" as a fallback option.
   * @param v An optional additional parameter depending of the defined value.
   */
  double get(String value, double v = NAN) const;
  double p(double x) const
  {
    return get("density", x);
  }
  /** Draws a sample from the present statistics. */
  double draw() const;

  /** Gets the Kullback-Leibler divergence between this empirical distribution and another probality density.
   *   - The <a href="http://en.wikipedia.org/wiki/Kullback-Leibler_divergence">Kullback–Leibler divergence</a> <tt>d(p||q) = >_w p ln(p/q)</tt>, where <tt>p</tt> is the empirical distribution and <tt>q</tt> the model distribution corresponds to the average number of bits difference when coding <tt>p</tt> with <tt>q</tt>.
   * @param density The other distribution density.
   * @param inverse
   *   - If false <tt>p</tt> is this distribution, empirical, and <tt>density</tt> is the <tt>q</tt> model distribution.
   *   - If true <tt>q</tt> is this distribution, the model, and <tt>density</tt> is the <tt>p</tt>empirical distribution.
   * @return The Kullback-Leibler divergence in bits.
   */
  double getDivergence(const random::Density& density, bool inverse = false) const;

  /** Gets a theoretical density with the same 1st and 2nd order parameters as this one.
   * @param model Either <tt>uniform</tt>, <tt>gaussian</tt>, <tt>gamma</tt> or <tt>automatic</tt> (i.e. automatically choose the best model among the others).
   */
  const random::Density& getDensityModel(String model = "automatic") const;

  /** Returns all statistical values as a JSON string.
   * @param what The list of value to return, by default all values are returned.
   * @param oneline If true returns the result a 1D line, else returns a 2D format.
   */
  std::string asString(String what = "", bool oneline = false) const;

  /** Plots the histogram in a file.
   * @param file The plot file name, a <a href="http://gnuplot.sourceforge.net">gnuplot</a> display data, script and image files are generated, with extensions <tt>.dat</tt>, <tt>.gnuplot.sh</tt> and <tt>.png</tt>.
   * @param model If not empty, also draw the corresponding model, either <tt>uniform</tt>, <tt>gaussian</tt>, <tt>gamma</tt> or <tt>automatic</tt> (i.e. automatically choose the best model among the others).
   * @param show If true performs on the fly display.
   */
  void plot(String file, String model = "", bool show = false);
};
