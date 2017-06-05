/** Defines a discrete-time recurrent input/output kernel transform.
 * - A recurrent transform of the form for \f$n \in \{0, N\{\f$ and \f$t \in \{0, T\{\f$:
 * <center>\f$x_n(t) = \Phi_{n0t}\left(\cdots, x_{n'}(t'), \cdots, i_{m}(s), \cdots\right)
 * + \sum_{d = 1}^{D_{n}} W_{nd} \, \Phi_{ndt}\left(\cdots, x_{n'}(t'), \cdots, i_{m}(s), \cdots\right)\f$ </center> with:
 * -  \f$N\f$ nodes of value \f$x_{n}(t)\f$ indexed by \f$n \in \{0, N\{\f$,
 *   -  with a maximal state recurrent range of \f$R\f$,  \f$t - R \leq t' < t\f$ (i.e., taking into account recurrent value up to \f$R\f$ time-steps in the past),
 *   - and initial condition \f$x_{n}(t) = 0, t < 0\f$;
 *   - \f$N_0 \leq N\f$ of these nodes are output;
 * - \f$M\f$ input \f$i_{m}(s)\f$ indexed by \f$m \in \{0, M\{\f$,
 *   - with a maximal input recurrent range of \f$S\f$, \f$t - S \leq  s < t\f$,
 *   - and initial condition \f$i_{m}(s) = 0, s < 0\f$;
 * - The node structure is defined by \f$1 + D_{n}\f$ predefined kernels \f$\Phi_{ndt}\left(\right)\f$ for each node;
 * - The node parameters are static adjustable weights \f$W_{npd}\f$.
 */
class KernelTransform: public RecurrentTransform {
  double *weights;
  unsigned int *offsets, count;
  // A durty trick because C++ constructor can fucking not manage inheritance :(
  const KernelTransform *root;
  void init();
  void initOk()
  {
    if(weights == NULL)
      init();
  }
public:
  KernelTransform(const KernelTransform &transform);

  /** Resets the transform for the given parameters.
   * @param N The number of input units, defined unit indexes stand in <tt>{0, N{</tt>.
   * @param R The maximal recurrent time range, at a current time <tt>t</tt>, defined times stand in <tt>[t+0, t+R[</tt>.
   * @param input The transform input.
   */
  KernelTransform(unsigned int N, unsigned int R, const Input &input);
  virtual ~KernelTransform();

  /** Returns the recurrent kernel dimension.
   * - The method is to be overwritten to implement a given transform.
   * @param n The state unit index in <tt>{0, N{</tt>.
   * @return the state unit recurrent kernel dimension.
   */
  virtual unsigned int getKernelDimension(unsigned int n) const
  {
    assume(false, "illegal-state", "in network::KernelTransform::getKernelDimension, this virtual method must be overloaded");
    return 0;
  }
  /** Returns the recurrent kernel value.
   * - The method is to be overwritten to implement a given transform.
   * - The method uses input.get() and get() to get the related input or state values.
   * - The method has not to be protected against illegal-argument, since invoked by a safe method.
   * @param n The state unit index in <tt>{0, N{</tt>.
   * @param d The recurrent kernel index in <tt>{0, getKernelDimension(n)}</tt>.
   * @param t The time index in <tt>{0, T{</tt>.
   * @return The corresponding kernel value \f$\Phi_{ndt}\left(\cdots, x_{n'}[t'], \cdots, I_{m}[s], \cdots\right)\f$.
   */
  virtual double getKernelValue(unsigned int n, unsigned int d, double t) const
  {
    assume(false, "illegal-state", "in network::KernelTransform::getKernelValue, this virtual method must be overloaded");
    return NAN;
  }
  /** Returns the recurrent kernel derivative with respect to a recurrent state value.
   * - The method is to be overwritten to implement a given transform.
   * - The method uses input.get() and get() to get the related input or state values.
   * - The method has not to be protected against illegal-argument, since invoked by a safe method.
   * @param n The state unit index in <tt>{0, N{</tt>.
   * @param d The recurrent kernel index in <tt>{0, getKernelDimension(n)}</tt>.
   * @param t The state time index in <tt>{0, T{</tt>.
   * @param n_ The recurrent unit index in <tt>{0, N{</tt>.
   * @param t_ The recurrent time index in <tt>{t-R, t{</tt>.
   * @return Returns <tt>0.0</tt> if \f$x_n[t]\f$ does not depend on \f$x_k[s]\f$, else \f$\frac{\partial \phi_{ndt}}{\partial x_k[s]}\f$.
   */
  virtual double getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
  {
    assume(false, "illegal-state", "in network::KernelTransform::getKernelDerivative, this virtual method must be overloaded");
    return NAN;
  }
  /** Returns the total number of weights.
   * @return <tt>>_n getKernelDimension(n)</tt>
   */
  unsigned int getWeightCount() const
  {
    return count;
  }
  /** Gets one weight \f$W_{nd}\f$.
   * @param n The state unit index in <tt>{0, N{</tt>.
   * @param d The recurrent kernel index in <tt>{1, getKernelDimension()}</tt>.
   * @return The weight value.
   */
  double getWeight(unsigned int n, unsigned int d) const
  {
    const_cast < KernelTransform * > (this)->initOk();
    return n < N && 0 < d && d <= getKernelDimension(n) ? weights[offsets[n] + d] : 0;
  }
  /** Sets one weight \f$W_{nd}\f$ value.
   * - This routine is to be overloaded if the weights are bounded (e.g. if positive). A typical implementation writes, e.g.:<pre>
   *  bool setWeight(unsigned int n, unsigned int d, double w) {
   *    // Constraint the weight of idex 0 to be positive
   *    if (d == 0 && w < 0) {
   *      setWeight(n, d, 0);
   *      return false;
   *    } else
   *      return setWeight(n, d, d);
   *  }</pre>
   * @param n The state unit index in <tt>{0, N{</tt>.
   * @param d The recurrent kernel index in <tt>{1, getKernelDimension()}</tt>.
   * @param w The weight value.
   * @return True if the value is inside the bounds, false if the value has been bounded, i.e. is equal to the bound or outside the bound.
   */
  virtual bool setWeight(unsigned int n, unsigned int d, double w)
  {
    assume(n < N && 0 < d && d <= getKernelDimension(n), "illegal-argument", "in network::KernelTransform::setWeight, index out of range, we must have n=%d in {0, %d{ and d=%d in {1, %d}", n, N, d, getKernelDimension(n));
    initOk();
    weights[offsets[n] + d] = w;
    return true;
  }
  /** Sets random or constant weights values.
   * - Weights are drawn from either a uniform or a normal distribution.
   *   - If sigma = 0, it sets or adds to the weights a fixed value.
   *   - The uniform distribution stands in the  \f$[\min, \max] = [m - \sqrt{3} \, \sigma, m + \sqrt{3} \, \sigma] = m \pm 1.732 \, \sigma \f$ interval.
   * @param mean The distribution mean.
   * @param sigma The distribution standard-deviation.
   * @param add If true adds the random noise to existing values, else replaces values by the noise.
   * @param mode Either "uniform" or "normal" distribution.
   * @param seed If seed == -1 changes randomly the seed, taking the system time. The initial seed is 0, at the beginning of the program.
   * @return this.
   */
  KernelTransform& setWeightsRandom(double mean = 0.0, double sigma = 0.0, bool add = false, String mode = "normal", int seed = -1);
  double getValue(unsigned int n, double t) const;
  double getValueDerivative(unsigned int n, double t, unsigned int n_, double t_) const;
  /** Returns the weight values as a JSON string. */
  std::string asString() const;
};
