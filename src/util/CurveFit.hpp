#ifndef CURVEFIT_HPP
#define CURVEFIT_HPP

#include "util/numeric.hpp"
#include "util/Histogram.hpp"
#include "util/s_string.h"
#include <vector>

/** Fits an on-the-fly model on a time series.
 *  The present implementation considers a constant, affine or exponential model triplet of the a time series \f$c(t)\f$ writing :
 *  \f$\hat{c}(t) = \left\{ 0 | \nu \, t | \nu \, e^{-t/\tau} \right\} + \beta\f$
 * on a sliding exponential window of width \f$W=\frac{\log(1 - r)}{\log(\gamma)}\f$ where \f$r\f$ is the fraction of data average within this window, which is automatically adjusted.
 *
 * Implementation details:
 * - The model is chosen comparing the average prediction error on the last sample, while the estimation is performed on the previous samples.
 * - The time decay \f$\tau\f$ is estimated in the least-square sense on:
 * <center>\f$\min_{1/\tau, k} \sum_{t}^{T-1} \gamma^{T-t} \, (k - sg(c(t) - c(t-1)) \, t / \tau - \log(|c(t) - c(t-1)|))^2\f$</center>
 * - The bias \f$\beta\f$ and gain are computed using standard least-square criterion (\f$\tau\f$ being given in the exponential case).
 */
class CurveFit: public numeric {
private:
  static const unsigned int G = 13;
  static constexpr double gammas[G] = {0.10, 0.20, 0.30, 0.40,  0.53,  0.64,  0.73, 0.81,  0.87,  0.92,  0.96,  0.98,  0.99};
private:
  std::vector < double > values;
  mutable double c1, l1, T0[G], T1[G], T2[G], C0[G], C1[G], L0[G], L1[G], cmin, cmax, bias, gain, decay, error;
  mutable bool updated;
  void update() const;
public:
  /// @cond INTERNAL
  CurveFit(const CurveFit &fit);
  CurveFit();
  ///@endcond

  /** Clears the estimation. */
  void clear();

  /** Adds a value in sequence.
   * @param c The value to add in sequence, i.e., at time 0, 1, 2, ...
   */
  void add(double c);
  /** Gets the number of samples. */
  unsigned int getCount() const
  {
    return values.size();
  }
  /** Gets the exponential decay \f$\tau\f$ or NAN if undefined. */
  double getDecay() const
  {
    update();
    return decay;
  }
  /** Gets the best model gain \f$\nu\f$ or 0 NAN in undefined. */
  double getGain() const
  {
    update();
    return gain;
  }
  /** Gets the best model bias \f$\beta\f$. */
  double getBias() const
  {
    update();
    return bias;
  }
  /** Gets the best model average L1 error. */
  double getError() const
  {
    update();
    return error;
  }
  /** Returns the parameters as a JSON string. */
  std::string asString() const;

  /** Plots the observed and modeled curve in a file.
   * @param file The file name, a <a href="http://gnuplot.sourceforge.net">gnuplot</a> display data, script and image files are generated, with extensions <tt>.dat</tt>, <tt>.gnuplot.sh</tt> and <tt>.png</tt>.
   * @param show If true performs on the fly display.
   */
  void show(String file, bool show = false) const;
  /** Returns the value statistics. */
  const Histogram getHistogram() const;
};

#endif
