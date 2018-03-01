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
 */
class CurveFit: public numeric {
private:
  static const unsigned int G = 13;
  static constexpr double gammas[G] = { 0.10, 0.20, 0.30, 0.40, 0.53, 0.64, 0.73, 0.81, 0.87, 0.92, 0.95, 0.98, 0.99 };
private:
  double lgammas[G], tgammas[G], c0, c1, P[G][3], bias[G][3], gain[G][3], decay[G][3], error[G][3];
  unsigned int count, igamma, imode;
  std::vector < double > values;
  void update();
  bool updated;
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
    return count;
  }
  /** Gets the exponential decay \f$\tau\f$ or NAN if undefined. */
  double getDecay() const
  {
    const_cast<CurveFit*>(this)->update();
    return decay[igamma][imode];
  }
  /** Gets the best model gain \f$\nu\f$ or 0 NAN in undefined. */
  double getGain() const
  {
    const_cast<CurveFit*>(this)->update();
    return gain[igamma][imode];
  }
  /** Gets the best model bias \f$\beta\f$. */
  double getBias() const
  {
     const_cast<CurveFit*>(this)->update();
    return bias[igamma][imode];
  }
  /** Gets the best model average L1 error. */
  double getError() const
  {
    const_cast<CurveFit*>(this)->update();
    return error[igamma][imode];
  }
  /** Gets the best model gamma filtering. */
  double getGamma() const
  {
    const_cast<CurveFit*>(this)->update();
    return gammas[igamma];
  }
  /** Gets the best model mode.
   * - Returns 'c' for constant model, 'a' for an affine model and 'e' for an exponential model.
   */
  char getMode() const
  {
    const_cast<CurveFit*>(this)->update();
    return imode == 2 ? 'e' : imode == 1 ? 'a' : 'c';
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
