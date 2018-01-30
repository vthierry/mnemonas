#ifndef Density_hpp
#define Density_hpp

#include "util/numeric.hpp"
#include <random>

/** Defines a 1D real probabilty density.
 *  - This factory encapsulates random number generators.
 *   - All routines depends on setSeed. If setSeed(-1) -> it is a non-reproducible pseudo-random generator, else it generates always the same pseudo-random numbers.
 */
class Density: public numeric {
public:
  /// @cond INTERNAL
  virtual ~Density() {}
  ///@endcond

  /** Returns the probability for the given value.
   * @param x The input value.
   * @return A value in [0,1].
   */
  virtual double p(double x) const;
private:
  static std::mt19937_64 generator;
  static std::uniform_real_distribution < double > distribution;
public:
  /** Generates the seed of the pseudo random generator.
   * @param seed If seed == -1 changes randomly the seed, else generates a reproducible pseudo random sequence.
   */
  static void setSeed(int seed = -1);

  /** Generates a uniform random or pseudo random value.
   * - Samples a distribution
   * of mean \f$m = \frac{\min + \max}{2}\f$ and standard deviation \f$\sigma = \frac{|\max - \min|}{\sqrt{12}}\f$,
   * i.e., in the interval \f$[\min, \max] = [m - \sqrt{3} \, \sigma, m + \sqrt{3} \, \sigma] = m \pm 1.732 \, \sigma \f$
   * @param min Minimal value.
   * @param max Maximal value.
   * @return A pseudo random value drawn uniformly between min and max.
   */
  static double uniform(double min = 0, double max = 1);

  /** Generates an integer uniform random or pseudo random value.
   * @param min Minimal value.
   * @param max Maximal value.
   * @return A pseudo random value drawn uniformly in <tt>{min, max{</tt>,  i.e. between min and max excluded.
   */
  static int uniform(int min, int max);

  /** Gets a Gaussian random value.
   * @param mean The mean value.
   * @param stdev The standard-deviation value.
   * @return A random Gaussian variable.
   */
  static double gaussian(double mean = 0, double stdev = 1);

  /** Gets the Gaussian mean correspondign to a given bias.
   * @param bias The bias, i.e., the proportion of positive value, between 0 .. 1.
   * @param stdev The standard-deviation value.
   * @return The Gaussian mean corresponding to this bias.
   */
  static double getGaussianBias(double bias, double stdev = 1);

  /** Gets a Gamma distribution random value.
   * @param degree  Gamma degree (1 for Poisson).
   * @param rate Gamma rate 1 / tau > 0.
   * @return The inter-event interval d, such that t^n+1 = t^n + d
   */
  static double gamma(unsigned int degree = 1, double rate = 1);

  /** Gets an array of random true values, adjusting the random draw to get an exact number of true value.
   * @param size Array size.
   * @param count Number of true values.
   * @return The <tt>bool[size]</tt> buffer. To be deleted after use.
   */
  static bool *booleans(unsigned int size, unsigned int count);
private:
  // Precomputed table of Gaussian bias
  static const double gaussian_bias[];
  // Returns log(uniform()) avoiding -inf values
  static double loguniform();
};

#endif
