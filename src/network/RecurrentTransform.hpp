namespace network {
/** Defines a discrete-time recurrent input/output transform. */
  class RecurrentTransform: public Transform {
    // Recursion buffer and current time
    mutable double *values, upsilon0;
    mutable unsigned int t0;
    // Used to control recursion causality
    mutable int n_current, t_current;
    mutable bool no_recursion;
protected:
    unsigned int R, L;
public:
    /** Resets the transform for the given parameters.
     * @param N The number of output units, defined unit indexes stand in <tt>{0, N{</tt>.
     * @param R The maximal recurrent time range, at a current time <tt>t</tt>, defined times stand in <tt>[t+0, t+R[</tt>.
     * @param input The transform input.
     */
    RecurrentTransform(unsigned int N, unsigned int R, const Input &input);

    /// @cond INTERNAL
    virtual ~RecurrentTransform();
    ///@endcond

    /** Resets the transform buffering.
     * - The current time is reset to 0.
     * @param buffered If true all <tt>input.getT()</tt> are buffered, else only <tt>R+1</tt> are buffered.
     * @param upsilon Standard-deviation of the initial conditions normal random values.
     * @return this.
     */
    RecurrentTransform& reset(bool buffered = false, double upsilon = 0);
    /** Gets the maximal recurrent range. */
    unsigned int getR() const
    {
      return R;
    }
    double get(unsigned int n, double t) const;

    /** Sets an input value of the recurrent equation.
     * @param n The input unit index.
     * @param t The value time.
     * @param value The enforced input value.
     */
    void set(unsigned int n, double t, double value);

    /** Implements the discrete-time recurrent equation.
     * - The method is to be overwritten to implement a given transform.
     * @param n The input unit index.
     * @param t The value time.
     * - Recurrent values at time <tt>t', t - R <= t' < t</tt> can be called.
     * @return The input value if defined, else raises a fatal error.
     * - It calculates
     * <center>\f$x_n(t) = \Phi_{nt}\left(\cdots, x_{n'}(t'), \cdots, i_{m}(s), \cdots\right)\f$,</center>
     */
    virtual double getValue(unsigned int n, double t) const;

    /** Implements the discrete-time recurrent equation derivative.
     * - The method is to be overwritten to implement a given transform.
     * @param n The input unit index.
     * @param t The value time.
     * - Recurrent values at time <tt>t', t - R <= t' < t</tt> can be called.
     * @param n_ The afferent input unit index denominator.
     * @param t_ The value time denominator.
     * @return The input value derivate if defined, 0 in the abscence of dependence, else raises a fatal error.
     * - If the recurrent equation writes
     * <center>\f$x_n(t) = \Phi_{nt}\left(\cdots, x_{n'}(t'), \cdots, i_{m}(s), \cdots\right)\f$,</center>
     * this method returns:
     * <center>\f$\frac{\partial \Phi_{nt}}{\partial x_{n'}(t')}\left(\cdots, x_{n'}(t'), \cdots, i_{m}(s), \cdots\right)\f$,</center>
     */
    virtual double getValueDerivative(unsigned int n, double t, unsigned int n_, double t_) const;

    /** Checks if two units are connected.
     * - The method is to be overwritten to implement sparse connected networks.
     * @param n The efferent input unit index.
     * @param t The efferent input unit current time.
     * @param n_ The afferent input unit index denominator.
     * @param t_ The afferent input unit current time.
     * @return False if the unit of index <tt>n_</tt> never connects to the unit of index <tt>n</tt>, for the given times, true if the connection is to be checked via getValueDerivative()
     */
    virtual bool isConnected(unsigned int n, double t, unsigned int n_, double t_) const;

    /** Calculates the numerical approximate value derivative.
     * - This is only used for debugging purpose to check the coherence between getValue() and getValureDerivative().
     * @param n The input unit index.
     * @param t The value time.
     * @param n_ The input unit index denominator.
     * @param t_ The value time denominator.
     * @param epsilon The finite difference epsilon.
     * @return The input value derivate approximation:
     * <center>\f$\frac{\left.getValue(n, t)\right|_{x_{n'}(t') += \epsilon}-\left.getValue(n, t)\right|_{x_{n'}(t') -= \epsilon}}{\epsilon}\f$</center>
     */
    double getValueDerivativeApproximation(unsigned int n, double t, unsigned int n_, double t_, double epsilon = 1e-3) const;

    /** Estimates the Lyapunov exponent.
     * - Performs <tt>M simulations until <tt>t = T - W</tt>, adds a perturation and estimates the trajectory divergence.
     * @param W The Lyapunov exponent estimation window.
     * @param M The number of iterations.
     * @param d0 The perturbation amplitude.
     */
    double getLyapunovExponent(unsigned int W = 10, unsigned int M = 10, double d0 = 1e-6);

    /** Returns the transform parameters as a JSON string. */
    std::string asString() const;
  };
}
