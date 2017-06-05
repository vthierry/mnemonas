/** Defines a discrete-time recurrent input/output transform. */
class RecurrentTransform: public Transform {
  mutable double *values;
  mutable unsigned int t0;
protected:
  unsigned int R, L;
public:
  RecurrentTransform(const RecurrentTransform &transform);

  /** Resets the transform for the given parameters.
   * @param N The number of output units, defined unit indexes stand in <tt>{0, N{</tt>.
   * @param R The maximal recurrent time range, at a current time <tt>t</tt>, defined times stand in <tt>[t+0, t+R[</tt>.
   * @param input The transform input.
   */
  RecurrentTransform(unsigned int N, unsigned int R, const Input &input);
  virtual ~RecurrentTransform();

  /** Resets the transform buffering.
   * - The current time is reset to 0.
   * @param buffered If true all <tt>input.getT()</tt> are buffered, else only <tt>R+1</tt> are buffered.
   * @return this.
   */
  RecurrentTransform& reset(bool buffered = false);
  /** Gets the maximal recuurent range. */
  unsigned int getR() const
  {
    return R;
  }
  double get(unsigned int n, double t) const;

  /** Implements the discrete-time recurrent equation.
   * - The method is to be overwritten to implement a given transform.
   * @param n The input unit index.
   * @param t The value time.
   * - Recurrent values at time <tt>t', t - R <= t' < t</tt> can be called.
   * @return The input value if defined, else raises a fatal error.
   * - It calculates
   * <center>\f$x_n(t) = \Phi_{nt}\left(\cdots, x_{n'}(t'), \cdots, i_{m}(s), \cdots\right)\f$,</center>
   */
  virtual double getValue(unsigned int n, double t) const
  {
    assume(false, "illegal-state", "in network::RecurrentTransform::get, this virtual method must be overloaded");
    return NAN;
  }
  /** Implements the discrete-time recurrent equation derivative.
   * - The method is to be overwritten to implement a given transform.
   * @param n The input unit index.
   * @param t The value time.
   * - Recurrent values at time <tt>t', t - R <= t' < t</tt> can be called.
   * @param n_ The input unit index denominator.
   * @param t_ The value time denominator.
   * @return The input value derivate if defined, 0 in the abscence of dependence, else raises a fatal error.
   * - If the recurrent equation writes
   * <center>\f$x_n(t) = \Phi_{nt}\left(\cdots, x_{n'}(t'), \cdots, i_{m}(s), \cdots\right)\f$,</center>
   * this method returns:
   * <center>\f$\frac{\partial \Phi_{nt}}{\partial x_{n'}(t')}\left(\cdots, x_{n'}(t'), \cdots, i_{m}(s), \cdots\right)\f$,</center>
   */
  virtual double getValueDerivative(unsigned int n, double t, unsigned int n_, double t_) const
  {
    assume(false, "illegal-state", "in network::RecurrentTransform::get, this virtual method must be overloaded");
    return NAN;
  }
};
