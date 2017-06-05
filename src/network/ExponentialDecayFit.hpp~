/** Fits an exponential decay model on a time series.
 * - The model is of the form \f$c(t) = a \, e^{-t/\tau} + \beta\f$ and, in the least-square sense:
 *   - The time decay \f$\tau\f$ is fitted on \f$log(c(t) - c(t-1)) = k' - t / \tau\f$.
 *   - The bias \f$\beta\f$ is fitted on \f$c(t) = k'' \, (c(t) - c(t-1)) + b\f$.
 * - The least-square problem for \f$\tau\f$ writes:
 * <center>\f$\min_{1/\tau, k} \sum_{t}^T \gamma^{T-t} \, (k - t / \tau - log(c(t) - c(t-1)))^2\f$</center>
 * for an exponential window of width \f$W=\frac{\log(1 - r)}{\log(\gamma)}\f$ where \f$r\f$ is the fraction of data average within this window (typically 90%) and the bias is estimated as:
 * <center>\f$\min_{b} \sum_{t=1}^T \, \delta_{0 < \hat{b}(t) < \min_t c(t)} \, \gamma^{T-t} \, (b - b(t))^2, \;\;\; \hat{b}(t) \stackrel{\rm def}{=} c(t) - \frac{c(t-1) - c(t)}{e^\frac{1}{\tau}-1}\f$,</center>
 *  while to plot the model we also estimate:
 * <center>\f$\min_{\log(a)} \sum_{t=1}^T \, \gamma^{T-t} \, (\log(a) - \log(\hat{a})(t))^2, \;\;\; \log(\hat{a})(t) \stackrel{\rm def}{=} \log(c(t) - b) + \frac{t}{\tau}\f$,</center>
 */
class ExponentialDecayFit {
private:
  mutable double gamma, n, c1, T0, T1, T2, L0, L1, cmin, bias;
  std::vector < double > values;
public:
  ExponentialDecayFit(const ExponentialDecayFit &fit);

  /** Constructs an exponential decay interpolator.
   * @param window The sampling window containing 90% of the least-square average.
   */
  ExponentialDecayFit(unsigned int window = 10);
  /** Clears the estomation. */
  void clear();

  /** Adds a value in sequence.
   * @param c The value to add in sequence, i.e., at time 0, 1, 2, ...
   * @return The estimation decay stability \f$\tau_t - \tau_{t-1}\f$ in number of samples.
   */
  double add(double c);
  /** Gets the number of samples. */
  double getCount() const
  {
    return n;
  }
  /** Gets the decay \f$\tau\f$ or NAN if undefined. */
  double getDecay() const;
  /** Gets the bias \f$\beta\f$ or NAN if undefined. */
  double getBias() const;
  /** Gets the minimlal value. */
  double getMinimalValue() const
  {
    return cmin;
  }
  /** Gets a future value estimation time.
   * @param c The targeted value.
   * @return The estimation time in number of sample. Returns NAN if below the bias, 0 if already attained.
   */
  double getEstimationTime(double c) const;
  /** Returns the parameters as a JSON string. */
  std::string asString() const;

  /** Plots the observed and modeled exponential decay in a file.
   * @param file The file name, a <a href="http://gnuplot.sourceforge.net">gnuplot</a> display data, script and image files are generated, with extensions <tt>.dat</tt>, <tt>.gnuplot.sh</tt> and <tt>.png</tt>.
   * @param show If true performs on the fly display.
   */
  void show(String file, bool show = false) const;
};
